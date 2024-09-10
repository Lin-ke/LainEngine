#ifndef RENDERING_DEVICE_GRAPH_H
#define RENDERING_DEVICE_GRAPH_H
#include "core/thread/worker_thread_pool.h"
#include "rendering_device_driver.h"
#define USE_BUFFER_BARRIERS 1
#define PRINT_RESOURCE_TRACKER_TOTAL 1
// Buffer barriers have not shown any significant improvement or shown to be
// even detrimental to performance. However, there are currently some known
// cases where using them can solve problems that using singular memory
// barriers does not, probably due to driver issues (see comment on PR #84976
// https://github.com/godotengine/godot/pull/84976#issuecomment-1878566830).

namespace lain {

class RenderingDeviceGraph {
 public:
  struct ComputeListInstruction {
    enum Type {
      TYPE_NONE,
      TYPE_BIND_PIPELINE,
      TYPE_BIND_UNIFORM_SET,
      TYPE_DISPATCH,
      TYPE_DISPATCH_INDIRECT,
      TYPE_SET_PUSH_CONSTANT,
      TYPE_UNIFORM_SET_PREPARE_FOR_USE
    };

    Type type = TYPE_NONE;
  };
  /// @brief DrawList中的命令
  struct DrawListInstruction {
    enum Type {
      TYPE_NONE,
      TYPE_BIND_INDEX_BUFFER,
      TYPE_BIND_PIPELINE,
      TYPE_BIND_UNIFORM_SET,
      TYPE_BIND_VERTEX_BUFFERS,
      TYPE_CLEAR_ATTACHMENTS,
      TYPE_DRAW,
      TYPE_DRAW_INDEXED,
      TYPE_EXECUTE_COMMANDS,
      TYPE_NEXT_SUBPASS,
      TYPE_SET_BLEND_CONSTANTS,
      TYPE_SET_LINE_WIDTH,
      TYPE_SET_PUSH_CONSTANT,
      TYPE_SET_SCISSOR,
      TYPE_SET_VIEWPORT,
      TYPE_UNIFORM_SET_PREPARE_FOR_USE
    };

    Type type = TYPE_NONE;
  };
  /// @brief 命令的类型
  struct RecordedCommand {
    enum Type {
      TYPE_NONE,
      TYPE_BUFFER_CLEAR,
      TYPE_BUFFER_COPY,
      TYPE_BUFFER_GET_DATA,
      TYPE_BUFFER_UPDATE,
      TYPE_COMPUTE_LIST,
      TYPE_DRAW_LIST,
      TYPE_TEXTURE_CLEAR,
      TYPE_TEXTURE_COPY,
      TYPE_TEXTURE_GET_DATA,
      TYPE_TEXTURE_RESOLVE,
      TYPE_TEXTURE_UPDATE,
      TYPE_CAPTURE_TIMESTAMP,
      TYPE_MAX
    };
    Type type = TYPE_NONE;
    BitField<RDD::PipelineStageBits> next_stages;
    BitField<RDD::PipelineStageBits> self_stages;

    BitField<RDD::PipelineStageBits> previous_stages;
    int32_t label_index = -1;

    // barries
    // MemoryBarrier只有一个？
    RDD::MemoryBarrier memory_barrier;
    int32_t normalization_barrier_index = -1;  // 规范到纹理布局，指向command_normalization_barriers
    int normalization_barrier_count = 0;
    int32_t transition_barrier_index = -1;  // 这里的index指向command_transition_barriers
    int32_t transition_barrier_count = 0;
#if USE_BUFFER_BARRIERS
    int32_t buffer_barrier_index = -1;
    int32_t buffer_barrier_count = 0;
#endif

    // 图：邻接表 adjacent commands
    int32_t adjacent_command_list_index = -1;
  };

  struct CommandBufferPool {
    // Provided by RenderingDevice. RDD
    RDD::CommandPoolID pool;

    // Created internally by RenderingDeviceGraph.
    LocalVector<RDD::CommandBufferID> buffers;
    LocalVector<RDD::SemaphoreID> semaphores;
    uint32_t buffers_used = 0;
  };

  // copy所需数据的结构 (在update中使用)
  struct RecordedBufferCopy {
    RDD::BufferID source;
    RDD::BufferCopyRegion region;
  };

  struct RecordedBufferToTextureCopy {
    RDD::BufferID from_buffer;
    RDD::BufferTextureCopyRegion region;
  };

