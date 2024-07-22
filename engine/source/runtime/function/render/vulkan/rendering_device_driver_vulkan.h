
#ifndef RENDERING_DEVICE_DRIVER_VULKAN_H
#define RENDERING_DEVICE_DRIVER_VULKAN_H
#include "core/templates/hash_map.h"
#include "core/templates/hash_set.h"
#include "core/templates/paged_allocator.h"
#include "function/render/common/rendering_device_driver.h"
#include "rendering_context_driver_vulkan.h"
#include "vulkan_header.h"

namespace lain::graphics {

// Design principles:
// - Vulkan structs are zero-initialized and fields not requiring a non-zero value are omitted (except in cases where expresivity reasons apply).
// @TODO 在未来采用一种状态机的方式来管理一些状态，例如
// - 能否set viewport, 因为这需要正在绘制shader object或者管线通过dynamic_state_viewport标志创建。
// - etc. （例如，在绑定vertex 之后是否重新绑定了vertex，这与缓存有关？
// 参考granite的实现，在draw命令时调用flush函数，。
class RenderingDeviceDriverVulkan : public RenderingDeviceDriver {
  /// ---apis
  struct CommandQueue;
  struct SwapChain;

 public:
  /*****************/
  /**** TEXTURE ****/
  /*****************/
  // 这里只有imageview
  // vk image似乎没有什么用，保存在create_info->image里

  struct TextureInfo {
    VkImageView vk_view = VK_NULL_HANDLE;
    DataFormat rd_format = DATA_FORMAT_MAX;
    VkImageCreateInfo vk_create_info = {};
    VkImageViewCreateInfo vk_view_create_info = {};
    struct {
      VmaAllocation handle = nullptr;
      VmaAllocationInfo info = {};
    } allocation;  // All 0/null if just a view.
  };

  virtual TextureID texture_create(const TextureFormat& p_format, const TextureView& p_view) override final;
  virtual TextureID texture_create_from_extension(uint64_t p_native_texture, TextureType p_type, DataFormat p_format,
                                                  uint32_t p_array_layers, bool p_depth_stencil) override final;
  virtual TextureID texture_create_shared(TextureID p_original_texture, const TextureView& p_view) override final;
  // 区别在sub resource range和view type上
  virtual TextureID texture_create_shared_from_slice(TextureID p_original_texture, const TextureView& p_view,
                                                     TextureSliceType p_slice_type, uint32_t p_layer, uint32_t p_layers,
                                                     uint32_t p_mipmap, uint32_t p_mipmaps) override final;
  virtual void texture_free(TextureID p_texture) override final;
  virtual uint8_t* texture_map(TextureID p_texture, const TextureSubresource& p_subresource) override final;
  virtual void texture_unmap(TextureID p_texture) override final;

  virtual void texture_get_copyable_layout(TextureID p_texture, const TextureSubresource& p_subresource,
                                           TextureCopyableLayout* r_layout) override final;
  virtual uint64_t texture_get_allocation_size(TextureID p_texture) override final;
  // virtual bool texture_can_make_shared_with_format(TextureID p_texture, DataFormat p_format, bool &r_raw_reinterpretation) override final;
  virtual BitField<TextureUsageBits> texture_get_usages_supported_by_format(DataFormat p_format, bool p_cpu_readable) override;

  // 已有VKimage,创建view

 private:
  VkSampleCountFlagBits _ensure_supported_sample_count(TextureSamples p_requested_sample_count);
  /*****************/
  /**** SAMPLER ****/
  /*****************/
 public:
  virtual SamplerID sampler_create(const SamplerState& p_state) final override;
  virtual void sampler_free(SamplerID p_sampler) final override;
  virtual bool sampler_is_format_supported_for_filter(DataFormat p_format, SamplerFilter p_filter) override final;