  enum ResourceUsage {
    RESOURCE_USAGE_NONE,
    RESOURCE_USAGE_COPY_FROM,
    RESOURCE_USAGE_COPY_TO,
    RESOURCE_USAGE_RESOLVE_FROM,
    RESOURCE_USAGE_RESOLVE_TO,
    RESOURCE_USAGE_UNIFORM_BUFFER_READ,
    RESOURCE_USAGE_INDIRECT_BUFFER_READ,
    RESOURCE_USAGE_TEXTURE_BUFFER_READ,
    RESOURCE_USAGE_TEXTURE_BUFFER_READ_WRITE,
    RESOURCE_USAGE_STORAGE_BUFFER_READ,
    RESOURCE_USAGE_STORAGE_BUFFER_READ_WRITE,
    RESOURCE_USAGE_VERTEX_BUFFER_READ,
    RESOURCE_USAGE_INDEX_BUFFER_READ,
    RESOURCE_USAGE_TEXTURE_SAMPLE,
    RESOURCE_USAGE_STORAGE_IMAGE_READ,
    RESOURCE_USAGE_STORAGE_IMAGE_READ_WRITE,
    RESOURCE_USAGE_ATTACHMENT_COLOR_READ_WRITE,
    RESOURCE_USAGE_ATTACHMENT_DEPTH_STENCIL_READ_WRITE
  };
  // tracker保存对资源写入和读取的命令的引用
  //  当命令以只读方式使用资源时，对该命令的引用将存储在资源跟踪器的列表中。对命令的引用放置在写入资源的最后一个操作的邻接列表中。
  // 当命令以读写方式使用资源时，对该命令的引用将存储在资源跟踪器中，替换前一个命令并清除从该资源读取的命令列表。对命令的引用放置在读取或写入资源的所有操作的邻接列表中。
  // 纹理有一个例外：如果操作必须更改使用类型，则该操作将被视为正在写入资源，因为需要内存布局转换。两个操作是否都是只读并不重要：无论如何都会建立写入依赖关系。这是值得记住的，因为如果纹理的使用经常变化，图表就会认为操作是相关的。
  struct ResourceTracker {
    uint32_t ref_count = 0;
    int64_t command_frame = -1;

    // reference of commands
    int32_t read_full_cmd_idx = -1;
    int32_t read_slice_cmd_idx = -1;
    int32_t write_cmd_idx = -1;
    int32_t draw_idx = -1;
    ResourceUsage draw_usage = RESOURCE_USAGE_NONE;
    int32_t compute_idx = -1;
    ResourceUsage compute_usage = RESOURCE_USAGE_NONE;

    bool write_cmd_list_enable = false;  // 已经有写命令
    bool read_cmd_list_enable = false;   // 已经有读命令

    ResourceUsage usage = RESOURCE_USAGE_NONE;
    BitField<RDD::BarrierAccessBits> usage_access;
    // buffer
    RDD::BufferID buffer_driver_id;  // .id = 0
    RDD::BufferRange buffer_range;
    // texture
    RDD::TextureID texture_driver_id;  // .id = 0
    RDD::TextureSubresourceRange texture_subresources;

    uint32_t texture_usage;
    int32_t slice_cmd_idx = -1;  // 最近使用的切片命令的索引

    ResourceTracker* parent = nullptr;
    // 保存脏列表，包含与父纹理使用的内存布局不同的切片。
    // 一个命令不允许使用重叠的相同纹理的切片（UB）
    ResourceTracker* dirty_shared_list = nullptr;  // 双向链表（to child）
    ResourceTracker* next_shared = nullptr;        // to parent
    Rect2i slice_or_dirty_rect;
    bool in_parent_dirty_list = false;

    _FORCE_INLINE_ void reset_if_outdated(int64_t new_command_frame) {
      if (new_command_frame != command_frame) {
        usage_access.clear();
        command_frame = new_command_frame;
        read_full_cmd_idx = -1;
        read_slice_cmd_idx = -1;
        write_cmd_idx = -1;
        draw_idx = -1;
        compute_idx = -1;
        slice_cmd_idx = -1;
      }
    }
    _FORCE_INLINE_ bool is_outdated(int64_t new_command_frame) const {
      return new_command_frame != command_frame;
    }
    L_INLINE bool is_buffer() const { return buffer_driver_id.id != 0; }
    L_INLINE bool is_texture() const { return texture_driver_id.id != 0; }
    L_INLINE bool has_write() const { return write_cmd_idx > 0; }
    L_INLINE Rect2i get_subresources() const {
      ERR_FAIL_COND_V_MSG(!is_buffer() && !is_texture(), Rect2i(),
                          "Resource is not a buffer or texture.");
      if (is_buffer()) {
        return Rect2i(buffer_range.offset, 0, buffer_range.size, 0);
      } else {
        return Rect2i(texture_subresources.base_mipmap, texture_subresources.base_layer,
                      texture_subresources.mipmap_count, texture_subresources.layer_count);
      }
    }
  };