  /**********************/
  /**** VERTEX ARRAY ****/
  /**********************/
 private:
  struct VertexFormatInfo {
    TightLocalVector<VkVertexInputBindingDescription> vk_bindings;
    TightLocalVector<VkVertexInputAttributeDescription> vk_attributes;
    VkPipelineVertexInputStateCreateInfo vk_create_info = {};
  };

 public:
  virtual VertexFormatID vertex_format_create(VectorView<VertexAttribute> p_vertex_attribs) override final;
  virtual void vertex_format_free(VertexFormatID p_vertex_format) override final;
  /*****************/
  /**** BUFFERS ****/
  /*****************/
 private:
  struct BufferInfo {
    VkBuffer vk_buffer = VK_NULL_HANDLE;
    struct {
      VmaAllocation handle = nullptr;
      uint64_t size = UINT64_MAX;
    } allocation;
    uint64_t size = 0;
    VkBufferView vk_view = VK_NULL_HANDLE;  // For texel buffers.
  };

 public:
  virtual BufferID buffer_create(uint64_t p_size, BitField<BufferUsageBits> p_usage,
                                 MemoryAllocationType p_allocation_type) override final;
  virtual bool buffer_set_texel_format(BufferID p_buffer, DataFormat p_format) override final;
  virtual void buffer_free(BufferID p_buffer) override final;
  virtual uint64_t buffer_get_allocation_size(BufferID p_buffer) override final;
  virtual uint8_t* buffer_map(BufferID p_buffer) override final;
  virtual void buffer_unmap(BufferID p_buffer) override final;
  /****************/
  /**** SHADER ****/
  /****************/
 private:
  // @TODO 光追
  struct ShaderBinary {
    enum { VERSION = 1 };
    struct DataBinding {
      uint32_t type = 0;
      uint32_t binding = 0;
      uint32_t stages = 0;
      uint32_t length = 0;  // Size of arrays (in total elements), or UBOs (in bytes * total elements).
      uint32_t writable = 0;
    };

    struct SpecializationConstant {
      uint32_t type = 0;
      uint32_t constant_id = 0;
      uint32_t int_value = 0;
      uint32_t stage_flags = 0;
    };

    struct Data {
      uint64_t vertex_input_mask = 0;
      uint32_t fragment_output_mask = 0;
      uint32_t specialization_constants_count = 0;
      uint32_t is_compute = 0;
      uint32_t compute_local_size[3] = {};
      uint32_t set_count = 0;  // uniform set count
      uint32_t push_constant_size = 0;
      uint32_t vk_push_constant_stages_mask = 0;  // push constants的shader stage
      uint32_t stage_count = 0;
      uint32_t shader_name_len = 0;
    };
  };

  struct ShaderInfo {
    VkShaderStageFlags vk_push_constant_stages = 0;
    TightLocalVector<VkPipelineShaderStageCreateInfo> vk_stages_create_info;
    TightLocalVector<VkDescriptorSetLayout> vk_descriptor_set_layouts;  // 需要根据ShaderReflectionData创建
    VkPipelineLayout vk_pipeline_layout = VK_NULL_HANDLE;               // pipeline layout是由shader控制的
  };

 public:
  virtual Vector<uint8_t> shader_compile_binary_from_spirv(VectorView<ShaderStageSPIRVData> p_spirv,
                                                           const String& p_shader_name) override final;
  virtual ShaderID shader_create_from_bytecode(const Vector<uint8_t>& p_shader_binary, ShaderDescription& r_shader_desc,
                                               String& r_name) override final;
  virtual void shader_free(ShaderID p_shader) override final;

  /*********************/
  /**** UNIFORM SET ****/
  /*********************/

  static const uint32_t MAX_UNIFORM_POOL_ELEMENT = 65535;
  // 为每个描述符集=[描述符类型+数量]创建一个池

  struct DescriptorSetPoolKey {
    uint16_t uniform_type[UNIFORM_TYPE_MAX] = {};