 private:
  struct InstructionList {
    LocalVector<uint8_t> data;
    LocalVector<ResourceTracker*> command_trackers;
    LocalVector<ResourceUsage> command_tracker_usages;
    BitField<RDD::PipelineStageBits> stages;
    int32_t index = 0;  // 当前是第几个compute index

    void clear() {
      data.clear();
      command_trackers.clear();
      command_tracker_usages.clear();
      stages.clear();
    }
  };

  struct ComputeInstructionList : InstructionList {
    // No extra contents.
  };

  struct DrawInstructionList : InstructionList {
    RDD::RenderPassID render_pass;
    RDD::FramebufferID framebuffer;
    Rect2i region;
    LocalVector<RDD::RenderPassClearValue> clear_values;
  };

  struct RecordedCommandSort {
    uint32_t level = 0;
    uint32_t priority = 0;
    int32_t index = -1; // 通过index获取offset再获得command
    static const uint32 PriorityTable[RecordedCommand::Type::TYPE_MAX];

    RecordedCommandSort() = default;

    bool operator<(const RecordedCommandSort& p_other) const {
      if (level < p_other.level) {
        return true;
      } else if (level > p_other.level) {
        return false;
      }

      if (priority < p_other.priority) {
        return true;
      } else if (priority > p_other.priority) {
        return false;
      }

      return index < p_other.index;
    }
  };

  struct RecordedCommandListNode {
    int32_t command_index = -1;
    int32_t next_list_index = -1;
  };
  // 记录关于切片的命令
  struct RecordedSliceListNode {
    int32_t command_index = -1;
    int32_t next_list_index = -1;
    Rect2i
        subresources;  // 用rect2i记录subresource，可以覆盖texturesubresourcerange 和 buffer range
  };

  struct RecordedBufferClearCommand : RecordedCommand {
    RDD::BufferID buffer;
    uint32_t offset = 0;
    uint32_t size = 0;
  };

  struct RecordedBufferCopyCommand : RecordedCommand {
    RDD::BufferID source;
    RDD::BufferID destination;
    RDD::BufferCopyRegion region;
  };

  struct RecordedBufferGetDataCommand : RecordedCommand {
    RDD::BufferID source;
    RDD::BufferID destination;
    RDD::BufferCopyRegion region;
  };

  struct RecordedBufferUpdateCommand : RecordedCommand {
    RDD::BufferID destination;
    uint32_t buffer_copies_count = 0;

    _FORCE_INLINE_ RecordedBufferCopy* buffer_copies() {
      // this[1] 是 到this后，一个sizeof(this)的位置
      // 也就是说，this[1]是一个RecordedBufferCopy的数组
      // 其长度由buffer_copies_count决定
      // 相当于RecordedBufferCopy *buffer_copies = nullptr;
      // 可以参看函数_run_draw_list_command
      return reinterpret_cast<RecordedBufferCopy*>(&this[1]);
    }

    _FORCE_INLINE_ const RecordedBufferCopy* buffer_copies() const {
      return reinterpret_cast<const RecordedBufferCopy*>(&this[1]);
    }
  };

  struct RecordedComputeListCommand : RecordedCommand {
    uint32_t instruction_data_size = 0;

    _FORCE_INLINE_ uint8_t* instruction_data() { return reinterpret_cast<uint8_t*>(&this[1]); }

    _FORCE_INLINE_ const uint8_t* instruction_data() const {
      return reinterpret_cast<const uint8_t*>(&this[1]);
    }
  };

  struct RecordedDrawListCommand : RecordedCommand {
    uint32_t instruction_data_size = 0;
    RDD::RenderPassID render_pass;
    RDD::FramebufferID framebuffer;
    RDD::CommandBufferType command_buffer_type;
    Rect2i region;
    uint32_t clear_values_count = 0;

    _FORCE_INLINE_ RDD::RenderPassClearValue* clear_values() {
      return reinterpret_cast<RDD::RenderPassClearValue*>(&this[1]);
    }

    _FORCE_INLINE_ const RDD::RenderPassClearValue* clear_values() const {
      return reinterpret_cast<const RDD::RenderPassClearValue*>(&this[1]);
    }
    // 天才
    _FORCE_INLINE_ uint8_t* instruction_data() {
      return reinterpret_cast<uint8_t*>(&clear_values()[clear_values_count]);
    }

    _FORCE_INLINE_ const uint8_t* instruction_data() const {
      return reinterpret_cast<const uint8_t*>(&clear_values()[clear_values_count]);
    }
  };

  struct RecordedTextureClearCommand : RecordedCommand {
    RDD::TextureID texture;
    RDD::TextureSubresourceRange range;
    Color color;
  };

  struct RecordedTextureCopyCommand : RecordedCommand {
    RDD::TextureID from_texture;
    RDD::TextureID to_texture;
    uint32_t texture_copy_regions_count = 0;

    _FORCE_INLINE_ RDD::TextureCopyRegion* texture_copy_regions() {
      return reinterpret_cast<RDD::TextureCopyRegion*>(&this[1]);
    }

    _FORCE_INLINE_ const RDD::TextureCopyRegion* texture_copy_regions() const {
      return reinterpret_cast<const RDD::TextureCopyRegion*>(&this[1]);
    }
  };

  struct RecordedTextureGetDataCommand : RecordedCommand {
    RDD::TextureID from_texture;
    RDD::BufferID to_buffer;
    uint32_t buffer_texture_copy_regions_count = 0;

    _FORCE_INLINE_ RDD::BufferTextureCopyRegion* buffer_texture_copy_regions() {
      return reinterpret_cast<RDD::BufferTextureCopyRegion*>(&this[1]);
    }

    _FORCE_INLINE_ const RDD::BufferTextureCopyRegion* buffer_texture_copy_regions() const {
      return reinterpret_cast<const RDD::BufferTextureCopyRegion*>(&this[1]);
    }
  };

  struct RecordedTextureResolveCommand : RecordedCommand {
    RDD::TextureID from_texture;
    RDD::TextureID to_texture;
    uint32_t src_layer = 0;
    uint32_t src_mipmap = 0;
    uint32_t dst_layer = 0;
    uint32_t dst_mipmap = 0;
  };

  struct RecordedTextureUpdateCommand : RecordedCommand {
    RDD::TextureID to_texture;
    uint32_t buffer_to_texture_copies_count = 0;

    _FORCE_INLINE_ RecordedBufferToTextureCopy* buffer_to_texture_copies() {
      return reinterpret_cast<RecordedBufferToTextureCopy*>(&this[1]);
    }

    _FORCE_INLINE_ const RecordedBufferToTextureCopy* buffer_to_texture_copies() const {
      return reinterpret_cast<const RecordedBufferToTextureCopy*>(&this[1]);
    }
  };

  struct RecordedCaptureTimestampCommand : RecordedCommand {
    RDD::QueryPoolID pool;
    uint32_t index = 0;
  };

  struct DrawListBindIndexBufferInstruction : DrawListInstruction {
    RDD::BufferID buffer;
    RenderingDeviceCommons::IndexBufferFormat format;
    uint32_t offset = 0;
  };

  struct DrawListBindPipelineInstruction : DrawListInstruction {
    RDD::PipelineID pipeline;
  };

  struct DrawListBindUniformSetInstruction : DrawListInstruction {
    RDD::UniformSetID uniform_set;
    RDD::ShaderID shader;
    uint32_t set_index = 0;
  };

  struct DrawListBindVertexBuffersInstruction : DrawListInstruction {
    uint32_t vertex_buffers_count = 0;
    // vertex_buffer在this（该对象）后的第一个位置，长度为count
    _FORCE_INLINE_ RDD::BufferID* vertex_buffers() {
      return reinterpret_cast<RDD::BufferID*>(&this[1]);
    }

    _FORCE_INLINE_ const RDD::BufferID* vertex_buffers() const {
      return reinterpret_cast<const RDD::BufferID*>(&this[1]);
    }
    // vertex_buffer_offsets 在vertex_buffers后的第一个位置
    _FORCE_INLINE_ uint64_t* vertex_buffer_offsets() {
      return reinterpret_cast<uint64_t*>(&vertex_buffers()[vertex_buffers_count]);
    }

    _FORCE_INLINE_ const uint64_t* vertex_buffer_offsets() const {
      return reinterpret_cast<const uint64_t*>(&vertex_buffers()[vertex_buffers_count]);
    }
  };

  struct DrawListClearAttachmentsInstruction : DrawListInstruction {
    uint32_t attachments_clear_count = 0;
    uint32_t attachments_clear_rect_count = 0;

    _FORCE_INLINE_ RDD::AttachmentClear* attachments_clear() {
      return reinterpret_cast<RDD::AttachmentClear*>(&this[1]);
    }