    bool operator<(const DescriptorSetPoolKey& p_other) const {
      return memcmp(uniform_type, p_other.uniform_type, sizeof(uniform_type)) < 0;
    }
  };
  uint32_t max_descriptor_sets_per_pool = 0;
  using DescriptorSetPools =
      RBMap<DescriptorSetPoolKey, HashMap<VkDescriptorPool, uint32_t>>;  // 池和引用次数，超过Max则需要新的（相同的）池
  struct UniformSetInfo {
    VkDescriptorSet vk_descriptor_set = VK_NULL_HANDLE;
    VkDescriptorPool vk_descriptor_pool = VK_NULL_HANDLE;
    DescriptorSetPools::Iterator pool_sets_it = {};
  };
  DescriptorSetPools descriptor_set_pools;
  VkDescriptorPool _descriptor_set_pool_find_or_create(const DescriptorSetPoolKey& p_key,
                                                       DescriptorSetPools::Iterator* r_pool_sets_it);
  void _descriptor_set_pool_unreference(DescriptorSetPools::Iterator p_pool_sets_it, VkDescriptorPool p_vk_descriptor_pool);

 public:
  virtual UniformSetID uniform_set_create(VectorView<BoundUniform> p_uniforms, ShaderID p_shader,
                                          uint32_t p_set_index) override final;
  virtual void uniform_set_free(UniformSetID p_uniform_set) override final;

  /******************/
  /**** TRANSFER ****/
  /******************/

  virtual void command_clear_buffer(CommandBufferID p_cmd_buffer, BufferID p_buffer, uint64_t p_offset,
                                    uint64_t p_size) override final;
  virtual void command_copy_buffer(CommandBufferID p_cmd_buffer, BufferID p_src_buffer, BufferID p_dst_buffer,
                                   VectorView<BufferCopyRegion> p_regions) override final;

  virtual void command_copy_texture(CommandBufferID p_cmd_buffer, TextureID p_src_texture, TextureLayout p_src_texture_layout,
                                    TextureID p_dst_texture, TextureLayout p_dst_texture_layout,
                                    VectorView<TextureCopyRegion> p_regions) override final;
  virtual void command_resolve_texture(CommandBufferID p_cmd_buffer, TextureID p_src_texture, TextureLayout p_src_texture_layout,
                                       uint32_t p_src_layer, uint32_t p_src_mipmap, TextureID p_dst_texture,
                                       TextureLayout p_dst_texture_layout, uint32_t p_dst_layer,
                                       uint32_t p_dst_mipmap) override final;
  virtual void command_clear_color_texture(CommandBufferID p_cmd_buffer, TextureID p_texture, TextureLayout p_texture_layout,
                                           const Color& p_color, const TextureSubresourceRange& p_subresources) override final;

  virtual void command_copy_buffer_to_texture(CommandBufferID p_cmd_buffer, BufferID p_src_buffer, TextureID p_dst_texture,
                                              TextureLayout p_dst_texture_layout,
                                              VectorView<BufferTextureCopyRegion> p_regions) override final;
  virtual void command_copy_texture_to_buffer(CommandBufferID p_cmd_buffer, TextureID p_src_texture,
                                              TextureLayout p_src_texture_layout, BufferID p_dst_buffer,
                                              VectorView<BufferTextureCopyRegion> p_regions) override final;

  /************** */
  /**** Barriers ****/
  /************** */
  virtual void command_pipeline_barrier(CommandBufferID p_cmd_buffer, BitField<PipelineStageBits> p_src_stages,
                                        BitField<PipelineStageBits> p_dst_stages, VectorView<MemoryBarrier> p_memory_barriers,
                                        VectorView<BufferBarrier> p_buffer_barriers,
                                        VectorView<TextureBarrier> p_texture_barriers) override final;
  /****************/
  /**** FENCES ****/
  /****************/

 private:
  struct Fence {
    VkFence vk_fence = VK_NULL_HANDLE;
    CommandQueue* queue_signaled_from = nullptr;
  };

 public:
  virtual FenceID fence_create() override final;
  virtual Error fence_wait(FenceID p_fence) override final;
  virtual void fence_free(FenceID p_fence) override final;

  /********************/
  /**** SEMAPHORES ****/
  /********************/

  virtual SemaphoreID semaphore_create() override final;
  virtual void semaphore_free(SemaphoreID p_semaphore) override final;
  /*****************/
  /**** QUERIES ****/
  /*****************/
  // ----- TIMESTAMP -----

  // Basic.
  virtual QueryPoolID timestamp_query_pool_create(uint32_t p_query_count) override final;
  virtual void timestamp_query_pool_free(QueryPoolID p_pool_id) override final;
  virtual void timestamp_query_pool_get_results(QueryPoolID p_pool_id, uint32_t p_query_count,
                                                uint64_t* r_results) override final;
  virtual uint64_t timestamp_query_result_to_time(uint64_t p_result) override final;
  // commands
  virtual void command_timestamp_query_pool_reset(CommandBufferID p_cmd_buffer, QueryPoolID p_pool_id,
                                                  uint32_t p_query_count) override final;
  virtual void command_timestamp_write(CommandBufferID p_cmd_buffer, QueryPoolID p_pool_id, uint32_t p_index) override final;
  // ----- OCCLUSION -----
  // @todo add occulusion query

  /********************/
  /**** SUBMISSION ****/
  /********************/
  // d3d12
 public:
  virtual void begin_segment(uint32_t p_frame_index, uint32_t p_frames_drawn) override final;
  virtual void end_segment() override final;

  /******************/
  /**** COMMAND BUFFERS ****/
  /******************/

  // ----- QUEUE FAMILY -----

  virtual CommandQueueFamilyID command_queue_family_get(BitField<CommandQueueFamilyBits> p_cmd_queue_family_bits,
                                                        RenderingContextDriver::SurfaceID p_surface = 0) override final;

  // ----- QUEUE -----
 private:
  struct CommandQueue {
    LocalVector<VkSemaphore> present_semaphores;
    LocalVector<VkSemaphore> image_semaphores;
    LocalVector<SwapChain*> image_semaphores_swap_chains;  // --- image semaphore的index 到 swap chain
    LocalVector<uint32_t> pending_semaphores_for_execute;
    LocalVector<uint32_t> pending_semaphores_for_fence;  // 需要给fence信号的信号量
    LocalVector<uint32_t> free_image_semaphores;         // --- 等待清理
    LocalVector<Pair<Fence*, uint32_t>>
        image_semaphores_for_fences;  // fence to image_semaphores index，需要给fence的信号量加入这里
    uint32_t queue_family = 0;        // 通过这个索引到vkqueue
    uint32_t queue_index = 0;
    uint32_t present_semaphore_index = 0;
  };

 public:
  virtual CommandQueueID command_queue_create(CommandQueueFamilyID p_cmd_queue_family,
                                              bool p_identify_as_main_queue) override final;
  virtual Error command_queue_execute_and_present(CommandQueueID p_cmd_queue, VectorView<SemaphoreID> p_wait_semaphores,
                                                  VectorView<CommandBufferID> p_cmd_buffers,
                                                  VectorView<SemaphoreID> p_cmd_semaphores, FenceID p_cmd_fence,
                                                  VectorView<SwapChainID> p_swap_chains) override final;
  virtual void command_queue_free(CommandQueueID p_cmd_queue) override final;

  // ----- POOL -----
 private:
  struct CommandPool {
    VkCommandPool vk_command_pool = VK_NULL_HANDLE;
    CommandBufferType buffer_type = COMMAND_BUFFER_TYPE_PRIMARY;
  };

 public:
  virtual CommandPoolID command_pool_create(CommandQueueFamilyID p_cmd_queue_family,
                                            CommandBufferType p_cmd_buffer_type) override final;
  virtual void command_pool_free(CommandPoolID p_cmd_pool) override final;