    _FORCE_INLINE_ const RDD::AttachmentClear* attachments_clear() const {
      return reinterpret_cast<const RDD::AttachmentClear*>(&this[1]);
    }

    _FORCE_INLINE_ Rect2i* attachments_clear_rect() {
      return reinterpret_cast<Rect2i*>(&attachments_clear()[attachments_clear_count]);
    }

    _FORCE_INLINE_ const Rect2i* attachments_clear_rect() const {
      return reinterpret_cast<const Rect2i*>(&attachments_clear()[attachments_clear_count]);
    }
  };

  struct DrawListDrawInstruction : DrawListInstruction {
    uint32_t vertex_count = 0;
    uint32_t instance_count = 0;
  };

  struct DrawListDrawIndexedInstruction : DrawListInstruction {
    uint32_t index_count = 0;
    uint32_t instance_count = 0;
    uint32_t first_index = 0;
  };

  struct DrawListEndRenderPassInstruction : DrawListInstruction {
    // No contents.
  };

  struct DrawListExecuteCommandsInstruction : DrawListInstruction {
    RDD::CommandBufferID command_buffer;
  };

  struct DrawListSetPushConstantInstruction : DrawListInstruction {
    uint32_t size = 0;
    RDD::ShaderID shader;

    _FORCE_INLINE_ uint8_t* data() { return reinterpret_cast<uint8_t*>(&this[1]); }

    _FORCE_INLINE_ const uint8_t* data() const {
      return reinterpret_cast<const uint8_t*>(&this[1]);
    }
  };

  struct DrawListNextSubpassInstruction : DrawListInstruction {
    RDD::CommandBufferType command_buffer_type;
  };

  struct DrawListSetBlendConstantsInstruction : DrawListInstruction {
    Color color;
  };

  struct DrawListSetLineWidthInstruction : DrawListInstruction {
    float width;
  };

  struct DrawListSetScissorInstruction : DrawListInstruction {
    Rect2i rect;
  };

  struct DrawListSetViewportInstruction : DrawListInstruction {
    Rect2i rect;
  };

  struct DrawListUniformSetPrepareForUseInstruction : DrawListInstruction {
    RDD::UniformSetID uniform_set;
    RDD::ShaderID shader;
    uint32_t set_index = 0;
  };

  struct ComputeListBindPipelineInstruction : ComputeListInstruction {
    RDD::PipelineID pipeline;
  };

  struct ComputeListBindUniformSetInstruction : ComputeListInstruction {
    RDD::UniformSetID uniform_set;
    RDD::ShaderID shader;
    uint32_t set_index = 0;
  };

  struct ComputeListDispatchInstruction : ComputeListInstruction {
    uint32_t x_groups = 0;
    uint32_t y_groups = 0;
    uint32_t z_groups = 0;
  };

  struct ComputeListDispatchIndirectInstruction : ComputeListInstruction {
    RDD::BufferID buffer;
    uint32_t offset = 0;
  };

  struct ComputeListSetPushConstantInstruction : ComputeListInstruction {
    uint32_t size = 0;
    RDD::ShaderID shader;

    _FORCE_INLINE_ uint8_t* data() { return reinterpret_cast<uint8_t*>(&this[1]); }

    _FORCE_INLINE_ const uint8_t* data() const {
      return reinterpret_cast<const uint8_t*>(&this[1]);
    }
  };

  struct ComputeListUniformSetPrepareForUseInstruction : ComputeListInstruction {
    RDD::UniformSetID uniform_set;
    RDD::ShaderID shader;
    uint32_t set_index = 0;
  };

  struct BarrierGroup {
    BitField<RDD::PipelineStageBits> src_stages;
    BitField<RDD::PipelineStageBits> dst_stages;
    RDD::MemoryBarrier memory_barrier;
    LocalVector<RDD::TextureBarrier> normalization_barriers;
    LocalVector<RDD::TextureBarrier> transition_barriers;
#if USE_BUFFER_BARRIERS
    LocalVector<RDD::BufferBarrier> buffer_barriers;
#endif

    void clear() {
      src_stages.clear();
      dst_stages.clear();
      memory_barrier.src_access.clear();
      memory_barrier.dst_access.clear();
      normalization_barriers.clear();
      transition_barriers.clear();
#if USE_BUFFER_BARRIERS  // 如果define了且非0
      buffer_barriers.clear();
#endif
    }
  };
  //在initialize中初始化command_pool和command_buffer
  struct SecondaryCommandBuffer {
    LocalVector<uint8_t> instruction_data;
    RDD::CommandBufferID command_buffer;
    RDD::CommandPoolID command_pool;
    RDD::RenderPassID render_pass;
    RDD::FramebufferID framebuffer;
    WorkerThreadPool::TaskID task;
  };