 private:
  virtual CommandBufferID command_buffer_create(CommandPoolID p_cmd_pool) override final;
  virtual bool command_buffer_begin(CommandBufferID p_cmd_buffer) override final;
  virtual bool command_buffer_begin_secondary(CommandBufferID p_cmd_buffer, RenderPassID p_render_pass, uint32_t p_subpass,
                                              FramebufferID p_framebuffer) override final;
  virtual void command_buffer_end(CommandBufferID p_cmd_buffer) override final;
  virtual void command_buffer_execute_secondary(CommandBufferID p_cmd_buffer,
                                                VectorView<CommandBufferID> p_secondary_cmd_buffers) override final;

  /********************/
  /**** SWAP CHAIN ****/
  /********************/

 private:
  // FrameBuffer和VKImage都是屏幕所用的
  // FrameBuffer hold the Images, Renderpass use the format
  struct SwapChain {
    VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE;
    RenderingContextDriver::SurfaceID surface = RenderingContextDriver::SurfaceID();
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    TightLocalVector<VkImage> images;
    TightLocalVector<VkImageView> image_views;
    TightLocalVector<FramebufferID> framebuffers;
    LocalVector<CommandQueue*> command_queues_acquired;        //
    LocalVector<uint32_t> command_queues_acquired_semaphores;  // semaphore in command queue index
    uint32_t image_index = 0;
    RenderPassID render_pass;
    VkPresentModeKHR active_present_mode = VK_PRESENT_MODE_FIFO_KHR;
  };

 public:
  virtual SwapChainID swap_chain_create(RenderingContextDriver::SurfaceID p_surface) override final;
  // VKFramebuffer create in resize..
  virtual Error swap_chain_resize(CommandQueueID p_cmd_queue, SwapChainID p_swap_chain,
                                  uint32_t p_desired_framebuffer_count) override final;
  // acquire next image
  virtual FramebufferID swap_chain_acquire_framebuffer(CommandQueueID p_cmd_queue, SwapChainID p_swap_chain,
                                                       bool& r_resize_required) override final;
  virtual RenderPassID swap_chain_get_render_pass(SwapChainID p_swap_chain) override final;
  virtual DataFormat swap_chain_get_format(SwapChainID p_swap_chain) override final;  // 其实就是RGBA8或者BGRA8

  // external wait needed!
  virtual void swap_chain_free(SwapChainID p_swap_chain) override final;

  /*********************/
  /**** FRAMEBUFFER ****/
  /*********************/

  virtual FramebufferID framebuffer_create(RenderPassID p_render_pass, VectorView<TextureID> p_attachments, uint32_t p_width,
                                           uint32_t p_height) override final;
  virtual void framebuffer_free(FramebufferID p_framebuffer) override final;

  /******************/
  /**** PIPELINE ****/
  /******************/
 private:
  struct PipelineCacheHeader {
    uint32_t magic = 0;
    uint32_t data_size = 0;
    uint64_t data_hash = 0;
    uint32_t vendor_id = 0;
    uint32_t device_id = 0;
    uint32_t driver_version = 0;
    uint8_t uuid[VK_UUID_SIZE] = {};
    uint8_t driver_abi = 0;
  };

  struct PipelineCache {
    String file_path;
    size_t current_size = 0;
    Vector<uint8_t> buffer;  // Header then data.
    VkPipelineCache vk_cache = VK_NULL_HANDLE;
  };

  static int caching_instance_count;
  PipelineCache pipelines_cache;
  String pipeline_cache_id;

  virtual PipelineID render_pipeline_create(ShaderID p_shader, VertexFormatID p_vertex_format, RenderPrimitive p_render_primitive,
                                            PipelineRasterizationState p_rasterization_state,
                                            PipelineMultisampleState p_multisample_state,
                                            PipelineDepthStencilState p_depth_stencil_state,
                                            PipelineColorBlendState p_blend_state, VectorView<int32_t> p_color_attachments,
                                            BitField<PipelineDynamicStateFlags> p_dynamic_state, RenderPassID p_render_pass,
                                            uint32_t p_render_subpass,
                                            VectorView<PipelineSpecializationConstant> p_specialization_constants) override final;