  struct Frame {
    TightLocalVector<SecondaryCommandBuffer> secondary_command_buffers;
    uint32_t secondary_command_buffers_used = 0; // used buffers
  };

 private:
  /// @brief 添加一个命令到图中
  /// @param p_resource_trackers
  /// @param p_resource_usages
  /// @param p_resource_count
  /// @param p_command_index
  /// @param r_command
  void _add_command_to_graph(ResourceTracker** p_resource_trackers,
                             ResourceUsage* p_resource_usages, uint32_t p_resource_count,
                             int32_t p_command_index, RecordedCommand* r_command);
  /// @brief
  /// @param p_previous_command_index
  /// @param p_command_index
  /// @param r_command
  void _add_adjacent_command(int32_t p_previous_command_index, int32_t p_command_index,
                             RecordedCommand* r_command);
  /// @brief
  /// @param p_command_index
  /// @param p_list_index
  /// @return
  int32_t _add_to_command_list(int32_t p_command_index, int32_t p_list_index);
  static bool _is_write_usage(ResourceUsage p_usage);
  static RDD::TextureLayout _usage_to_image_layout(ResourceUsage p_usage);
  static RDD::BarrierAccessBits _usage_to_access_bits(ResourceUsage p_usage);
#if USE_BUFFER_BARRIERS
  void _add_buffer_barrier_to_command(RDD::BufferID p_buffer_id,
                                      BitField<RDD::BarrierAccessBits> p_src_access,
                                      BitField<RDD::BarrierAccessBits> p_dst_access,
                                      RDD::BufferRange p_range, int32_t& r_barrier_index,
                                      int32_t& r_barrier_count);
#endif
  void _add_texture_barrier_to_command(RDD::TextureID p_texture_id,
                                       BitField<RDD::BarrierAccessBits> p_src_access,
                                       BitField<RDD::BarrierAccessBits> p_dst_access,
                                       ResourceUsage p_prev_usage, ResourceUsage p_next_usage,
                                       RDD::TextureSubresourceRange p_subresources,
                                       LocalVector<RDD::TextureBarrier>& r_barrier_vector,
                                       int32_t& r_barrier_index, int32_t& r_barrier_count);
  int32_t _add_to_write_list(int32_t p_command_index, Rect2i p_subresources, int32_t p_list_index);
  int32_t _add_to_slice_read_list(int32_t p_command_index, Rect2i p_subresources,
                                  int32_t p_list_index);
  void _print_render_commands(const RecordedCommandSort* p_sorted_commands,
                              uint32_t p_sorted_commands_count);

  // alloc command
  RecordedCommand* _allocate_command(uint32_t p_command_size, int32_t& r_command_index);
  DrawListInstruction* _allocate_draw_list_instruction(uint32_t p_instruction_size);
  ComputeListInstruction* _allocate_compute_list_instruction(uint32_t p_instruction_size);

  void _run_secondary_command_buffer_task(const SecondaryCommandBuffer* p_secondary);
  void _run_draw_list_command(RDD::CommandBufferID p_command_buffer,
                              const uint8_t* p_instruction_data, uint32_t p_instruction_data_size);
  void _run_compute_list_command(RDD::CommandBufferID p_command_buffer,
                                 const uint8_t* p_instruction_data,
                                 uint32_t p_instruction_data_size);
  void _run_render_command(int32_t p_level, const RecordedCommandSort* p_sorted_commands,
                           uint32_t p_sorted_commands_count, RDD::CommandBufferID& r_command_buffer,
                           CommandBufferPool& r_command_buffer_pool, int32_t& r_current_label_index,
                           int32_t& r_current_label_level);
  /// @brief command_end_label和command_begin_label
  /// @param 

  void _run_label_command_change(RDD::CommandBufferID p_command_buffer, int32_t p_new_label_index,
                                 int32_t p_new_level, bool p_ignore_previous_value,
                                 bool p_use_label_for_empty,
                                 const RecordedCommandSort* p_sorted_commands,
                                 uint32_t p_sorted_commands_count, int32_t& r_current_label_index,
                                 int32_t& r_current_label_level);
  void _wait_for_secondary_command_buffer_tasks();
  void _group_barriers_for_render_commands(RDD::CommandBufferID p_command_buffer,
                                           const RecordedCommandSort* p_sorted_commands,
                                           uint32_t p_sorted_commands_count,
                                           bool p_full_memory_barrier);
void _boost_priority_for_render_commands(RecordedCommandSort *p_sorted_commands, uint32_t p_sorted_commands_count, uint32_t &r_boosted_priority);