  virtual void pipeline_free(PipelineID p_pipeline) override final;
  // --- binding --- // 管线需要绑定的是push constants

  virtual void command_bind_push_constants(CommandBufferID p_cmd_buffer, ShaderID p_shader, uint32_t p_first_index,
                                           VectorView<uint32_t> p_data) override final;

  // --- cache ---
  virtual bool pipeline_cache_create(const Vector<uint8_t>& p_data) override final;
  virtual void pipeline_cache_free() override final;
  /*******************/
  /**** RENDERING ****/
  /*******************/
  // ----- SUBPASS -----
  // 这里封装的十分如封
  virtual RenderPassID render_pass_create(VectorView<Attachment> p_attachments, VectorView<Subpass> p_subpasses,
                                          VectorView<SubpassDependency> p_subpass_dependencies,
                                          uint32_t p_view_count) override final;
  virtual void render_pass_free(RenderPassID p_render_pass) override final;
  // ----- RENDER command -----
  virtual void command_begin_render_pass(CommandBufferID p_cmd_buffer, RenderPassID p_render_pass, FramebufferID p_framebuffer,
                                         CommandBufferType p_cmd_buffer_type, const Rect2i& p_rect,
                                         VectorView<RenderPassClearValue> p_clear_values) override final;
  virtual void command_end_render_pass(CommandBufferID p_cmd_buffer) override final;
  virtual void command_next_render_subpass(CommandBufferID p_cmd_buffer, CommandBufferType p_cmd_buffer_type) override final;
  // ----- COMMANDS -----
  // dynamic state
  virtual void command_render_set_viewport(CommandBufferID p_cmd_buffer, VectorView<Rect2i> p_viewports) override final;
  virtual void command_render_set_scissor(CommandBufferID p_cmd_buffer, VectorView<Rect2i> p_scissors) override final;
  virtual void command_render_clear_attachments(CommandBufferID p_cmd_buffer, VectorView<AttachmentClear> p_attachment_clears,
                                                VectorView<Rect2i> p_rects) override final;

  // Binding.
  virtual void command_bind_render_pipeline(CommandBufferID p_cmd_buffer, PipelineID p_pipeline) override final;
  virtual void command_bind_render_uniform_set(CommandBufferID p_cmd_buffer, UniformSetID p_uniform_set, ShaderID p_shader,
                                               uint32_t p_set_index) override final;

  // Drawing.
  virtual void command_render_draw(CommandBufferID p_cmd_buffer, uint32_t p_vertex_count, uint32_t p_instance_count,
                                   uint32_t p_base_vertex, uint32_t p_first_instance) override final;
  virtual void command_render_draw_indexed(CommandBufferID p_cmd_buffer, uint32_t p_index_count, uint32_t p_instance_count,
                                           uint32_t p_first_index, int32_t p_vertex_offset,
                                           uint32_t p_first_instance) override final;
  virtual void command_render_draw_indexed_indirect(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset,
                                                    uint32_t p_draw_count, uint32_t p_stride) override final;
  virtual void command_render_draw_indexed_indirect_count(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer,
                                                          uint64_t p_offset, BufferID p_count_buffer,
                                                          uint64_t p_count_buffer_offset, uint32_t p_max_draw_count,
                                                          uint32_t p_stride) override final;
  virtual void command_render_draw_indirect(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset,
                                            uint32_t p_draw_count, uint32_t p_stride) override final;
  virtual void command_render_draw_indirect_count(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset,
                                                  BufferID p_count_buffer, uint64_t p_count_buffer_offset,
                                                  uint32_t p_max_draw_count, uint32_t p_stride) override final;

  // Buffer binding.
  // 主要就是绑定顶点数组和索引数组
  virtual void command_render_bind_vertex_buffers(CommandBufferID p_cmd_buffer, uint32_t p_binding_count,
                                                  const BufferID* p_buffers, const uint64_t* p_offsets) override final;
  virtual void command_render_bind_index_buffer(CommandBufferID p_cmd_buffer, BufferID p_buffer, IndexBufferFormat p_format,
                                                uint64_t p_offset) override final;

  // Dynamic state.
  virtual void command_render_set_blend_constants(CommandBufferID p_cmd_buffer, const Color& p_constants) override final;
  virtual void command_render_set_line_width(CommandBufferID p_cmd_buffer, float p_width) override final;

  /*************** */
  /**** COMPUTE ****/
  /*************** */
  virtual PipelineID compute_pipeline_create(
      ShaderID p_shader, VectorView<PipelineSpecializationConstant> p_specialization_constants) override final;
  // ----- compute command -----
  // Binding.
  virtual void command_bind_compute_pipeline(CommandBufferID p_cmd_buffer, PipelineID p_pipeline) override final;
  virtual void command_bind_compute_uniform_set(CommandBufferID p_cmd_buffer, UniformSetID p_uniform_set, ShaderID p_shader,
                                                uint32_t p_set_index) override final;

  // Dispatching.
  virtual void command_compute_dispatch(CommandBufferID p_cmd_buffer, uint32_t p_x_groups, uint32_t p_y_groups,
                                        uint32_t p_z_groups) override final;
  virtual void command_compute_dispatch_indirect(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer,
                                                 uint64_t p_offset) override final;

  /****************/
  /**** LABELS ****/
  /****************/

  virtual void command_begin_label(CommandBufferID p_cmd_buffer, const char* p_label_name, const Color& p_color) override final;
  virtual void command_end_label(CommandBufferID p_cmd_buffer) override final;

  /*****************/
  /**** GENERIC ****/
  /*****************/
 private:
  struct Queue {
    VkQueue queue = VK_NULL_HANDLE;
    uint32_t virtual_count = 0;  // 这个队列被使用的次数 ，暂不用
    BinaryMutex submit_mutex;
  };
  struct ShaderCapabilities {
    bool shader_float16_is_supported = false;
    bool shader_int8_is_supported = false;
  };
  struct VRSCapabilities {
    bool pipeline_vrs_supported = false;    // We can specify our fragment rate on a pipeline level.
    bool primitive_vrs_supported = false;   // We can specify our fragment rate on each drawcall.
    bool attachment_vrs_supported = false;  // We can provide a density map attachment on our framebuffer.

    Size2i min_texel_size;
    Size2i max_texel_size;

    Size2i texel_size;  // The texel size we'll use
  };
  // multi-view capabilities 被放到了更上一层，因为更通用吧
  struct StorageBufferCapabilities {
    bool storage_buffer_16_bit_access_is_supported = false;
    bool uniform_and_storage_buffer_16_bit_access_is_supported = false;
    bool storage_push_constant_16_is_supported = false;
    bool storage_input_output_16 = false;
  };

  struct SubgroupCapabilities {
    uint32_t size = 0;
    uint32_t min_size = 0;
    uint32_t max_size = 0;
    VkShaderStageFlags supported_stages = 0;
    VkSubgroupFeatureFlags supported_operations = 0;
    VkBool32 quad_operations_in_all_stages = false;
    bool size_control_is_supported = false;

    uint32_t supported_stages_flags_rd() const;
    String supported_stages_desc() const;
    uint32_t supported_operations_flags_rd() const;
    String supported_operations_desc() const;
  };
  struct PipelineCacheControlCapabilities {
    bool is_supported;
  };