    // 工具
    RecordedCommand* _get_command(int32_t p_command_index) {
      return reinterpret_cast<RecordedCommand*>(
          &command_data[command_data_offsets[p_command_index]]);
    }

   public:
    RenderingDeviceGraph();
    ~RenderingDeviceGraph();
    void initialize(RDD * p_driver, RenderingContextDriver::Device p_device, uint32_t p_frame_count,
                    RDD::CommandQueueFamilyID p_secondary_command_queue_family,
                    uint32_t p_secondary_command_buffers_per_frame);
    // 释放command_pool的资源， secondary_command_buffers的资源可以继续用
    // frame 需要clear
    void finalize();
    void begin();
    void add_buffer_clear(RDD::BufferID p_dst, ResourceTracker * p_dst_tracker, uint32_t p_offset,
                          uint32_t p_size);
    void add_buffer_copy(RDD::BufferID p_src, ResourceTracker * p_src_tracker, RDD::BufferID p_dst,
                         ResourceTracker * p_dst_tracker, RDD::BufferCopyRegion p_region);
    void add_buffer_get_data(RDD::BufferID p_src, ResourceTracker * p_src_tracker,
                             RDD::BufferID p_dst, RDD::BufferCopyRegion p_region);
    void add_buffer_update(RDD::BufferID p_dst, ResourceTracker * p_dst_tracker,
                           VectorView<RecordedBufferCopy> p_buffer_copies);
    void add_compute_list_begin();
    void add_compute_list_bind_pipeline(RDD::PipelineID p_pipeline);
    void add_compute_list_bind_uniform_set(RDD::ShaderID p_shader, RDD::UniformSetID p_uniform_set,
                                           uint32_t set_index);
    void add_compute_list_dispatch(uint32_t p_x_groups, uint32_t p_y_groups, uint32_t p_z_groups);
    void add_compute_list_dispatch_indirect(RDD::BufferID p_buffer, uint32_t p_offset);
    void add_compute_list_set_push_constant(RDD::ShaderID p_shader, const void* p_data,
                                            uint32_t p_data_size);
    void add_compute_list_uniform_set_prepare_for_use(
        RDD::ShaderID p_shader, RDD::UniformSetID p_uniform_set, uint32_t set_index);
    void add_compute_list_usage(ResourceTracker * p_tracker, ResourceUsage p_usage);
    void add_compute_list_usages(VectorView<ResourceTracker*> p_trackers,
                                 VectorView<ResourceUsage> p_usages);
    void add_compute_list_end();
    void add_draw_list_begin(RDD::RenderPassID p_render_pass, RDD::FramebufferID p_framebuffer,
                             Rect2i p_region, VectorView<RDD::RenderPassClearValue> p_clear_values,
                             bool p_uses_color, bool p_uses_depth);
    void add_draw_list_bind_index_buffer(RDD::BufferID p_buffer, RDD::IndexBufferFormat p_format,
                                         uint32_t p_offset);
    void add_draw_list_bind_pipeline(RDD::PipelineID p_pipeline,
                                     BitField<RDD::PipelineStageBits> p_pipeline_stage_bits);
    void add_draw_list_bind_uniform_set(RDD::ShaderID p_shader, RDD::UniformSetID p_uniform_set,
                                        uint32_t set_index);
    void add_draw_list_bind_vertex_buffers(VectorView<RDD::BufferID> p_vertex_buffers,
                                           VectorView<uint64_t> p_vertex_buffer_offsets);
    void add_draw_list_clear_attachments(VectorView<RDD::AttachmentClear> p_attachments_clear,
                                         VectorView<Rect2i> p_attachments_clear_rect);
    void add_draw_list_draw(uint32_t p_vertex_count, uint32_t p_instance_count);
    void add_draw_list_draw_indexed(uint32_t p_index_count, uint32_t p_instance_count,
                                    uint32_t p_first_index);
    void add_draw_list_execute_commands(RDD::CommandBufferID p_command_buffer);
    void add_draw_list_next_subpass(RDD::CommandBufferType p_command_buffer_type);
    void add_draw_list_set_blend_constants(const Color& p_color);
    void add_draw_list_set_line_width(float p_width);
    void add_draw_list_set_push_constant(RDD::ShaderID p_shader, const void* p_data,
                                         uint32_t p_data_size);
    void add_draw_list_set_scissor(Rect2i p_rect);
    void add_draw_list_set_viewport(Rect2i p_rect);
    void add_draw_list_uniform_set_prepare_for_use(
        RDD::ShaderID p_shader, RDD::UniformSetID p_uniform_set, uint32_t set_index);
    void add_draw_list_usage(ResourceTracker * p_tracker, ResourceUsage p_usage);
    void add_draw_list_usages(VectorView<ResourceTracker*> p_trackers,
                              VectorView<ResourceUsage> p_usages);
    void add_draw_list_end();
    void add_texture_clear(RDD::TextureID p_dst, ResourceTracker * p_dst_tracker,
                           const Color& p_color, const RDD::TextureSubresourceRange& p_range);
    void add_texture_copy(RDD::TextureID p_src, ResourceTracker * p_src_tracker,
                          RDD::TextureID p_dst, ResourceTracker * p_dst_tracker,
                          VectorView<RDD::TextureCopyRegion> p_texture_copy_regions);
    void add_texture_get_data(
        RDD::TextureID p_src, ResourceTracker * p_src_tracker, RDD::BufferID p_dst,
        VectorView<RDD::BufferTextureCopyRegion> p_buffer_texture_copy_regions,
        ResourceTracker* p_dst_tracker = nullptr);
    void add_texture_resolve(RDD::TextureID p_src, ResourceTracker * p_src_tracker,
                             RDD::TextureID p_dst, ResourceTracker * p_dst_tracker,
                             uint32_t p_src_layer, uint32_t p_src_mipmap, uint32_t p_dst_layer,
                             uint32_t p_dst_mipmap);
    void add_texture_update(
        RDD::TextureID p_dst, ResourceTracker * p_dst_tracker,
        VectorView<RecordedBufferToTextureCopy> p_buffer_copies,
        VectorView<ResourceTracker*> p_buffer_trackers = VectorView<ResourceTracker*>());
    void add_capture_timestamp(RDD::QueryPoolID p_query_pool, uint32_t p_index);
    void add_synchronization();
    void begin_label(const String& p_label_name, const Color& p_color);
    void end_label();
    // compile
    void end(bool p_reorder_commands, bool p_full_barriers, RDD::CommandBufferID& r_command_buffer,
             CommandBufferPool& r_command_buffer_pool);
    static ResourceTracker* resource_tracker_create();
    static void resource_tracker_free(ResourceTracker * tracker);