  struct DeviceFunctions {
    PFN_vkCreateSwapchainKHR CreateSwapchainKHR = nullptr;
    PFN_vkDestroySwapchainKHR DestroySwapchainKHR = nullptr;
    PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR = nullptr;
    PFN_vkAcquireNextImageKHR AcquireNextImageKHR = nullptr;
    PFN_vkQueuePresentKHR QueuePresentKHR = nullptr;
    PFN_vkCreateRenderPass2KHR CreateRenderPass2KHR = nullptr;
  };
  /// --- device ---
  VkDevice vk_device = VK_NULL_HANDLE;
  RenderingContextDriverVulkan* context_driver = nullptr;
  RenderingContextDriver::Device context_device = {};
  VkPhysicalDevice physical_device = {};
  VkPhysicalDeviceFeatures physical_device_features = {};
  VkPhysicalDeviceFeatures requested_device_features = {};
  RDD::Capabilities device_capabilities;
  SubgroupCapabilities subgroup_capabilities;
  MultiviewCapabilities multiview_capabilities;
  VRSCapabilities vrs_capabilities;
  StorageBufferCapabilities storage_buffer_capabilities;
  ShaderCapabilities shader_capabilities;
  PipelineCacheControlCapabilities pipeline_cache_control_capabilities;
  DeviceFunctions device_functions;
  /// --- memory ---
  VmaAllocator allocator = nullptr;
  HashMap<uint32_t, VmaPool> small_allocs_pools;  // index to pool
  VmaPool _find_or_create_small_allocs_pool(uint32_t p_mem_type_index);

  /// --extensions ---
  VkPhysicalDeviceProperties physical_device_properties = {};
  HashSet<CharString> enabled_device_extension_names;
  HashMap<CharString, bool> requested_device_extensions;

  // 我觉得这个形式比较好
  struct QueueFamilyInfo {
    TightLocalVector<Queue> physical_queue;
    VkQueueFamilyProperties properties;
  };
  /// --- queue ---
  TightLocalVector<TightLocalVector<Queue>> queue_families;  // --- each queue family may have more than one queue.
  TightLocalVector<VkQueueFamilyProperties> queue_family_properties;

  /// --- frame ---
  uint32_t frame_count = 1;
  uint32_t present_frame_latency = 0;

  virtual Error initialize(uint32_t p_device_index, uint32_t p_frame_count);
  void finatialize();

  virtual void set_object_name(ObjectType p_type, ID p_driver_id, const String& p_name) override;

 private:
  /// ---tools function ---
  Error _initialize_device_extensions();
  Error _check_device_features();
  Error _check_device_capabilities();
  void _register_requested_device_extension(const CharString&, bool);
  /// --- queue create ---
  Error _add_queue_create_info(LocalVector<VkDeviceQueueCreateInfo>& r_queue_create_info);
  /// --- device---
  Error _initialize_device(const LocalVector<VkDeviceQueueCreateInfo>& p_queue_create_info);
  Error _initialize_allocator();
  Error _initialize_pipeline_cache();
  void _set_object_name(VkObjectType p_object_type, uint64_t p_object_handle, String p_object_name);
  void _swap_chain_release(SwapChain* swap_chain);
  VkResult _create_render_pass(VkDevice p_device, const VkRenderPassCreateInfo2* p_create_info,
                               const VkAllocationCallbacks* p_allocator, VkRenderPass* p_render_pass);
  // used in swap_chain recreate.
  bool _release_image_semaphore(CommandQueue* p_command_queue, uint32_t p_semaphore_index, bool p_release_on_swap_chain);
  bool _recreate_image_semaphore(CommandQueue* p_command_queue, uint32_t p_semaphore_index, bool p_release_on_swap_chain);

 public:
  /// --- limits --- 
  const RDD::Capabilities& get_capabilities() const { return device_capabilities; }
  virtual const RDD::MultiviewCapabilities& get_multiview_capabilities() override final { return multiview_capabilities; }
  RenderingDeviceDriverVulkan(RenderingContextDriverVulkan* p_context_driver);
  virtual ~RenderingDeviceDriverVulkan();

 private:
  using VersatileResource = VersatileResourceTemplate<BufferInfo, TextureInfo, VertexFormatInfo, ShaderInfo, UniformSetInfo>;
  PagedAllocator<VersatileResource> resources_allocator;
  // VersatileResource::allocate<Type>(resource_allocator)
};

}  // namespace lain::graphics

#endif  // RENDERING_DEVICE_DRIVER_VULKAN_H