    bool driver_honors_barriers =
        false;  // 是否支持barrier，vulkan默认是1，d3d12则有enhanced_barriers_supported标志位
    bool driver_clears_with_copy_engine = false;  // 是否支持通过copy命令来进行clear
    RDD* driver = nullptr;
    RenderingContextDriver::Device device;
    int64_t tracking_frame = 0;  // 当前帧
    uint32_t command_count = 0;
    // label
    uint32_t command_label_count = 0;
    LocalVector<char> command_label_chars;
	LocalVector<Color> command_label_colors;
	LocalVector<uint32_t> command_label_offsets; // 在chars中的offset
	int32_t command_label_index = -1; // count时为在label中， -1则否
  // frame
  uint32_t frame = 0;
  TightLocalVector<Frame> frames;

  RBMap<ResourceTracker*, uint32_t> write_dependency_counters;

  int32_t command_timestamp_index = -1;  // previous timestamp command index

  // 在当前命令同步（通过add_synchronization 设置为true）
  bool command_synchronization_pending = false;
  int32_t command_synchronization_index = -1;  // previous synchronization command index
                                               // Texture
  LocalVector<RDD::TextureBarrier> command_transition_barriers;  // transition barrier
  LocalVector<RDD::TextureBarrier>
      command_normalization_barriers;  // normalization barrier,转变layout
#if USE_BUFFER_BARRIERS
  LocalVector<RDD::BufferBarrier> command_buffer_barriers;  // buffer barrier
#endif

  LocalVector<uint32_t> command_data_offsets;  // 标记了每个data的offset
  LocalVector<uint8_t> command_data;           // 存储了所有的command data

  LocalVector<RecordedCommandListNode>
      command_list_nodes;  // 邻接表节点数组，在recordcommand中通过id索引，插入是倒着的
  LocalVector<RecordedSliceListNode> write_slice_list_nodes;  // 写入切片节点数组
  LocalVector<RecordedSliceListNode> read_slice_list_nodes;   // 读切片节点数组

  DrawInstructionList draw_instruction_list;
  ComputeInstructionList compute_instruction_list;

 private:
#if PRINT_RESOURCE_TRACKER_TOTAL
  static uint32_t resource_tracker_total;
#endif
  BarrierGroup barrier_group;
  };

using RDG = RenderingDeviceGraph;

}  //namespace lain

#endif