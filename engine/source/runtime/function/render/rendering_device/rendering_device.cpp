#include "rendering_device.h"
#include "_generated/serializer/all_serializer.h"
#include "core/engine/engine.h"
#include "core/io/dir_access.h"

using namespace lain;
static const char* SHADER_UNIFORM_NAMES[RenderingDevice::UNIFORM_TYPE_MAX] = {
    "Sampler", "CombinedSampler", "Texture", "Image", "TextureBuffer", "SamplerTextureBuffer", "ImageBuffer", "UniformBuffer", "StorageBuffer", "InputAttachment"};

RenderingDevice* RenderingDevice::singleton = nullptr;
RenderingDevice::ShaderCompileToSPIRVFunction RenderingDevice::compile_to_spirv_function = nullptr;
RenderingDevice::ShaderCacheFunction RenderingDevice::cache_function = nullptr;
RenderingDevice::ShaderSPIRVGetCacheKeyFunction RenderingDevice::get_spirv_cache_key_function = nullptr;

// When true, the command graph will attempt to reorder the rendering commands submitted by the user based on the dependencies detected from
// the commands automatically. This should improve rendering performance in most scenarios at the cost of some extra CPU overhead.
//
// This behavior can be disabled if it's suspected that the graph is not detecting dependencies correctly and more control over the order of
// the commands is desired (e.g. debugging).

#define RENDER_GRAPH_REORDER 1

// Synchronization barriers are issued between the graph's levels only with the necessary amount of detail to achieve the correct result. If
// it's suspected that the graph is not doing this correctly, full barriers can be issued instead that will block all types of operations
// between the synchronization levels. This setting will have a very negative impact on performance when enabled, so it's only intended for
// debugging purposes.

#define RENDER_GRAPH_FULL_BARRIERS 0

// The command graph can automatically issue secondary command buffers and record them on background threads when they reach an arbitrary
// size threshold. This can be very beneficial towards reducing the time the main thread takes to record all the rendering commands. However,
// this setting is not enabled by default as it's been shown to cause some strange issues with certain IHVs that have yet to be understood.

/***************************/
/**** ID INFRASTRUCTURE ****/
/***************************/
// 利用reverse_map，在删除时可以快速找到哪些资源被这个资源依赖，从而删除
void RenderingDevice::_add_dependency(RID p_id, RID p_depends_on) {
  if (!dependency_map.has(p_depends_on)) {
    dependency_map[p_depends_on] = HashSet<RID>();
  }

  dependency_map[p_depends_on].insert(p_id);

  if (!reverse_dependency_map.has(p_id)) {
    reverse_dependency_map[p_id] = HashSet<RID>();
  }

  reverse_dependency_map[p_id].insert(p_depends_on);
}

void RenderingDevice::_free_dependencies(RID p_id) {
  // Direct dependencies must be freed.

  HashMap<RID, HashSet<RID>>::Iterator E = dependency_map.find(p_id);
  if (E) {
    while (E->value.size()) {
      free(*E->value.begin());
    }
    dependency_map.remove(E);
  }

  // Reverse dependencies must be unreferenced.
  E = reverse_dependency_map.find(p_id);

  if (E) {
    for (const RID& F : E->value) {
      HashMap<RID, HashSet<RID>>::Iterator G = dependency_map.find(F);
      ERR_CONTINUE(!G);  // 说明建图发生了错误，但是仍然continue
      ERR_CONTINUE(!G->value.has(p_id));
      G->value.erase(p_id);
    }

    reverse_dependency_map.remove(E);  // 不再需要挨个free了
  }
}

static String _get_device_vendor_name(const RenderingContextDriver::Device& p_device) {
  switch (p_device.vendor) {
    case RenderingContextDriver::VENDOR_AMD:
      return "AMD";
    case RenderingContextDriver::VENDOR_IMGTEC:
      return "ImgTec";
    case RenderingContextDriver::VENDOR_APPLE:
      return "Apple";
    case RenderingContextDriver::VENDOR_NVIDIA:
      return "NVIDIA";
    case RenderingContextDriver::VENDOR_ARM:
      return "ARM";
    case RenderingContextDriver::VENDOR_MICROSOFT:
      return "Microsoft";
    case RenderingContextDriver::VENDOR_QUALCOMM:
      return "Qualcomm";
    case RenderingContextDriver::VENDOR_INTEL:
      return "Intel";
    default:
      return "Unknown";
  }
}
static String _get_device_type_name(const RenderingContextDriver::Device& p_device) {
  switch (p_device.type) {
    case RenderingContextDriver::DEVICE_TYPE_INTEGRATED_GPU:
      return "Integrated";
    case RenderingContextDriver::DEVICE_TYPE_DISCRETE_GPU:
      return "Discrete";
    case RenderingContextDriver::DEVICE_TYPE_VIRTUAL_GPU:
      return "Virtual";
    case RenderingContextDriver::DEVICE_TYPE_CPU:
      return "CPU";
    case RenderingContextDriver::DEVICE_TYPE_OTHER:
    default:
      return "Other";
  }
}
static uint32_t _get_device_type_score(const RenderingContextDriver::Device& p_device) {
  switch (p_device.type) {
    case RenderingContextDriver::DEVICE_TYPE_INTEGRATED_GPU:
      return 4;
    case RenderingContextDriver::DEVICE_TYPE_DISCRETE_GPU:
      return 5;
    case RenderingContextDriver::DEVICE_TYPE_VIRTUAL_GPU:
      return 3;
    case RenderingContextDriver::DEVICE_TYPE_CPU:
      return 2;
    case RenderingContextDriver::DEVICE_TYPE_OTHER:
    default:
      return 1;
  }
}

/***************************/
/**** BUFFER MANAGEMENT ****/
/***************************/

Error RenderingDevice::_insert_staging_block() {
  StagingBufferBlock block;
  // 每次分配一个staging_buffer_block_size大小的buffer，staging_buffer_block_size记载在project_settings
  block.driver_id = driver->buffer_create(staging_buffer_block_size, RDD::BUFFER_USAGE_TRANSFER_FROM_BIT, RDD::MEMORY_ALLOCATION_TYPE_CPU);
  ERR_FAIL_COND_V(!block.driver_id, ERR_CANT_CREATE);

  // block.frame_used = 0;
  // block.fill_amount = 0;

  staging_buffer_blocks.insert(staging_buffer_current, block);
  return OK;
}
/*********************/
/**** (RenderPass) ****/
/*********************/
bool RenderingDevice::FramebufferFormatKey::operator<(const RenderingDevice::FramebufferFormatKey& p_key) const {
  if (view_count != p_key.view_count) {
    return view_count < p_key.view_count;
  }

  uint32_t pass_size = passes.size();
  uint32_t key_pass_size = p_key.passes.size();
  if (pass_size != key_pass_size) {
    return pass_size < key_pass_size;
  }
  const FramebufferPass* pass_ptr = passes.ptr();
  const FramebufferPass* key_pass_ptr = p_key.passes.ptr();

  for (uint32_t i = 0; i < pass_size; i++) {
    {  // Compare color attachments.
      uint32_t attachment_size = pass_ptr[i].color_attachments.size();
      uint32_t key_attachment_size = key_pass_ptr[i].color_attachments.size();
      if (attachment_size != key_attachment_size) {
        return attachment_size < key_attachment_size;
      }
      const int32_t* pass_attachment_ptr = pass_ptr[i].color_attachments.ptr();
      const int32_t* key_pass_attachment_ptr = key_pass_ptr[i].color_attachments.ptr();

      for (uint32_t j = 0; j < attachment_size; j++) {
        if (pass_attachment_ptr[j] != key_pass_attachment_ptr[j]) {
          return pass_attachment_ptr[j] < key_pass_attachment_ptr[j];
        }
      }
    }
    {  // Compare input attachments.
      uint32_t attachment_size = pass_ptr[i].input_attachments.size();
      uint32_t key_attachment_size = key_pass_ptr[i].input_attachments.size();
      if (attachment_size != key_attachment_size) {
        return attachment_size < key_attachment_size;
      }
      const int32_t* pass_attachment_ptr = pass_ptr[i].input_attachments.ptr();
      const int32_t* key_pass_attachment_ptr = key_pass_ptr[i].input_attachments.ptr();

      for (uint32_t j = 0; j < attachment_size; j++) {
        if (pass_attachment_ptr[j] != key_pass_attachment_ptr[j]) {
          return pass_attachment_ptr[j] < key_pass_attachment_ptr[j];
        }
      }
    }
    {  // Compare resolve attachments.
      uint32_t attachment_size = pass_ptr[i].resolve_attachments.size();
      uint32_t key_attachment_size = key_pass_ptr[i].resolve_attachments.size();
      if (attachment_size != key_attachment_size) {
        return attachment_size < key_attachment_size;
      }
      const int32_t* pass_attachment_ptr = pass_ptr[i].resolve_attachments.ptr();
      const int32_t* key_pass_attachment_ptr = key_pass_ptr[i].resolve_attachments.ptr();

      for (uint32_t j = 0; j < attachment_size; j++) {
        if (pass_attachment_ptr[j] != key_pass_attachment_ptr[j]) {
          return pass_attachment_ptr[j] < key_pass_attachment_ptr[j];
        }
      }
    }
    {  // Compare preserve attachments.
      uint32_t attachment_size = pass_ptr[i].preserve_attachments.size();
      uint32_t key_attachment_size = key_pass_ptr[i].preserve_attachments.size();
      if (attachment_size != key_attachment_size) {
        return attachment_size < key_attachment_size;
      }
      const int32_t* pass_attachment_ptr = pass_ptr[i].preserve_attachments.ptr();
      const int32_t* key_pass_attachment_ptr = key_pass_ptr[i].preserve_attachments.ptr();

      for (uint32_t j = 0; j < attachment_size; j++) {
        if (pass_attachment_ptr[j] != key_pass_attachment_ptr[j]) {
          return pass_attachment_ptr[j] < key_pass_attachment_ptr[j];
        }
      }
    }
    if (pass_ptr[i].depth_attachment != key_pass_ptr[i].depth_attachment) {
      return pass_ptr[i].depth_attachment < key_pass_ptr[i].depth_attachment;
    }
  }

  int as = attachment_formats.size();
  int bs = p_key.attachment_formats.size();
  if (as != bs) {
    return as < bs;
  }

  const AttachmentFormat* af_a = attachment_formats.ptr();
  const AttachmentFormat* af_b = p_key.attachment_formats.ptr();
  for (int i = 0; i < as; i++) {
    const AttachmentFormat& a = af_a[i];
    const AttachmentFormat& b = af_b[i];
    if (a.format != b.format) {
      return a.format < b.format;
    }
    if (a.samples != b.samples) {
      return a.samples < b.samples;
    }
    if (a.usage_flags != b.usage_flags) {
      return a.usage_flags < b.usage_flags;
    }
  }

  return false;  // Equal.
}

RID RenderingDevice::texture_create(const TextureFormat& p_format, const TextureView& p_view, const Vector<Vector<uint8_t>>& p_data) {
  _THREAD_SAFE_METHOD_

  // Some adjustments will happen.
  TextureFormat format = p_format;

  if (format.shareable_formats.size()) {
    ERR_FAIL_COND_V_MSG(format.shareable_formats.find(format.format) == -1, RID(), "If supplied a list of shareable formats, the current format must be present in the list");
    ERR_FAIL_COND_V_MSG(p_view.format_override != DATA_FORMAT_MAX && format.shareable_formats.find(p_view.format_override) == -1, RID(),
                        "If supplied a list of shareable formats, the current view format override "
                        "must be present in the list");
  }

  ERR_FAIL_INDEX_V(format.texture_type, RDD::TEXTURE_TYPE_MAX, RID());

  ERR_FAIL_COND_V_MSG(format.width < 1, RID(), "Width must be equal or greater than 1 for all textures");

  if (format.texture_type != TEXTURE_TYPE_1D && format.texture_type != TEXTURE_TYPE_1D_ARRAY) {
    ERR_FAIL_COND_V_MSG(format.height < 1, RID(), "Height must be equal or greater than 1 for 2D and 3D textures");
  }

  if (format.texture_type == TEXTURE_TYPE_3D) {
    ERR_FAIL_COND_V_MSG(format.depth < 1, RID(), "Depth must be equal or greater than 1 for 3D textures");
  }

  ERR_FAIL_COND_V(format.mipmaps < 1, RID());

  if (format.texture_type == TEXTURE_TYPE_1D_ARRAY || format.texture_type == TEXTURE_TYPE_2D_ARRAY || format.texture_type == TEXTURE_TYPE_CUBE_ARRAY ||
      format.texture_type == TEXTURE_TYPE_CUBE) {
    ERR_FAIL_COND_V_MSG(format.array_layers < 1, RID(), "Amount of layers must be equal or greater than 1 for arrays and cubemaps.");
    ERR_FAIL_COND_V_MSG((format.texture_type == TEXTURE_TYPE_CUBE_ARRAY || format.texture_type == TEXTURE_TYPE_CUBE) && (format.array_layers % 6) != 0, RID(),
                        "Cubemap and cubemap array textures must provide a layer number that is multiple of 6");
  } else {
    format.array_layers = 1;
  }

  ERR_FAIL_INDEX_V(format.samples, TEXTURE_SAMPLES_MAX, RID());

  format.height = format.texture_type != TEXTURE_TYPE_1D && format.texture_type != TEXTURE_TYPE_1D_ARRAY ? format.height : 1;
  format.depth = format.texture_type == TEXTURE_TYPE_3D ? format.depth : 1;

  uint32_t required_mipmaps = get_image_required_mipmaps(format.width, format.height, format.depth);

  ERR_FAIL_COND_V_MSG(required_mipmaps < format.mipmaps, RID(),
                      "Too many mipmaps requested for texture format and dimensions (" + itos(format.mipmaps) + "), maximum allowed: (" + itos(required_mipmaps) + ").");

  uint32_t forced_usage_bits = 0;
  if (p_data.size()) {
    ERR_FAIL_COND_V_MSG(p_data.size() != (int)format.array_layers, RID(),
                        "Default supplied data for image format is of invalid length (" + itos(p_data.size()) + "), should be (" + itos(format.array_layers) + ").");

    for (uint32_t i = 0; i < format.array_layers; i++) {
      uint32_t required_size = get_image_format_required_size(format.format, format.width, format.height, format.depth, format.mipmaps);
      ERR_FAIL_COND_V_MSG((uint32_t)p_data[i].size() != required_size, RID(),
                          "Data for slice index " + itos(i) + " (mapped to layer " + itos(i) + ") differs in size (supplied: " + itos(p_data[i].size()) +
                              ") than what is required by the format (" + itos(required_size) + ").");
    }

    if (!(format.usage_bits & TEXTURE_USAGE_CAN_UPDATE_BIT)) {
      forced_usage_bits = TEXTURE_USAGE_CAN_UPDATE_BIT;
    }
  }

  {
    // Validate that this image is supported for the intended use.
    bool cpu_readable = (format.usage_bits & RDD::TEXTURE_USAGE_CPU_READ_BIT);
    BitField<RDD::TextureUsageBits> supported_usage = driver->texture_get_usages_supported_by_format(format.format, cpu_readable);

    String format_text = "'" + String(FORMAT_NAMES[format.format]) + "'";
    auto valid = [&](RDD::TextureUsageBits usage) -> bool {
      if (format.usage_bits & usage && !supported_usage.has_flag(usage)) {
        ERR_FAIL_V_MSG(false, "Format " + format_text + " does not support usage as " + Serializer::write(usage).string_value());
      }
      return true;
    };
    bool is_valid = false;
    is_valid = valid(TEXTURE_USAGE_SAMPLING_BIT);
    is_valid = valid(TEXTURE_USAGE_COLOR_ATTACHMENT_BIT);  
    is_valid = valid(TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    is_valid = valid(TEXTURE_USAGE_STORAGE_BIT);
    is_valid = valid(TEXTURE_USAGE_STORAGE_ATOMIC_BIT);
    is_valid = valid(TEXTURE_USAGE_VRS_ATTACHMENT_BIT);
    ERR_FAIL_COND_V(!is_valid, RID());
  }

  // Transfer and validate view info.
  // 构造向下的TextureView
  RDD::TextureView tv;
  if (p_view.format_override == DATA_FORMAT_MAX) {
    tv.format = format.format;
  } else {
    ERR_FAIL_INDEX_V(p_view.format_override, DATA_FORMAT_MAX, RID());
    tv.format = p_view.format_override;
  }
  ERR_FAIL_INDEX_V(p_view.swizzle_r, TEXTURE_SWIZZLE_MAX, RID());
  ERR_FAIL_INDEX_V(p_view.swizzle_g, TEXTURE_SWIZZLE_MAX, RID());
  ERR_FAIL_INDEX_V(p_view.swizzle_b, TEXTURE_SWIZZLE_MAX, RID());
  ERR_FAIL_INDEX_V(p_view.swizzle_a, TEXTURE_SWIZZLE_MAX, RID());
  tv.swizzle_r = p_view.swizzle_r;
  tv.swizzle_g = p_view.swizzle_g;
  tv.swizzle_b = p_view.swizzle_b;
  tv.swizzle_a = p_view.swizzle_a;

  // Create.

  Texture texture;
  format.usage_bits |= forced_usage_bits;
  texture.driver_id = driver->texture_create(format, tv);  // 从这里能看出来，format 和 tv 是数据通信的方式，各种不同的texture只是在不同层级中保存信息
  ERR_FAIL_COND_V(!texture.driver_id, RID());
  texture.type = format.texture_type;
  texture.format = format.format;
  texture.width = format.width;
  texture.height = format.height;
  texture.depth = format.depth;
  texture.layers = format.array_layers;
  texture.mipmaps = format.mipmaps;
  texture.base_mipmap = 0;
  texture.base_layer = 0;
  texture.is_resolve_buffer = format.is_resolve_buffer;
  texture.usage_flags = format.usage_bits & ~forced_usage_bits;
  texture.samples = format.samples;
  texture.allowed_shared_formats = format.shareable_formats;
  texture.has_initial_data = !p_data.is_empty();

  if ((format.usage_bits & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
    texture.read_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_DEPTH_BIT);
    texture.barrier_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_DEPTH_BIT);
    if (format_has_stencil(format.format)) {
      texture.barrier_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_STENCIL_BIT);
    }
  } else {
    texture.read_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_COLOR_BIT);
    texture.barrier_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_COLOR_BIT);
  }

  texture.bound = false;

  // Textures are only assumed to be immutable if they have initial data and none of the other bits that indicate write usage are enabled.
  bool texture_mutable_by_default = texture.usage_flags & (TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | TEXTURE_USAGE_STORAGE_BIT |
                                                           TEXTURE_USAGE_STORAGE_ATOMIC_BIT | TEXTURE_USAGE_VRS_ATTACHMENT_BIT);
  if (p_data.is_empty() || texture_mutable_by_default) {
    _texture_make_mutable(&texture, RID());  // w: for tracker
  }

  texture_memory += driver->texture_get_allocation_size(texture.driver_id);

  RID id = texture_owner.make_rid(texture);
#ifdef DEV_ENABLED
  driver->set_object_name(RDD::OBJECT_TYPE_TEXTURE, texture.driver_id, itos(id.get_id()));
#endif
  if (p_data.size()) {
    for (uint32_t i = 0; i < p_format.array_layers; i++) {
      _texture_update(id, i, p_data[i], true, false);
    }

    if (texture.draw_tracker != nullptr) {
      // Draw tracker can assume the texture will be in transfer destination.
      texture.draw_tracker->usage = RDG::ResourceUsage::RESOURCE_USAGE_COPY_TO;
    }
  }

  return id;
}
RD::TextureFormat RenderingDevice::texture_get_format(RID p_texture) {
  _THREAD_SAFE_METHOD_

  Texture* tex = texture_owner.get_or_null(p_texture);
  ERR_FAIL_NULL_V(tex, TextureFormat());

  RD::TextureFormat tf;

  tf.format = tex->format;
  tf.width = tex->width;
  tf.height = tex->height;
  tf.depth = tex->depth;
  tf.array_layers = tex->layers;
  tf.mipmaps = tex->mipmaps;
  tf.texture_type = tex->type;
  tf.samples = tex->samples;
  tf.usage_bits = tex->usage_flags;
  tf.shareable_formats = tex->allowed_shared_formats;
  tf.is_resolve_buffer = tex->is_resolve_buffer;

  return tf;
}
RID lain::RenderingDevice::texture_create_shared(const TextureView& p_view, RID p_with_texture) {
  _THREAD_SAFE_METHOD_

  Texture* src_texture = texture_owner.get_or_null(p_with_texture);
  ERR_FAIL_NULL_V(src_texture, RID());

  if (src_texture->owner.is_valid()) {  // Ahh this is a share. The RenderingDeviceDriver needs the actual owner.
    p_with_texture = src_texture->owner;
    src_texture = texture_owner.get_or_null(src_texture->owner);
    ERR_FAIL_NULL_V(src_texture, RID());  // This is a bug.
  }

  // Create view.

  Texture texture = *src_texture;  // 根据原texture创建一个新的texture，修改这个
  texture.shared_fallback = nullptr;

  RDD::TextureView tv;
  bool create_shared = true;
  bool raw_reintepretation = false;
  if (p_view.format_override == DATA_FORMAT_MAX || p_view.format_override == texture.format) {
    tv.format = texture.format;  // 创建原 format的tv
  } else {                       // 需要验证
    ERR_FAIL_INDEX_V(p_view.format_override, DATA_FORMAT_MAX, RID());

    ERR_FAIL_COND_V_MSG(!texture.allowed_shared_formats.has(p_view.format_override), RID(),
                        "Format override is not in the list of allowed shareable formats for original texture.");
    tv.format = p_view.format_override;
    create_shared = driver->texture_can_make_shared_with_format(texture.driver_id, p_view.format_override, raw_reintepretation);
  }
  tv.swizzle_r = p_view.swizzle_r;
  tv.swizzle_g = p_view.swizzle_g;
  tv.swizzle_b = p_view.swizzle_b;
  tv.swizzle_a = p_view.swizzle_a;

  if (create_shared) {
    texture.driver_id = driver->texture_create_shared(texture.driver_id, tv);
  } else {
    // The regular view will use the same format as the main texture.
    RDD::TextureView regular_view = tv;
    regular_view.format = src_texture->format;
    texture.driver_id = driver->texture_create_shared(texture.driver_id, regular_view);

    // Create the independent texture for the alias.
    RDD::TextureFormat alias_format = texture.texture_format();
    alias_format.format = tv.format;
    alias_format.usage_bits = TEXTURE_USAGE_SAMPLING_BIT | TEXTURE_USAGE_CAN_COPY_TO_BIT;

    _texture_check_shared_fallback(src_texture);
    _texture_check_shared_fallback(&texture);

    texture.shared_fallback->texture = driver->texture_create(alias_format, tv);
    texture.shared_fallback->raw_reinterpretation = raw_reintepretation;
    texture_memory += driver->texture_get_allocation_size(texture.shared_fallback->texture);

    RDG::ResourceTracker* tracker = RDG::resource_tracker_create();
    tracker->texture_driver_id = texture.shared_fallback->texture;
    tracker->texture_subresources = texture.barrier_range();
    tracker->texture_usage = alias_format.usage_bits;
    tracker->ref_count = 1;
    texture.shared_fallback->texture_tracker = tracker;
    texture.shared_fallback->revision = 0;

    if (raw_reintepretation && src_texture->shared_fallback->buffer.id == 0) {
      // For shared textures of the same size, we create the buffer on the main texture if it doesn't have it already.
      _texture_create_reinterpret_buffer(src_texture);
    }
  }

  ERR_FAIL_COND_V(!texture.driver_id, RID());

  texture.slice_trackers.clear();

  if (texture.draw_tracker != nullptr) {
    texture.draw_tracker->ref_count++;
  }

  texture.owner = p_with_texture;
  RID id = texture_owner.make_rid(texture);
#ifdef DEV_ENABLED
  set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
  _add_dependency(id, p_with_texture);

  return id;
}

RID lain::RenderingDevice::texture_create_shared_from_slice(const TextureView& p_view, RID p_with_texture, uint32_t p_layer, uint32_t p_mipmap, uint32_t p_mipmaps,
                                                            TextureSliceType p_slice_type, uint32_t p_layers) {
  _THREAD_SAFE_METHOD_

  Texture* src_texture = texture_owner.get_or_null(p_with_texture);
  ERR_FAIL_NULL_V(src_texture, RID());

  if (src_texture->owner.is_valid()) {  // // Ahh this is a share. The RenderingDeviceDriver needs the actual owner.
    p_with_texture = src_texture->owner;
    src_texture = texture_owner.get_or_null(src_texture->owner);
    ERR_FAIL_NULL_V(src_texture, RID());  // This is a bug.
  }

  ERR_FAIL_COND_V_MSG(p_slice_type == TEXTURE_SLICE_CUBEMAP && (src_texture->type != TEXTURE_TYPE_CUBE && src_texture->type != TEXTURE_TYPE_CUBE_ARRAY), RID(),
                      "Can only create a cubemap slice from a cubemap or cubemap array mipmap");

  ERR_FAIL_COND_V_MSG(p_slice_type == TEXTURE_SLICE_3D && src_texture->type != TEXTURE_TYPE_3D, RID(), "Can only create a 3D slice from a 3D texture");

  ERR_FAIL_COND_V_MSG(p_slice_type == TEXTURE_SLICE_2D_ARRAY && (src_texture->type != TEXTURE_TYPE_2D_ARRAY), RID(), "Can only create an array slice from a 2D array mipmap");

  // Create view.

  ERR_FAIL_UNSIGNED_INDEX_V(p_mipmap, src_texture->mipmaps, RID());
  ERR_FAIL_COND_V(p_mipmap + p_mipmaps > src_texture->mipmaps, RID());
  ERR_FAIL_UNSIGNED_INDEX_V(p_layer, src_texture->layers, RID());

  int slice_layers = 1;
  if (p_layers != 0) {
    ERR_FAIL_COND_V_MSG(p_layers > 1 && p_slice_type != TEXTURE_SLICE_2D_ARRAY, RID(), "layer slicing only supported for 2D arrays");
    ERR_FAIL_COND_V_MSG(p_layer + p_layers > src_texture->layers, RID(), "layer slice is out of bounds");
    slice_layers = p_layers;
  } else if (p_slice_type == TEXTURE_SLICE_2D_ARRAY) {
    ERR_FAIL_COND_V_MSG(p_layer != 0, RID(), "layer must be 0 when obtaining a 2D array mipmap slice");
    slice_layers = src_texture->layers;
  } else if (p_slice_type == TEXTURE_SLICE_CUBEMAP) {
    slice_layers = 6;
  }

  Texture texture = *src_texture;
  texture.shared_fallback = nullptr;

  get_image_format_required_size(texture.format, texture.width, texture.height, texture.depth, p_mipmap + 1, &texture.width, &texture.height);
  texture.mipmaps = p_mipmaps;
  texture.layers = slice_layers;
  texture.base_mipmap = p_mipmap;
  texture.base_layer = p_layer;

  if (p_slice_type == TEXTURE_SLICE_2D) {
    texture.type = TEXTURE_TYPE_2D;
  } else if (p_slice_type == TEXTURE_SLICE_3D) {
    texture.type = TEXTURE_TYPE_3D;
  }

  RDD::TextureView tv;
  bool create_shared = true;
  bool raw_reintepretation = false;
  if (p_view.format_override == DATA_FORMAT_MAX || p_view.format_override == texture.format) {
    tv.format = texture.format;
  } else {
    ERR_FAIL_INDEX_V(p_view.format_override, DATA_FORMAT_MAX, RID());

    ERR_FAIL_COND_V_MSG(!texture.allowed_shared_formats.has(p_view.format_override), RID(),
                        "Format override is not in the list of allowed shareable formats for original texture.");
    tv.format = p_view.format_override;
    create_shared = driver->texture_can_make_shared_with_format(texture.driver_id, p_view.format_override, raw_reintepretation);
  }

  tv.swizzle_r = p_view.swizzle_r;
  tv.swizzle_g = p_view.swizzle_g;
  tv.swizzle_b = p_view.swizzle_b;
  tv.swizzle_a = p_view.swizzle_a;

  if (p_slice_type == TEXTURE_SLICE_CUBEMAP) {
    ERR_FAIL_COND_V_MSG(p_layer >= src_texture->layers, RID(), "Specified layer is invalid for cubemap");
    ERR_FAIL_COND_V_MSG((p_layer % 6) != 0, RID(), "Specified layer must be a multiple of 6.");
  }

  if (create_shared) {
    texture.driver_id = driver->texture_create_shared_from_slice(src_texture->driver_id, tv, p_slice_type, p_layer, slice_layers, p_mipmap, p_mipmaps);
  } else {
    // The regular view will use the same format as the main texture.
    RDD::TextureView regular_view = tv;
    regular_view.format = src_texture->format;
    texture.driver_id = driver->texture_create_shared_from_slice(src_texture->driver_id, regular_view, p_slice_type, p_layer, slice_layers, p_mipmap, p_mipmaps);

    // Create the independent texture for the slice.
    RDD::TextureSubresourceRange slice_range = texture.barrier_range();
    slice_range.base_mipmap = 0;
    slice_range.base_layer = 0;

    RDD::TextureFormat slice_format = texture.texture_format();
    slice_format.width = MAX(texture.width >> p_mipmap, 1U);
    slice_format.height = MAX(texture.height >> p_mipmap, 1U);
    slice_format.depth = MAX(texture.depth >> p_mipmap, 1U);
    slice_format.format = tv.format;
    slice_format.usage_bits = TEXTURE_USAGE_SAMPLING_BIT | TEXTURE_USAGE_CAN_COPY_TO_BIT;

    _texture_check_shared_fallback(src_texture);
    _texture_check_shared_fallback(&texture);

    texture.shared_fallback->texture = driver->texture_create(slice_format, tv);
    texture.shared_fallback->raw_reinterpretation = raw_reintepretation;
    texture_memory += driver->texture_get_allocation_size(texture.shared_fallback->texture);

    RDG::ResourceTracker* tracker = RDG::resource_tracker_create();
    tracker->texture_driver_id = texture.shared_fallback->texture;
    tracker->texture_subresources = slice_range;
    tracker->texture_usage = slice_format.usage_bits;
    tracker->ref_count = 1;
    texture.shared_fallback->texture_tracker = tracker;
    texture.shared_fallback->revision = 0;

    if (raw_reintepretation && src_texture->shared_fallback->buffer.id == 0) {
      // For shared texture slices, we create the buffer on the slice if the source texture has no reinterpretation buffer.
      _texture_create_reinterpret_buffer(&texture);
    }
  }

  ERR_FAIL_COND_V(!texture.driver_id, RID());

  const Rect2i slice_rect(p_mipmap, p_layer, p_mipmaps, slice_layers);
  texture.owner = p_with_texture;
  texture.slice_type = p_slice_type;
  texture.slice_rect = slice_rect;

  // If parent is mutable, make slice mutable by default.
  if (src_texture->draw_tracker != nullptr) {
    texture.draw_tracker = nullptr;
    _texture_make_mutable(&texture, RID());
  }

  RID id = texture_owner.make_rid(texture);
#ifdef DEV_ENABLED
  set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
  _add_dependency(id, p_with_texture);

  return id;
}

Error lain::RenderingDevice::texture_update(RID p_texture, uint32_t p_layer, const Vector<uint8_t>& p_data) {
  return _texture_update(p_texture, p_layer, p_data, false, true);
}

Vector<uint8_t> lain::RenderingDevice::texture_get_data(RID p_texture, uint32_t p_layer) {
  _THREAD_SAFE_METHOD_

  Texture* tex = texture_owner.get_or_null(p_texture);
  ERR_FAIL_NULL_V(tex, Vector<uint8_t>());

  ERR_FAIL_COND_V_MSG(tex->bound, Vector<uint8_t>(),
                      "Texture can't be retrieved while a draw list that uses it as part of a framebuffer is being created. Ensure the draw list is finalized (and that the "
                      "color/depth texture using it is not set to `RenderingDevice.FINAL_ACTION_CONTINUE`) to retrieve this texture.");
  ERR_FAIL_COND_V_MSG(!(tex->usage_flags & TEXTURE_USAGE_CAN_COPY_FROM_BIT), Vector<uint8_t>(),
                      "Texture requires the `RenderingDevice.TEXTURE_USAGE_CAN_COPY_FROM_BIT` to be set to be retrieved.");

  ERR_FAIL_COND_V(p_layer >= tex->layers, Vector<uint8_t>());
  if ((tex->usage_flags & TEXTURE_USAGE_CPU_READ_BIT)) {
    // Does not need anything fancy, map and read.
    return _texture_get_data(tex, p_layer);
  } else {
    LocalVector<RDD::TextureCopyableLayout> mip_layouts;
    uint32_t work_mip_alignment = driver->api_trait_get(RDD::API_TRAIT_TEXTURE_TRANSFER_ALIGNMENT);
    uint32_t work_buffer_size = 0;
    mip_layouts.resize(tex->mipmaps);
    for (uint32_t i = 0; i < tex->mipmaps; i++) {
      RDD::TextureSubresource subres;
      subres.aspect = RDD::TEXTURE_ASPECT_COLOR;
      subres.layer = p_layer;
      subres.mipmap = i;
      driver->texture_get_copyable_layout(tex->driver_id, subres, &mip_layouts[i]);

      // Assuming layers are tightly packed. If this is not true on some driver, we must modify the copy algorithm.
      DEV_ASSERT(mip_layouts[i].layer_pitch == mip_layouts[i].size / tex->layers);

      work_buffer_size = STEPIFY(work_buffer_size, work_mip_alignment) + mip_layouts[i].size;
    }

    RDD::BufferID tmp_buffer = driver->buffer_create(work_buffer_size, RDD::BUFFER_USAGE_TRANSFER_TO_BIT, RDD::MEMORY_ALLOCATION_TYPE_CPU);
    ERR_FAIL_COND_V(!tmp_buffer, Vector<uint8_t>());

    thread_local LocalVector<RDD::BufferTextureCopyRegion> command_buffer_texture_copy_regions_vector;
    command_buffer_texture_copy_regions_vector.clear();

    uint32_t w = tex->width;
    uint32_t h = tex->height;
    uint32_t d = tex->depth;
    for (uint32_t i = 0; i < tex->mipmaps; i++) {
      RDD::BufferTextureCopyRegion copy_region;
      copy_region.buffer_offset = mip_layouts[i].offset;
      copy_region.texture_subresources.aspect = tex->read_aspect_flags;
      copy_region.texture_subresources.mipmap = i;
      copy_region.texture_subresources.base_layer = p_layer;
      copy_region.texture_subresources.layer_count = 1;
      copy_region.texture_region_size.x = w;
      copy_region.texture_region_size.y = h;
      copy_region.texture_region_size.z = d;
      command_buffer_texture_copy_regions_vector.push_back(copy_region);

      w = (w >> 1);
      h = (h >> 1);
      d = MAX(1u, d >> 1);
    }

    if (_texture_make_mutable(tex, p_texture)) {
      // The texture must be mutable to be used as a copy source due to layout transitions.
      draw_graph.add_synchronization();
    }

    draw_graph.add_texture_get_data(tex->driver_id, tex->draw_tracker, tmp_buffer, command_buffer_texture_copy_regions_vector);

    // Flush everything so memory can be safely mapped.
    _flush_and_stall_for_all_frames();  // 如果是多线程？

    const uint8_t* read_ptr = driver->buffer_map(tmp_buffer);
    ERR_FAIL_NULL_V(read_ptr, Vector<uint8_t>());

    uint32_t block_w = 0;
    uint32_t block_h = 0;
    get_compressed_image_format_block_dimensions(tex->format, block_w, block_h);

    Vector<uint8_t> buffer_data;
    uint32_t tight_buffer_size = get_image_format_required_size(tex->format, tex->width, tex->height, tex->depth, tex->mipmaps);
    buffer_data.resize(tight_buffer_size);

    uint8_t* write_ptr = buffer_data.ptrw();

    w = tex->width;
    h = tex->height;
    d = tex->depth;
    for (uint32_t i = 0; i < tex->mipmaps; i++) {
      uint32_t width = 0, height = 0, depth = 0;
      uint32_t tight_mip_size = get_image_format_required_size(tex->format, w, h, d, 1, &width, &height, &depth);
      uint32_t tight_row_pitch = tight_mip_size / ((height / block_h) * depth);

      // Copy row-by-row to erase padding due to alignments.
      const uint8_t* rp = read_ptr;
      uint8_t* wp = write_ptr;
      for (uint32_t row = h * d / block_h; row != 0; row--) {
        memcpy(wp, rp, tight_row_pitch);
        rp += mip_layouts[i].row_pitch;
        wp += tight_row_pitch;
      }

      w = MAX(block_w, w >> 1);
      h = MAX(block_h, h >> 1);
      d = MAX(1u, d >> 1);
      read_ptr += mip_layouts[i].size;
      write_ptr += tight_mip_size;
    }

    driver->buffer_unmap(tmp_buffer);
    driver->buffer_free(tmp_buffer);

    return buffer_data;
  }
}

bool lain::RenderingDevice::texture_is_format_supported_for_usage(DataFormat p_format, BitField<TextureUsageBits> p_usage) const {
  ERR_FAIL_INDEX_V(p_format, DATA_FORMAT_MAX, false);

  _THREAD_SAFE_METHOD_

  bool cpu_readable = (p_usage & RDD::TEXTURE_USAGE_CPU_READ_BIT);
  BitField<TextureUsageBits> supported = driver->texture_get_usages_supported_by_format(p_format, cpu_readable);
  bool any_unsupported = (((int64_t)supported) | ((int64_t)p_usage)) != ((int64_t)supported);
  return !any_unsupported;
}

bool lain::RenderingDevice::texture_is_shared(RID p_texture) {
  _THREAD_SAFE_METHOD_

  Texture* tex = texture_owner.get_or_null(p_texture);
  ERR_FAIL_NULL_V(tex, false);
  return tex->owner.is_valid();
}

bool lain::RenderingDevice::texture_is_valid(RID p_texture) {
  return texture_owner.owns(p_texture);
}

Error RenderingDevice::texture_resolve_multisample(RID p_from_texture, RID p_to_texture) {
	_THREAD_SAFE_METHOD_

	Texture *src_tex = texture_owner.get_or_null(p_from_texture);
	ERR_FAIL_NULL_V(src_tex, ERR_INVALID_PARAMETER);

	ERR_FAIL_COND_V_MSG(src_tex->bound, ERR_INVALID_PARAMETER,
			"Source texture can't be copied while a draw list that uses it as part of a framebuffer is being created. Ensure the draw list is finalized (and that the color/depth texture using it is not set to `RenderingDevice.FINAL_ACTION_CONTINUE`) to copy this texture.");
	ERR_FAIL_COND_V_MSG(!(src_tex->usage_flags & TEXTURE_USAGE_CAN_COPY_FROM_BIT), ERR_INVALID_PARAMETER,
			"Source texture requires the `RenderingDevice.TEXTURE_USAGE_CAN_COPY_FROM_BIT` to be set to be retrieved.");

	ERR_FAIL_COND_V_MSG(src_tex->type != TEXTURE_TYPE_2D, ERR_INVALID_PARAMETER, "Source texture must be 2D (or a slice of a 3D/Cube texture)");
	ERR_FAIL_COND_V_MSG(src_tex->samples == TEXTURE_SAMPLES_1, ERR_INVALID_PARAMETER, "Source texture must be multisampled.");

	Texture *dst_tex = texture_owner.get_or_null(p_to_texture);
	ERR_FAIL_NULL_V(dst_tex, ERR_INVALID_PARAMETER);

	ERR_FAIL_COND_V_MSG(dst_tex->bound, ERR_INVALID_PARAMETER,
			"Destination texture can't be copied while a draw list that uses it as part of a framebuffer is being created. Ensure the draw list is finalized (and that the color/depth texture using it is not set to `RenderingDevice.FINAL_ACTION_CONTINUE`) to copy this texture.");
	ERR_FAIL_COND_V_MSG(!(dst_tex->usage_flags & TEXTURE_USAGE_CAN_COPY_TO_BIT), ERR_INVALID_PARAMETER,
			"Destination texture requires the `RenderingDevice.TEXTURE_USAGE_CAN_COPY_TO_BIT` to be set to be retrieved.");

	ERR_FAIL_COND_V_MSG(dst_tex->type != TEXTURE_TYPE_2D, ERR_INVALID_PARAMETER, "Destination texture must be 2D (or a slice of a 3D/Cube texture).");
	ERR_FAIL_COND_V_MSG(dst_tex->samples != TEXTURE_SAMPLES_1, ERR_INVALID_PARAMETER, "Destination texture must not be multisampled.");

	ERR_FAIL_COND_V_MSG(src_tex->format != dst_tex->format, ERR_INVALID_PARAMETER, "Source and Destination textures must be the same format.");
	ERR_FAIL_COND_V_MSG(src_tex->width != dst_tex->width && src_tex->height != dst_tex->height && src_tex->depth != dst_tex->depth, ERR_INVALID_PARAMETER, "Source and Destination textures must have the same dimensions.");

	ERR_FAIL_COND_V_MSG(src_tex->read_aspect_flags != dst_tex->read_aspect_flags, ERR_INVALID_PARAMETER,
			"Source and destination texture must be of the same type (color or depth).");

	// Indicate the texture will get modified for the shared texture fallback.
	_texture_update_shared_fallback(p_to_texture, dst_tex, true);

	// The textures must be mutable to be used in the resolve operation.
	bool src_made_mutable = _texture_make_mutable(src_tex, p_from_texture);
	bool dst_made_mutable = _texture_make_mutable(dst_tex, p_to_texture);
	if (src_made_mutable || dst_made_mutable) {
		draw_graph.add_synchronization();
	}

	draw_graph.add_texture_resolve(src_tex->driver_id, src_tex->draw_tracker, dst_tex->driver_id, dst_tex->draw_tracker, src_tex->base_layer, src_tex->base_mipmap, dst_tex->base_layer, dst_tex->base_mipmap);

	return OK;
}
///
/// ******FRAME **********
///
/// frame operations
void RenderingDevice::_flush_and_stall_for_all_frames() {
  _stall_for_previous_frames();
  _end_frame();
  _execute_frame(false);
  _begin_frame();
}

void RenderingDevice::_stall_for_previous_frames() {
  for (uint32_t i = 0; i < frames.size(); i++) {
    if (frames[i].draw_fence_signaled) {
      driver->fence_wait(frames[i].draw_fence);
      frames[i].draw_fence_signaled = false;
    }
  }
}

void RenderingDevice::_begin_frame() {
  // Before beginning this frame, wait on the fence if it was signaled to make sure its work is finished.
  if (frames[frame].draw_fence_signaled) {
    driver->fence_wait(frames[frame].draw_fence);
    frames[frame].draw_fence_signaled = false;
  }

  // update_perf_report();

  // Begin recording on the frame's command buffers.
  driver->begin_segment(frame, frames_drawn++);
  driver->command_buffer_begin(frames[frame].setup_command_buffer);
  driver->command_buffer_begin(frames[frame].draw_command_buffer);

  // Reset the graph.
  draw_graph.begin();

  // Erase pending resources.
  _free_pending_resources(frame);

  // Advance staging buffer if used.
  if (staging_buffer_used) {
    staging_buffer_current = (staging_buffer_current + 1) % staging_buffer_blocks.size();
    staging_buffer_used = false;
  }

  if (frames[frame].timestamp_count) {
    driver->timestamp_query_pool_get_results(frames[frame].timestamp_pool, frames[frame].timestamp_count, frames[frame].timestamp_result_values.ptr());
    driver->command_timestamp_query_pool_reset(frames[frame].setup_command_buffer, frames[frame].timestamp_pool, frames[frame].timestamp_count);
    SWAP(frames[frame].timestamp_names, frames[frame].timestamp_result_names);
    SWAP(frames[frame].timestamp_cpu_values, frames[frame].timestamp_cpu_result_values);
  }

  frames[frame].timestamp_result_count = frames[frame].timestamp_count;
  frames[frame].timestamp_count = 0;
  frames[frame].index = Engine::GetSingleton()->get_frames_drawn();
}

void RenderingDevice::_end_frame() {
  if (draw_list) {
    ERR_PRINT(
        "Found open draw list at the end of the frame, this should never happen (further drawing "
        "will likely not work).");
  }

  if (compute_list) {
    ERR_PRINT(
        "Found open compute list at the end of the frame, this should never happen (further "
        "compute will likely not work).");
  }

  driver->command_buffer_end(frames[frame].setup_command_buffer);

  // The command buffer must be copied into a stack variable as the driver workarounds can change the command buffer in use.
  RDD::CommandBufferID command_buffer = frames[frame].draw_command_buffer;
  draw_graph.end(RENDER_GRAPH_REORDER, RENDER_GRAPH_FULL_BARRIERS, command_buffer, frames[frame].command_buffer_pool);
  driver->command_buffer_end(command_buffer);
  driver->end_segment();
}

void lain::RenderingDevice::_execute_frame(bool p_present) {
  // Check whether this frame should present the swap chains and in which queue.
  const bool frame_can_present = p_present && !frames[frame].swap_chains_to_present.is_empty();
  const bool separate_present_queue = main_queue != present_queue;
  thread_local LocalVector<RDD::SwapChainID> swap_chains;
  swap_chains.clear();

  // Execute the setup command buffer.
  // setup command buffer 给 setup_semaphore 信号
  driver->command_queue_execute_and_present(main_queue, {}, frames[frame].setup_command_buffer, frames[frame].setup_semaphore, {}, {});

  // Execute command buffers and use semaphores to wait on the execution of the previous one. Normally there's only one command buffer,
  // but driver workarounds can force situations where there'll be more.
  uint32_t command_buffer_count = 1;
  RDG::CommandBufferPool& buffer_pool = frames[frame].command_buffer_pool;
  if (buffer_pool.buffers_used > 0) {
    command_buffer_count += buffer_pool.buffers_used;
    buffer_pool.buffers_used = 0;
  }

  RDD::SemaphoreID wait_semaphore = frames[frame].setup_semaphore;
  for (uint32_t i = 0; i < command_buffer_count; i++) {
    RDD::CommandBufferID command_buffer;
    RDD::SemaphoreID signal_semaphore;
    RDD::FenceID signal_fence;
    if (i > 0) {
      command_buffer = buffer_pool.buffers[i - 1];
      signal_semaphore = buffer_pool.semaphores[i - 1];
    } else {
      command_buffer = frames[frame].draw_command_buffer;
      signal_semaphore = frames[frame].draw_semaphore;
    }

    bool signal_semaphore_valid;
    if (i == (command_buffer_count - 1)) {
      // This is the last command buffer, it should signal the fence.
      signal_fence = frames[frame].draw_fence;
      signal_semaphore_valid = false;

      if (frame_can_present && separate_present_queue) {
        // The semaphore is required if the frame can be presented and a separate present queue is used.
        signal_semaphore_valid = true;
      } else if (frame_can_present) {
        // Just present the swap chains as part of the last command execution.
        swap_chains = frames[frame].swap_chains_to_present;
      }
    } else {
      // Semaphores always need to be signaled if it's not the last command buffer.
      signal_semaphore_valid = true;
    }

    driver->command_queue_execute_and_present(main_queue, wait_semaphore, command_buffer, signal_semaphore_valid ? signal_semaphore : VectorView<RDD::SemaphoreID>(),
                                              signal_fence, swap_chains);

    // Make the next command buffer wait on the semaphore signaled by this one.
    wait_semaphore = signal_semaphore;
  }

  // Indicate the fence has been signaled so the next time the frame's contents need to be used, the CPU needs to wait on the work to be completed.
  frames[frame].draw_fence_signaled = true;

  if (frame_can_present) {
    if (separate_present_queue) {
      // Issue the presentation separately if the presentation queue is different from the main queue.
      driver->command_queue_execute_and_present(present_queue, wait_semaphore, {}, {}, {}, frames[frame].swap_chains_to_present);
    }

    frames[frame].swap_chains_to_present.clear();
  }
}

/// --- swap chain ---
/// ****** screen *******
/// *********************
Error RenderingDevice::screen_create(WindowSystem::WindowID p_screen) {
  _THREAD_SAFE_METHOD_

  RenderingContextDriver::SurfaceID surface = context->surface_get_from_window(p_screen);
  ERR_FAIL_COND_V_MSG(surface == 0, ERR_CANT_CREATE, "A surface was not created for the screen.");

  HashMap<WindowSystem::WindowID, RDD::SwapChainID>::ConstIterator it = screen_swap_chains.find(p_screen);
  ERR_FAIL_COND_V_MSG(it != screen_swap_chains.end(), ERR_CANT_CREATE, "A swap chain was already created for the screen.");

  RDD::SwapChainID swap_chain = driver->swap_chain_create(surface);
  ERR_FAIL_COND_V_MSG(swap_chain.id == 0, ERR_CANT_CREATE, "Unable to create swap chain.");
  // 实际是这个create的
  Error err = driver->swap_chain_resize(main_queue, swap_chain, _get_swap_chain_desired_count());
  ERR_FAIL_COND_V_MSG(err != OK, ERR_CANT_CREATE, "Unable to resize the new swap chain.");

  screen_swap_chains[p_screen] = swap_chain;

  return OK;
}

Error RenderingDevice::screen_prepare_for_drawing(WindowSystem::WindowID p_screen) {
  _THREAD_SAFE_METHOD_

  HashMap<WindowSystem::WindowID, RDD::SwapChainID>::ConstIterator it = screen_swap_chains.find(p_screen);
  ERR_FAIL_COND_V_MSG(it == screen_swap_chains.end(), ERR_CANT_CREATE, "A swap chain was not created for the screen.");

  // Erase the framebuffer corresponding to this screen from the map in case any of the operations fail.
  screen_framebuffers.erase(p_screen);

  // If this frame has already queued this swap chain for presentation, we present it and remove it from the pending list.
  uint32_t to_present_index = 0;
  while (to_present_index < frames[frame].swap_chains_to_present.size()) {
    if (frames[frame].swap_chains_to_present[to_present_index] == it->value) {  // 好到这个交换链
      driver->command_queue_execute_and_present(present_queue, {}, {}, {}, {}, it->value);
      frames[frame].swap_chains_to_present.remove_at(to_present_index);
    } else {
      to_present_index++;
    }
  }

  bool resize_required = false;
  RDD::FramebufferID framebuffer = driver->swap_chain_acquire_framebuffer(main_queue, it->value, resize_required);
  if (resize_required) {
    // Flush everything so nothing can be using the swap chain before resizing it.
    _flush_and_stall_for_all_frames();

    Error err = driver->swap_chain_resize(main_queue, it->value, _get_swap_chain_desired_count());
    if (err != OK) {
      // Resize is allowed to fail silently because the window can be minimized.
      return err;
    }

    framebuffer = driver->swap_chain_acquire_framebuffer(main_queue, it->value, resize_required);
  }

  ERR_FAIL_COND_V_MSG(framebuffer.id == 0, FAILED, "Unable to acquire framebuffer.");

  // Store the framebuffer that will be used next to draw to this screen.
  screen_framebuffers[p_screen] = framebuffer;
  frames[frame].swap_chains_to_present.push_back(it->value);

  return OK;
}

int RenderingDevice::screen_get_width(WindowSystem::WindowID p_screen) const {
  _THREAD_SAFE_METHOD_
  RenderingContextDriver::SurfaceID surface = context->surface_get_from_window(p_screen);
  ERR_FAIL_COND_V_MSG(surface == 0, 0, "A surface was not created for the screen.");
  return context->surface_get_width(surface);
}

int RenderingDevice::screen_get_height(WindowSystem::WindowID p_screen) const {
  _THREAD_SAFE_METHOD_
  RenderingContextDriver::SurfaceID surface = context->surface_get_from_window(p_screen);
  ERR_FAIL_COND_V_MSG(surface == 0, 0, "A surface was not created for the screen.");
  return context->surface_get_height(surface);
}

RenderingDevice::FramebufferFormatID RenderingDevice::screen_get_framebuffer_format(WindowSystem::WindowID p_screen) const {
  _THREAD_SAFE_METHOD_

  HashMap<WindowSystem::WindowID, RDD::SwapChainID>::ConstIterator it = screen_swap_chains.find(p_screen);
  ERR_FAIL_COND_V_MSG(it == screen_swap_chains.end(), FAILED, "Screen was never prepared.");

  DataFormat format = driver->swap_chain_get_format(it->value);
  ERR_FAIL_COND_V(format == DATA_FORMAT_MAX, INVALID_ID);

  AttachmentFormat attachment;
  attachment.format = format;
  attachment.samples = TEXTURE_SAMPLES_1;
  attachment.usage_flags = TEXTURE_USAGE_COLOR_ATTACHMENT_BIT;
  Vector<AttachmentFormat> screen_attachment;
  screen_attachment.push_back(attachment);
  return const_cast<RenderingDevice*>(this)->framebuffer_format_create(screen_attachment);
}

Error RenderingDevice::screen_free(WindowSystem::WindowID p_screen) {
  _THREAD_SAFE_METHOD_

  HashMap<WindowSystem::WindowID, RDD::SwapChainID>::ConstIterator it = screen_swap_chains.find(p_screen);
  ERR_FAIL_COND_V_MSG(it == screen_swap_chains.end(), FAILED, "Screen was never created.");

  // Flush everything so nothing can be using the swap chain before erasing it.
  _flush_and_stall_for_all_frames();

  const WindowSystem::WindowID screen = it->key;
  const RDD::SwapChainID swap_chain = it->value;
  driver->swap_chain_free(swap_chain);
  screen_framebuffers.erase(screen);
  screen_swap_chains.erase(screen);

  return OK;
}
/// ********************************
/// ********** STAGING BUFFER ******
/// *********************************

Error RenderingDevice::_staging_buffer_allocate(uint32_t p_amount, uint32_t p_required_align, uint32_t& r_alloc_offset, uint32_t& r_alloc_size,
                                                StagingRequiredAction& r_required_action, bool p_can_segment) {
  // Determine a block to use.

  r_alloc_size = p_amount;
  r_required_action = STAGING_REQUIRED_ACTION_NONE;

  while (true) {
    r_alloc_offset = 0;

    // See if we can use current block.
    if (staging_buffer_blocks[staging_buffer_current].frame_used == frames_drawn) {
      // We used this block this frame, let's see if there is still room.

      uint32_t write_from = staging_buffer_blocks[staging_buffer_current].fill_amount;

      {
        uint32_t align_remainder = write_from % p_required_align;
        if (align_remainder != 0) {
          write_from += p_required_align - align_remainder;
        }
      }

      int32_t available_bytes = int32_t(staging_buffer_block_size) - int32_t(write_from);

      if ((int32_t)p_amount < available_bytes) {
        // All is good, we should be ok, all will fit.
        r_alloc_offset = write_from;
      } else if (p_can_segment && available_bytes >= (int32_t)p_required_align) {
        // Ok all won't fit but at least we can fit a chunkie.
        // All is good, update what needs to be written to.
        r_alloc_offset = write_from;
        r_alloc_size = available_bytes - (available_bytes % p_required_align);

      } else {
        // Can't fit it into this buffer.
        // Will need to try next buffer.

        staging_buffer_current = (staging_buffer_current + 1) % staging_buffer_blocks.size();

        // Before doing anything, though, let's check that we didn't manage to fill all blocks.
        // Possible in a single frame.
        if (staging_buffer_blocks[staging_buffer_current].frame_used == frames_drawn) {
          // Guess we did.. ok, let's see if we can insert a new block.
          if ((uint64_t)staging_buffer_blocks.size() * staging_buffer_block_size < staging_buffer_max_size) {
            // We can, so we are safe.
            Error err = _insert_staging_block();
            if (err) {
              return err;
            }
            // Claim for this frame.
            staging_buffer_blocks.write[staging_buffer_current].frame_used = frames_drawn;
          } else {
            // Ok, worst case scenario, all the staging buffers belong to this frame
            // and this frame is not even done.
            // If this is the main thread, it means the user is likely loading a lot of resources at once,.
            // Otherwise, the thread should just be blocked until the next frame (currently unimplemented).
            r_required_action = STAGING_REQUIRED_ACTION_FLUSH_AND_STALL_ALL;
          }

        } else {
          // Not from current frame, so continue and try again.
          continue;
        }
      }

    } else if (staging_buffer_blocks[staging_buffer_current].frame_used <= frames_drawn - frames.size()) {
      // This is an old block, which was already processed, let's reuse.
      staging_buffer_blocks.write[staging_buffer_current].frame_used = frames_drawn;
      staging_buffer_blocks.write[staging_buffer_current].fill_amount = 0;
    } else {
      // This block may still be in use, let's not touch it unless we have to, so.. can we create a new one?
      if ((uint64_t)staging_buffer_blocks.size() * staging_buffer_block_size < staging_buffer_max_size) {
        // We are still allowed to create a new block, so let's do that and insert it for current pos.
        Error err = _insert_staging_block();
        if (err) {
          return err;
        }
        // Claim for this frame.
        staging_buffer_blocks.write[staging_buffer_current].frame_used = frames_drawn;
      } else {
        // Oops, we are out of room and we can't create more.
        // Let's flush older frames.
        // The logic here is that if a game is loading a lot of data from the main thread, it will need to be stalled anyway.
        // If loading from a separate thread, we can block that thread until next frame when more room is made (not currently implemented, though).
        r_required_action = STAGING_REQUIRED_ACTION_STALL_PREVIOUS;
      }
    }

    // All was good, break.
    break;
  }

  staging_buffer_used = true;

  return OK;
}

void RenderingDevice::_staging_buffer_execute_required_action(StagingRequiredAction p_required_action) {
  switch (p_required_action) {
    case STAGING_REQUIRED_ACTION_NONE: {
      // Do nothing.
    } break;
    case STAGING_REQUIRED_ACTION_FLUSH_AND_STALL_ALL: {  // 该帧的满了
      _flush_and_stall_for_all_frames();

      // Clear the whole staging buffer.
      for (int i = 0; i < staging_buffer_blocks.size(); i++) {
        staging_buffer_blocks.write[i].frame_used = 0;
        staging_buffer_blocks.write[i].fill_amount = 0;
      }

      // Claim for current frame.
      staging_buffer_blocks.write[staging_buffer_current].frame_used = frames_drawn;
    } break;
    case STAGING_REQUIRED_ACTION_STALL_PREVIOUS: {
      _stall_for_previous_frames();

      for (int i = 0; i < staging_buffer_blocks.size(); i++) {
        // Clear all blocks but the ones from this frame.
        int block_idx = (i + staging_buffer_current) % staging_buffer_blocks.size();
        if (staging_buffer_blocks[block_idx].frame_used == frames_drawn) {
          break;  // Ok, we reached something from this frame, abort.
        }

        staging_buffer_blocks.write[block_idx].frame_used = 0;
        staging_buffer_blocks.write[block_idx].fill_amount = 0;
      }

      // Claim for current frame.
      staging_buffer_blocks.write[staging_buffer_current].frame_used = frames_drawn;
    } break;
    default: {
      DEV_ASSERT(false && "Unknown required action.");
    } break;
  }
}

/// *******************************
/// *********** buffer ************
///*******************************
RenderingDevice::Buffer* RenderingDevice::_get_buffer_from_owner(RID p_buffer) {
  Buffer* buffer = nullptr;
  if (vertex_buffer_owner.owns(p_buffer)) {
    buffer = vertex_buffer_owner.get_or_null(p_buffer);
  } else if (index_buffer_owner.owns(p_buffer)) {
    buffer = index_buffer_owner.get_or_null(p_buffer);
  } else if (uniform_buffer_owner.owns(p_buffer)) {
    buffer = uniform_buffer_owner.get_or_null(p_buffer);
  } else if (texture_buffer_owner.owns(p_buffer)) {
    DEV_ASSERT(false && "FIXME: Broken.");
    //buffer = texture_buffer_owner.get_or_null(p_buffer)->buffer;
  } else if (storage_buffer_owner.owns(p_buffer)) {
    buffer = storage_buffer_owner.get_or_null(p_buffer);
  }
  return buffer;
}

Error RenderingDevice::_buffer_update(Buffer* p_buffer, RID p_buffer_id, size_t p_offset, const uint8_t* p_data, size_t p_data_size, bool p_use_draw_queue,
                                      uint32_t p_required_align) {
  // Submitting may get chunked for various reasons, so convert this to a task.
  size_t to_submit = p_data_size;
  size_t submit_from = 0;

  thread_local LocalVector<RDG::RecordedBufferCopy> command_buffer_copies_vector;
  command_buffer_copies_vector.clear();

  while (to_submit > 0) {
    uint32_t block_write_offset;
    uint32_t block_write_amount;
    StagingRequiredAction required_action;

    Error err = _staging_buffer_allocate(MIN(to_submit, staging_buffer_block_size), p_required_align, block_write_offset, block_write_amount, required_action);
    if (err) {
      return err;
    }

    if (p_use_draw_queue && !command_buffer_copies_vector.is_empty() && required_action == STAGING_REQUIRED_ACTION_FLUSH_AND_STALL_ALL) {
      if (_buffer_make_mutable(p_buffer, p_buffer_id)) {
        // The buffer must be mutable to be used as a copy destination.
        draw_graph.add_synchronization();
      }

      // If we're using the draw queue and the staging buffer requires flushing everything, we submit the command early and clear the current vector.
      draw_graph.add_buffer_update(p_buffer->driver_id, p_buffer->draw_tracker, command_buffer_copies_vector);
      command_buffer_copies_vector.clear();
    }

    _staging_buffer_execute_required_action(required_action);

    // Map staging buffer (It's CPU and coherent).
    uint8_t* data_ptr = driver->buffer_map(staging_buffer_blocks[staging_buffer_current].driver_id);
    ERR_FAIL_NULL_V(data_ptr, ERR_CANT_CREATE);

    // Copy to staging buffer.
    memcpy(data_ptr + block_write_offset, p_data + submit_from, block_write_amount);

    // Unmap.
    driver->buffer_unmap(staging_buffer_blocks[staging_buffer_current].driver_id);

    // Insert a command to copy this.
    RDD::BufferCopyRegion region;
    region.src_offset = block_write_offset;
    region.dst_offset = submit_from + p_offset;
    region.size = block_write_amount;
    if (p_use_draw_queue) {
      RDG::RecordedBufferCopy buffer_copy;
      buffer_copy.source = staging_buffer_blocks[staging_buffer_current].driver_id;
      buffer_copy.region = region;
      command_buffer_copies_vector.push_back(buffer_copy);
    } else {
      driver->command_copy_buffer(frames[frame].setup_command_buffer, staging_buffer_blocks[staging_buffer_current].driver_id, p_buffer->driver_id, region);
    }

    staging_buffer_blocks.write[staging_buffer_current].fill_amount = block_write_offset + block_write_amount;

    to_submit -= block_write_amount;
    submit_from += block_write_amount;
  }

  if (p_use_draw_queue && !command_buffer_copies_vector.is_empty()) {
    if (_buffer_make_mutable(p_buffer, p_buffer_id)) {
      // The buffer must be mutable to be used as a copy destination.
      draw_graph.add_synchronization();
    }

    draw_graph.add_buffer_update(p_buffer->driver_id, p_buffer->draw_tracker, command_buffer_copies_vector);
  }

  return OK;
}
// data 应该是 uint8*
Error RenderingDevice::buffer_update(RID p_buffer, uint32_t p_offset, uint32_t p_size, const void* p_data) {
  _THREAD_SAFE_METHOD_

  ERR_FAIL_COND_V_MSG(draw_list, ERR_INVALID_PARAMETER, "Updating buffers is forbidden during creation of a draw list");
  ERR_FAIL_COND_V_MSG(compute_list, ERR_INVALID_PARAMETER, "Updating buffers is forbidden during creation of a compute list");

  Buffer* buffer = _get_buffer_from_owner(p_buffer);
  if (!buffer) {
    ERR_FAIL_V_MSG(ERR_INVALID_PARAMETER, "Buffer argument is not a valid buffer of any type.");
  }

  ERR_FAIL_COND_V_MSG(p_offset + p_size > buffer->size, ERR_INVALID_PARAMETER,
                      "Attempted to write buffer (" + itos((p_offset + p_size) - buffer->size) + " bytes) past the end.");

  return _buffer_update(buffer, p_buffer, p_offset, (uint8_t*)p_data, p_size, true);
}

uint32_t RenderingDevice::_get_swap_chain_desired_count() const {
  return MAX(3U, uint32_t(GLOBAL_GET("rendering/rendering_device/vsync/swapchain_image_count")));  // 这个也要调整吗
      // 如果这个不调可以做成const expr，许多东西（？）可以被优化
}
/// @brief RenderingDevice的initialize，需要: 选择设备，初始化该设备的driver，新建queue（main，present）
// 获得surface，
/// @param p_context
/// @param p_main_window
/// @return
Error RenderingDevice::initialize(RenderingContextDriver* p_context, WindowSystem::WindowID p_main_window) {
  using RCD = RenderingContextDriver;
  Error err = OK;

  RCD::SurfaceID main_surface = 0;
  is_main_instance = (singleton == this) && (p_main_window != WindowSystem::INVALID_WINDOW_ID);
  if (p_main_window != WindowSystem::INVALID_WINDOW_ID) {
    // Retrieve the surface from the main window if it was specified.
    main_surface = p_context->surface_get_from_window(p_main_window);
    ERR_FAIL_COND_V(main_surface == 0, FAILED);
  }

  main_surface = p_context->surface_get_from_window(p_main_window);
  ERR_FAIL_COND_V(main_surface == 0, FAILED);

  context = p_context;
  driver = context->driver_create();
  print_verbose("Devices:");
  int32_t device_index = Engine::GetSingleton()->get_gpu_index();
  const uint32_t device_count = context->device_get_count();
  const bool detect_device = (device_index < 0) || (device_index >= int32_t(device_count));
  uint32_t device_type_score = 0;
  for (uint32_t i = 0; i < device_count; i++) {  // 打印信息，选择设备
    RenderingContextDriver::Device device_option = context->device_get(i);
    String name = device_option.name;
    String vendor = _get_device_vendor_name(device_option);
    String type = _get_device_type_name(device_option);
    bool present_supported = main_surface != 0 ? context->device_supports_present(i, main_surface) : false;
    print_verbose("  #" + itos(i) + ": " + vendor + " " + name + " - " + (present_supported ? "Supported" : "Unsupported") + ", " + type);
    if (detect_device && (present_supported || main_surface == 0)) {
      // If a window was specified, present must be supported by the device to be available as an option.
      // Assign a score for each type of device and prefer the device with the higher score.
      uint32_t option_score = _get_device_type_score(device_option);
      if (option_score > device_type_score) {
        device_index = i;
        device_type_score = option_score;
      }
    }
  }
  ERR_FAIL_COND_V_MSG((device_index < 0) || (device_index >= int32_t(device_count)), ERR_CANT_CREATE, "None of the devices supports both graphics and present queues.");
  uint32_t frame_count = 1;
  if (main_surface != 0) {
    frame_count = MAX(3U, uint32_t(GLOBAL_GET("rendering/rendering_device/vsync/frame_queue_size")));
  }

  frame = 0;
  frames.resize(frame_count);
  max_timestamp_query_elements = GLOBAL_GET("debug/settings/profiler/max_timestamp_query_elements");

  device = context->device_get(device_index);  // 选择设备

  err = driver->initialize(device_index, frame_count);  // driver初始化
  ERR_FAIL_COND_V_MSG(err != OK, FAILED, "Failed to initialize driver for device.");

  // Pick the main queue family. It is worth noting we explicitly do not request the transfer bit, as apparently the specification defines
  // that the existence of either the graphics or compute bit implies that the queue can also do transfer operations, but it is optional
  // to indicate whether it supports them or not with the dedicated transfer bit if either is set.
  BitField<RDD::CommandQueueFamilyBits> main_queue_bits;
  main_queue_bits.set_flag(RDD::COMMAND_QUEUE_FAMILY_GRAPHICS_BIT);
  main_queue_bits.set_flag(RDD::COMMAND_QUEUE_FAMILY_COMPUTE_BIT);
  main_queue_family = driver->command_queue_family_get(main_queue_bits, main_surface);
  if (!main_queue_family) {
    // 不用main_surface
    main_queue_family = driver->command_queue_family_get(main_queue_bits);
    // present 不用支持
    present_queue_family = driver->command_queue_family_get(BitField<RDD::CommandQueueFamilyBits>(), main_surface);
    ERR_FAIL_COND_V(!present_queue_family, FAILED);
  }
  ERR_FAIL_COND_V(!main_queue_family, FAILED);
  // Create the main queue.
  main_queue = driver->command_queue_create(main_queue_family, true);
  ERR_FAIL_COND_V(!main_queue, FAILED);
  if (present_queue_family) {
    // Create the presentation queue.
    present_queue = driver->command_queue_create(present_queue_family);
    ERR_FAIL_COND_V(!present_queue, FAILED);
  } else {
    present_queue = main_queue;
  }

  // Create data for all the frames.
  /// @brief 为每一帧创建数据
  /// command_pool, setup_command_buffer, draw_command_buffer, setup_semaphore, draw_semaphore, draw_fence
  /// query pool
  for (uint32_t i = 0; i < frames.size(); i++) {
    frames[i].index = 0;

    // Create command pool, command buffers, semaphores and fences.
    frames[i].command_pool = driver->command_pool_create(main_queue_family, RDD::COMMAND_BUFFER_TYPE_PRIMARY);
    ERR_FAIL_COND_V(!frames[i].command_pool, FAILED);
    frames[i].setup_command_buffer = driver->command_buffer_create(frames[i].command_pool);
    ERR_FAIL_COND_V(!frames[i].setup_command_buffer, FAILED);
    frames[i].draw_command_buffer = driver->command_buffer_create(frames[i].command_pool);
    ERR_FAIL_COND_V(!frames[i].draw_command_buffer, FAILED);
    frames[i].setup_semaphore = driver->semaphore_create();
    ERR_FAIL_COND_V(!frames[i].setup_semaphore, FAILED);
    frames[i].draw_semaphore = driver->semaphore_create();
    ERR_FAIL_COND_V(!frames[i].draw_semaphore, FAILED);
    frames[i].draw_fence = driver->fence_create();
    ERR_FAIL_COND_V(!frames[i].draw_fence, FAILED);
    frames[i].draw_fence_signaled = false;

    // Create query pool.
    frames[i].timestamp_pool = driver->timestamp_query_pool_create(max_timestamp_query_elements);
    frames[i].timestamp_names.resize(max_timestamp_query_elements);
    frames[i].timestamp_cpu_values.resize(max_timestamp_query_elements);
    frames[i].timestamp_count = 0;
    frames[i].timestamp_result_names.resize(max_timestamp_query_elements);
    frames[i].timestamp_cpu_result_values.resize(max_timestamp_query_elements);
    frames[i].timestamp_result_values.resize(max_timestamp_query_elements);
    frames[i].timestamp_result_count = 0;

    // Assign the main queue family and command pool to the command buffer pool.
    frames[i].command_buffer_pool.pool = frames[i].command_pool;
  }

  // Start from frame count, so everything else is immediately old.
  frames_drawn = frames.size();

  // Initialize recording on the first frame.
  driver->begin_segment(frame, frames_drawn++);
  driver->command_buffer_begin(frames[0].setup_command_buffer);
  driver->command_buffer_begin(frames[0].draw_command_buffer);

  secondary_command_buffer_per_frame = GLOBAL_GET("rendering/rendering_device/command/secondary_command_buffers_per_frame");
  secondary_command_buffer_per_frame = MIN(4u, secondary_command_buffer_per_frame);

  // Create draw graph and start it initialized as well.
  draw_graph.initialize(driver, device, frames.size(), main_queue_family, secondary_command_buffer_per_frame);
  draw_graph.begin();

  for (uint32_t i = 0; i < frames.size(); i++) {
    // Reset all queries in a query pool before doing any operations with them..
    driver->command_timestamp_query_pool_reset(frames[0].setup_command_buffer, frames[i].timestamp_pool, max_timestamp_query_elements);
  }

  // Convert block size from KB.
  staging_buffer_block_size = GLOBAL_GET("rendering/rendering_device/staging_buffer/block_size_kb");
  staging_buffer_block_size = MAX(4u, staging_buffer_block_size);
  staging_buffer_block_size *= 1024;

  // Convert staging buffer size from MB.
  staging_buffer_max_size = GLOBAL_GET("rendering/rendering_device/staging_buffer/max_size_mb");
  staging_buffer_max_size = MAX(1u, staging_buffer_max_size);
  staging_buffer_max_size *= 1024 * 1024;

  if (staging_buffer_max_size < staging_buffer_block_size * 4) {
    // Validate enough blocks.
    staging_buffer_max_size = staging_buffer_block_size * 4;
  }

  texture_upload_region_size_px = GLOBAL_GET("rendering/rendering_device/staging_buffer/texture_upload_region_size_px");
  texture_upload_region_size_px = nearest_power_of_2_templated(texture_upload_region_size_px);

  // Ensure current staging block is valid and at least one per frame exists.
  // already initialized(when construct)
  staging_buffer_current = 0;
  staging_buffer_used = false;
  // 每个frame都有一个staging block
  for (uint32_t i = 0; i < frames.size(); i++) {
    // Staging was never used, create a block.
    err = _insert_staging_block();
    ERR_FAIL_COND_V(err, FAILED);
  }
  draw_list = nullptr;
  compute_list = nullptr;

  bool project_pipeline_cache_enable = GLOBAL_GET("rendering/rendering_device/pipeline_cache/enable");
  if (is_main_instance && project_pipeline_cache_enable) {
    // Only the instance that is not a local device and is also the singleton is allowed to manage a pipeline cache.
    pipeline_cache_file_path =
        vformat("user://vulkan/pipelines.%s.%s", OS::GetSingleton()->GetCurrentRenderingMethod(), device.name.validate_filename().replace(" ", "_").to_lower());
    if (Engine::GetSingleton()->is_editor_hint()) {
      pipeline_cache_file_path += ".editor";
    }
    pipeline_cache_file_path += ".cache";

    Vector<uint8_t> cache_data = _load_pipeline_cache();
    pipeline_cache_enabled = driver->pipeline_cache_create(cache_data);
    if (pipeline_cache_enabled) {
      pipeline_cache_size = driver->pipeline_cache_query_size();
      print_verbose(vformat("Startup PSO cache (%.1f MiB)", pipeline_cache_size / (1024.0f * 1024.0f)));
    }
  }

  return OK;
}

Vector<uint8_t> RenderingDevice::_load_pipeline_cache() {
  DirAccess::make_dir_recursive_absolute(pipeline_cache_file_path.get_base_dir());

  if (FileAccess::exists(pipeline_cache_file_path)) {
    Error file_error;
    Vector<uint8_t> file_data = FileAccess::get_file_as_bytes(pipeline_cache_file_path, &file_error);
    return file_data;
  } else {
    return Vector<uint8_t>();
  }
}

void RenderingDevice::_update_pipeline_cache(bool p_closing) {
  {
    bool still_saving = pipeline_cache_save_task != WorkerThreadPool::INVALID_TASK_ID && !WorkerThreadPool::get_singleton()->is_task_completed(pipeline_cache_save_task);
    if (still_saving) {
      if (p_closing) {
        WorkerThreadPool::get_singleton()->wait_for_task_completion(pipeline_cache_save_task);
        pipeline_cache_save_task = WorkerThreadPool::INVALID_TASK_ID;
      } else {
        // We can't save until the currently running save is done. We'll retry next time; worst case, we'll save when exiting.
        // @todo 做异步
        return;
      }
    }
  }

  {  // 计算差别，若差别大于save_chunk_size_mb则保存
    size_t new_pipelines_cache_size = driver->pipeline_cache_query_size();
    ERR_FAIL_COND(!new_pipelines_cache_size);
    size_t difference = new_pipelines_cache_size - pipeline_cache_size;

    bool must_save = false;

    if (p_closing) {
      must_save = difference > 0;
    } else {
      float save_interval = GLOBAL_GET("rendering/rendering_device/pipeline_cache/save_chunk_size_mb");
      must_save = difference > 0 && difference / (1024.0f * 1024.0f) >= save_interval;
    }

    if (must_save) {
      pipeline_cache_size = new_pipelines_cache_size;
    } else {
      return;
    }
  }

  if (p_closing) {
    _save_pipeline_cache(this);
  } else {
    pipeline_cache_save_task = WorkerThreadPool::get_singleton()->add_native_task(&_save_pipeline_cache, this, false, "PipelineCacheSave");
  }
}

void RenderingDevice::_save_pipeline_cache(void* p_data) {
  RenderingDevice* self = static_cast<RenderingDevice*>(p_data);

  self->_thread_safe_.lock();
  Vector<uint8_t> cache_blob = self->driver->pipeline_cache_serialize();
  self->_thread_safe_.unlock();

  if (cache_blob.size() == 0) {
    return;
  }
  print_verbose(vformat("Updated PSO cache (%.1f MiB)", cache_blob.size() / (1024.0f * 1024.0f)));

  Ref<FileAccess> f = FileAccess::open(self->pipeline_cache_file_path, FileAccess::WRITE, nullptr);
  if (f.is_valid()) {
    f->store_buffer(cache_blob);
  }
}

/// framebuffer
RenderingDevice::FramebufferFormatID RenderingDevice::framebuffer_format_create(const Vector<AttachmentFormat>& p_format, uint32_t p_view_count) {
  FramebufferPass pass;
  for (int i = 0; i < p_format.size(); i++) {
    if (p_format[i].usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      pass.depth_attachment = i;
    } else {
      pass.color_attachments.push_back(i);
    }
  }

  Vector<FramebufferPass> passes;
  passes.push_back(pass);
  return framebuffer_format_create_multipass(p_format, passes, p_view_count);
}

RenderingDevice::FramebufferFormatID RenderingDevice::framebuffer_format_create_multipass(const Vector<AttachmentFormat>& p_attachments,
                                                                                          const Vector<FramebufferPass>& p_passes, uint32_t p_view_count) {
  _THREAD_SAFE_METHOD_

  FramebufferFormatKey key;
  key.attachment_formats = p_attachments;
  key.passes = p_passes;
  key.view_count = p_view_count;

  const RBMap<FramebufferFormatKey, FramebufferFormatID>::Element* E = framebuffer_format_cache.find(key);
  if (E) {
    // Exists, return.
    return E->get();  // return ID.
  }

  Vector<TextureSamples> samples;
  // 这个renderpass 另有他用，可能（shader 需要）因此无所谓version
  RDD::RenderPassID render_pass =
      _render_pass_create(p_attachments, p_passes, {}, {}, INITIAL_ACTION_CLEAR, FINAL_ACTION_STORE, p_view_count, &samples);  // Actions don't matter for this use case.

  if (!render_pass) {  // Was likely invalid.
    return INVALID_ID;
  }
  FramebufferFormatID id = FramebufferFormatID(framebuffer_format_cache.size()) | (FramebufferFormatID(ID_TYPE_FRAMEBUFFER_FORMAT) << FramebufferFormatID(ID_BASE_SHIFT));

  E = framebuffer_format_cache.insert(key, id);
  FramebufferFormat fb_format;
  fb_format.E = E;
  fb_format.render_pass = render_pass;
  fb_format.pass_samples = samples;
  fb_format.view_count = p_view_count;
  framebuffer_formats[id] = fb_format;
  return id;
}

RenderingDevice::FramebufferFormatID RenderingDevice::framebuffer_format_create_empty(TextureSamples p_samples) {
  FramebufferFormatKey key;
  key.passes.push_back(FramebufferPass());

  const RBMap<FramebufferFormatKey, FramebufferFormatID>::Element* E = framebuffer_format_cache.find(key);
  if (E) {
    // Exists, return.
    return E->get();
  }

  LocalVector<RDD::Subpass> subpass;
  subpass.resize(1);

  RDD::RenderPassID render_pass = driver->render_pass_create({}, subpass, {}, 1);
  ERR_FAIL_COND_V(!render_pass, FramebufferFormatID());

  FramebufferFormatID id = FramebufferFormatID(framebuffer_format_cache.size()) | (FramebufferFormatID(ID_TYPE_FRAMEBUFFER_FORMAT) << FramebufferFormatID(ID_BASE_SHIFT));

  E = framebuffer_format_cache.insert(key, id);
  // Frambuffer Format的关系
  FramebufferFormat fb_format;
  fb_format.E = E;
  fb_format.render_pass = render_pass;
  fb_format.pass_samples.push_back(p_samples);
  framebuffer_formats[id] = fb_format;
  return id;
}

RenderingDevice::TextureSamples RenderingDevice::framebuffer_format_get_texture_samples(FramebufferFormatID p_format, uint32_t p_pass) {
  HashMap<FramebufferFormatID, FramebufferFormat>::Iterator E = framebuffer_formats.find(p_format);
  ERR_FAIL_COND_V(!E, TEXTURE_SAMPLES_1);
  ERR_FAIL_COND_V(p_pass >= uint32_t(E->value.pass_samples.size()), TEXTURE_SAMPLES_1);

  return E->value.pass_samples[p_pass];
}

RID RenderingDevice::framebuffer_create(const Vector<RID>& p_texture_attachments, FramebufferFormatID p_format_check, uint32_t p_view_count) {
  _THREAD_SAFE_METHOD_
  FramebufferPass pass;

  for (int i = 0; i < p_texture_attachments.size(); i++) {
    Texture* texture = texture_owner.get_or_null(p_texture_attachments[i]);

    ERR_FAIL_COND_V_MSG(texture && texture->layers != p_view_count, RID(), "Layers of our texture doesn't match view count for this framebuffer");

    if (texture && texture->usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      pass.depth_attachment = i;
    } else if (texture && texture->usage_flags & TEXTURE_USAGE_VRS_ATTACHMENT_BIT) {
      pass.vrs_attachment = i;
    } else {
      if (texture && texture->is_resolve_buffer) {
        pass.resolve_attachments.push_back(i);
      } else {
        pass.color_attachments.push_back(texture ? i : ATTACHMENT_UNUSED);
      }
    }
  }

  Vector<FramebufferPass> passes;
  passes.push_back(pass);

  return framebuffer_create_multipass(p_texture_attachments, passes, p_format_check, p_view_count);
}
RID RenderingDevice::framebuffer_create_multipass(const Vector<RID>& p_texture_attachments, const Vector<FramebufferPass>& p_passes, FramebufferFormatID p_format_check,
                                                  uint32_t p_view_count) {
  _THREAD_SAFE_METHOD_

  Vector<AttachmentFormat> attachments;
  attachments.resize(p_texture_attachments.size());
  Size2i size;  // 要求与texture size相同
  bool size_set = false;
  AttachmentFormat* attachment_ptr = attachments.ptrw();
  for (int i = 0; i < p_texture_attachments.size(); i++) {
    AttachmentFormat af;
    Texture* texture = texture_owner.get_or_null(p_texture_attachments[i]);
    if (!texture) {
      af.usage_flags = AttachmentFormat::UNUSED_ATTACHMENT;
    } else {
      ERR_FAIL_COND_V_MSG(texture->layers != p_view_count, RID(), "Layers of our texture doesn't match view count for this framebuffer");
      // 验证size
      if (!size_set) {
        size.x = texture->width;
        size.y = texture->height;
        size_set = true;
      } else if (texture->usage_flags & TEXTURE_USAGE_VRS_ATTACHMENT_BIT) {
        // If this is not the first attachment we assume this is used as the VRS attachment.
        // In this case this texture will be 1/16th the size of the color attachment.
        // So we skip the size check.
      } else {
        ERR_FAIL_COND_V_MSG((uint32_t)size.x != texture->width || (uint32_t)size.y != texture->height, RID(), "All textures in a framebuffer should be the same size.");
      }

      af.format = texture->format;
      af.samples = texture->samples;
      af.usage_flags = texture->usage_flags;
    }
    attachment_ptr[i] = af;
  }

  ERR_FAIL_COND_V_MSG(!size_set, RID(), "All attachments unused.");

  FramebufferFormatID format_id = framebuffer_format_create_multipass(attachments, p_passes, p_view_count);
  if (format_id == INVALID_ID) {
    return RID();
  }

  ERR_FAIL_COND_V_MSG(p_format_check != INVALID_ID && format_id != p_format_check, RID(),
                      "The format used to check this framebuffer differs from the intended framebuffer format.");

  Framebuffer framebuffer;
  framebuffer.format_id = format_id;
  framebuffer.texture_ids = p_texture_attachments;
  framebuffer.size = size;
  framebuffer.view_count = p_view_count;

  RID id = framebuffer_owner.make_rid(framebuffer);
#ifdef DEV_ENABLED
  set_resource_name(id, "RID:" + itos(id.get_id()));
#endif

  for (int i = 0; i < p_texture_attachments.size(); i++) {
    if (p_texture_attachments[i].is_valid()) {
      _add_dependency(id, p_texture_attachments[i]);
    }
  }

  return id;
  // 注意这里只是新建了Frambuffer而没有新建RDD::framebuffer
  // 只有在draw_list里才会新建（需要这个frambuffer）
}

RID RenderingDevice::framebuffer_create_empty(const Size2i& p_size, TextureSamples p_samples, FramebufferFormatID p_format_check) {
  _THREAD_SAFE_METHOD_
  Framebuffer framebuffer;
  framebuffer.format_id = framebuffer_format_create_empty(p_samples);
  ERR_FAIL_COND_V(p_format_check != INVALID_FORMAT_ID && framebuffer.format_id != p_format_check, RID());
  framebuffer.size = p_size;
  framebuffer.view_count = 1;

  RID id = framebuffer_owner.make_rid(framebuffer);
#ifdef DEV_ENABLED
  set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
  return id;
}
RenderingDevice::FramebufferFormatID RenderingDevice::framebuffer_get_format(RID p_framebuffer) {
	_THREAD_SAFE_METHOD_

	Framebuffer *framebuffer = framebuffer_owner.get_or_null(p_framebuffer);
	ERR_FAIL_NULL_V(framebuffer, INVALID_ID);

	return framebuffer->format_id;
}

static RDD::AttachmentLoadOp initial_action_to_load_op(RenderingDevice::InitialAction p_action) {
  switch (p_action) {
    case RenderingDevice::INITIAL_ACTION_LOAD:
      return RDD::ATTACHMENT_LOAD_OP_LOAD;
    case RenderingDevice::INITIAL_ACTION_CLEAR:
      return RDD::ATTACHMENT_LOAD_OP_CLEAR;
    case RenderingDevice::INITIAL_ACTION_DISCARD:
      return RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
    default:
      ERR_FAIL_V_MSG(RDD::ATTACHMENT_LOAD_OP_DONT_CARE, "Invalid initial action value (" + itos(p_action) + ")");
  }
}
static RDD::AttachmentStoreOp final_action_to_store_op(RenderingDevice::FinalAction p_action) {
  switch (p_action) {
    case RenderingDevice::FINAL_ACTION_STORE:
      return RDD::ATTACHMENT_STORE_OP_STORE;
    case RenderingDevice::FINAL_ACTION_DISCARD:
      return RDD::ATTACHMENT_STORE_OP_DONT_CARE;
    default:
      ERR_FAIL_V_MSG(RDD::ATTACHMENT_STORE_OP_DONT_CARE, "Invalid final action value (" + itos(p_action) + ")");
  }
}

L_INLINE static RDD::AttachmentLoadOp color_action_to_load_op(RenderingDevice::ColorInitialAction p_action, int index) {
  if (p_action.load_attach & 1 << index) {
    return RDD::ATTACHMENT_LOAD_OP_LOAD;
  }
  if (p_action.clear_attach & 1 << index) {
    return RDD::ATTACHMENT_LOAD_OP_CLEAR;
  }
  if (p_action.discard_attach & 1 << index) {
    return RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
  }
  return RDD::ATTACHMENT_LOAD_OP_CLEAR;  // default
}
L_INLINE static RDD::AttachmentStoreOp color_action_to_store_op(RenderingDevice::ColorFinalAction p_action, int index) {
  if (p_action.store_attach & 1 << index) {
    return RDD::ATTACHMENT_STORE_OP_STORE;
  }
  if (p_action.discard_attach & 1 << index) {
    return RDD::ATTACHMENT_STORE_OP_DONT_CARE;
  }
  return RDD::ATTACHMENT_STORE_OP_STORE;
}

// 转移到RDD
RDD::RenderPassID RenderingDevice::_render_pass_create(const Vector<AttachmentFormat>& p_attachments, const Vector<FramebufferPass>& p_passes,
                                                       ColorInitialAction p_initial_action, ColorFinalAction p_final_action, InitialAction p_initial_depth_action,
                                                       FinalAction p_final_depth_action, uint32_t p_view_count, Vector<TextureSamples>* r_samples) {
  // NOTE:
  // Before the refactor to RenderingDevice-RenderingDeviceDriver, there was commented out code to
  // specify dependencies to external subpasses. Since it had been unused for a long timel it wasn't ported
  // to the new architecture.

  LocalVector<int32_t> attachment_last_pass;
  attachment_last_pass.resize(p_attachments.size());

  if (p_view_count > 1) {  // 多视图
    const RDD::MultiviewCapabilities& capabilities = driver->get_multiview_capabilities();

    // This only works with multiview!
    ERR_FAIL_COND_V_MSG(!capabilities.is_supported, RDD::RenderPassID(), "Multiview not supported");

    // Make sure we limit this to the number of views we support.
    ERR_FAIL_COND_V_MSG(p_view_count > capabilities.max_view_count, RDD::RenderPassID(), "Hardware does not support requested number of views for Multiview render pass");
  }

  LocalVector<RDD::Attachment> attachments;
  LocalVector<int> attachment_remap;

  for (int i = 0; i < p_attachments.size(); i++) {
    if (p_attachments[i].usage_flags == AttachmentFormat::UNUSED_ATTACHMENT) {
      attachment_remap.push_back(RDD::AttachmentReference::UNUSED);
      continue;
    }

    ERR_FAIL_INDEX_V(p_attachments[i].format, DATA_FORMAT_MAX, RDD::RenderPassID());
    ERR_FAIL_INDEX_V(p_attachments[i].samples, TEXTURE_SAMPLES_MAX, RDD::RenderPassID());
    ERR_FAIL_COND_V_MSG(!(p_attachments[i].usage_flags & (TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                                          TEXTURE_USAGE_INPUT_ATTACHMENT_BIT | TEXTURE_USAGE_VRS_ATTACHMENT_BIT)),
                        RDD::RenderPassID(), "Texture format for index (" + itos(i) + ") requires an attachment (color, depth-stencil, input or VRS) bit set.");

    RDD::Attachment description;
    description.format = p_attachments[i].format;
    description.samples = p_attachments[i].samples;

    // We can setup a framebuffer where we write to our VRS texture to set it up.
    // We make the assumption here that if our texture is actually used as our VRS attachment.
    // It is used as such for each subpass. This is fairly certain seeing the restrictions on subpasses.
    bool is_vrs = (p_attachments[i].usage_flags & TEXTURE_USAGE_VRS_ATTACHMENT_BIT) && i == p_passes[0].vrs_attachment;

    if (is_vrs) {
      description.load_op = RDD::ATTACHMENT_LOAD_OP_LOAD;
      description.store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
      description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_LOAD;
      description.stencil_store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
      description.initial_layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      description.final_layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    } else {
      if (p_attachments[i].usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT) {
        description.load_op = color_action_to_load_op(p_initial_action, i);
        description.store_op = color_action_to_store_op(p_final_action, i);
        description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
        description.stencil_store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
        description.initial_layout = RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        description.final_layout = RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      } else if (p_attachments[i].usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        description.load_op = initial_action_to_load_op(p_initial_depth_action);
        description.store_op = final_action_to_store_op(p_final_depth_action);
        description.stencil_load_op = initial_action_to_load_op(p_initial_depth_action);
        description.stencil_store_op = final_action_to_store_op(p_final_depth_action);
        description.initial_layout = RDD::TEXTURE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        description.final_layout = RDD::TEXTURE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      } else {
        description.load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
        description.store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
        description.stencil_load_op = RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
        description.stencil_store_op = RDD::ATTACHMENT_STORE_OP_DONT_CARE;
        description.initial_layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
        description.final_layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
      }
    }

    attachment_last_pass[i] = -1;
    attachment_remap.push_back(attachments.size());
    attachments.push_back(description);
  }

  LocalVector<RDD::Subpass> subpasses;
  subpasses.resize(p_passes.size());  // 将RD的framebuffer pass转换为RDD的subpass
  LocalVector<RDD::SubpassDependency> subpass_dependencies;
  // 写subpass
  for (int i = 0; i < p_passes.size(); i++) {
    const FramebufferPass* pass = &p_passes[i];
    RDD::Subpass& subpass = subpasses[i];

    TextureSamples texture_samples = TEXTURE_SAMPLES_1;
    bool is_multisample_first = true;

    for (int j = 0; j < pass->color_attachments.size(); j++) {
      int32_t attachment = pass->color_attachments[j];
      RDD::AttachmentReference reference;
      if (attachment == ATTACHMENT_UNUSED) {
        reference.attachment = RDD::AttachmentReference::UNUSED;
        reference.layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
      } else {
        ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(),
                             "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), color attachment (" + itos(j) + ").");
        // attachment必须有color attachment bit
        ERR_FAIL_COND_V_MSG(
            !(p_attachments[attachment].usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT), RDD::RenderPassID(),
            "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it's marked as depth, but it's not usable as color attachment.");
        ERR_FAIL_COND_V_MSG(
            attachment_last_pass[attachment] == i, RDD::RenderPassID(),
            "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it already was used for something else before in this pass.");

        if (is_multisample_first) {
          texture_samples = p_attachments[attachment].samples;
          is_multisample_first = false;
        } else {  // attachment需要相同的samples
          ERR_FAIL_COND_V_MSG(texture_samples != p_attachments[attachment].samples, RDD::RenderPassID(),
                              "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) +
                                  "), if an attachment is marked as multisample, all of them "
                                  "should be multisample and use the "
                                  "same number of samples.");
        }
        reference.attachment = attachment_remap[attachment];
        reference.layout = RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment_last_pass[attachment] = i;
      }
      reference.aspect = RDD::TEXTURE_ASPECT_COLOR_BIT;
      subpass.color_references.push_back(reference);
    }  // color_attachments, 并做好color_reference

    for (int j = 0; j < pass->input_attachments.size(); j++) {
      int32_t attachment = pass->input_attachments[j];
      RDD::AttachmentReference reference;
      if (attachment == ATTACHMENT_UNUSED) {
        reference.attachment = RDD::AttachmentReference::UNUSED;
        reference.layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
      } else {
        ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(),
                             "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), input attachment (" + itos(j) + ").");
        ERR_FAIL_COND_V_MSG(!(p_attachments[attachment].usage_flags & TEXTURE_USAGE_INPUT_ATTACHMENT_BIT), RDD::RenderPassID(),
                            "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it isn't marked as an input texture.");
        ERR_FAIL_COND_V_MSG(
            attachment_last_pass[attachment] == i, RDD::RenderPassID(),
            "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it already was used for something else before in this pass.");
        reference.attachment = attachment_remap[attachment];
        reference.layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;  // input attachments
        attachment_last_pass[attachment] = i;
      }
      reference.aspect = RDD::TEXTURE_ASPECT_COLOR_BIT;
      subpass.input_references.push_back(reference);
    }
    // 所有color_attachments都要有resolve_attachments
    if (pass->resolve_attachments.size() > 0) {
      ERR_FAIL_COND_V_MSG(
          pass->resolve_attachments.size() != pass->color_attachments.size(), RDD::RenderPassID(),
          "The amount of resolve attachments (" + itos(pass->resolve_attachments.size()) + ") must resolve attachments (" + itos(pass->color_attachments.size()) + ").");
      ERR_FAIL_COND_V_MSG(texture_samples == TEXTURE_SAMPLES_1, RDD::RenderPassID(), "Resolve attachments specified, but color attachments are not multisample.");
    }
    for (int j = 0; j < pass->resolve_attachments.size(); j++) {
      int32_t attachment = pass->resolve_attachments[j];
      RDD::AttachmentReference reference;
      if (attachment == ATTACHMENT_UNUSED) {
        reference.attachment = RDD::AttachmentReference::UNUSED;
        reference.layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
      } else {
        ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(),
                             "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), resolve attachment (" + itos(j) + ").");
        ERR_FAIL_COND_V_MSG(pass->color_attachments[j] == ATTACHMENT_UNUSED, RDD::RenderPassID(),
                            "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), resolve attachment (" + itos(j) +
                                "), the respective color attachment is marked as unused.");
        ERR_FAIL_COND_V_MSG(
            !(p_attachments[attachment].usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT), RDD::RenderPassID(),
            "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), resolve attachment, it isn't marked as a color texture.");
        ERR_FAIL_COND_V_MSG(
            attachment_last_pass[attachment] == i, RDD::RenderPassID(),
            "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it already was used for something else before in this pass.");
        bool multisample = p_attachments[attachment].samples > TEXTURE_SAMPLES_1;
        ERR_FAIL_COND_V_MSG(multisample, RDD::RenderPassID(),
                            "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), resolve attachments can't be multisample.");
        reference.attachment = attachment_remap[attachment];
        reference.layout = RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;  // RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        attachment_last_pass[attachment] = i;
      }
      reference.aspect = RDD::TEXTURE_ASPECT_COLOR_BIT;
      subpass.resolve_references.push_back(reference);
    }

    if (pass->depth_attachment != ATTACHMENT_UNUSED) {
      int32_t attachment = pass->depth_attachment;
      ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(),
                           "Invalid framebuffer depth format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), depth attachment.");
      ERR_FAIL_COND_V_MSG(
          !(p_attachments[attachment].usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT), RDD::RenderPassID(),
          "Invalid framebuffer depth format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it's marked as depth, but it's not a depth attachment.");
      ERR_FAIL_COND_V_MSG(
          attachment_last_pass[attachment] == i, RDD::RenderPassID(),
          "Invalid framebuffer depth format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it already was used for something else before in this pass.");
      subpass.depth_stencil_reference.attachment = attachment_remap[attachment];
      subpass.depth_stencil_reference.layout = RDD::TEXTURE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      attachment_last_pass[attachment] = i;
      // depth attachment需要相同的samples
      if (is_multisample_first) {
        texture_samples = p_attachments[attachment].samples;
        is_multisample_first = false;
      } else {
        ERR_FAIL_COND_V_MSG(texture_samples != p_attachments[attachment].samples, RDD::RenderPassID(),
                            "Invalid framebuffer depth format attachment(" + itos(attachment) + "), in pass (" + itos(i) +
                                "), if an attachment is marked as multisample, all of them should "
                                "be multisample and use the "
                                "same number of samples including the depth.");
      }

    } else {
      subpass.depth_stencil_reference.attachment = RDD::AttachmentReference::UNUSED;
      subpass.depth_stencil_reference.layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
    }

    if (pass->vrs_attachment != ATTACHMENT_UNUSED) {
      int32_t attachment = pass->vrs_attachment;
      ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(),
                           "Invalid framebuffer VRS format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), VRS attachment.");
      ERR_FAIL_COND_V_MSG(
          !(p_attachments[attachment].usage_flags & TEXTURE_USAGE_VRS_ATTACHMENT_BIT), RDD::RenderPassID(),
          "Invalid framebuffer VRS format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it's marked as VRS, but it's not a VRS attachment.");
      ERR_FAIL_COND_V_MSG(
          attachment_last_pass[attachment] == i, RDD::RenderPassID(),
          "Invalid framebuffer VRS attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it already was used for something else before in this pass.");

      subpass.vrs_reference.attachment = attachment_remap[attachment];
      subpass.vrs_reference.layout = RDD::TEXTURE_LAYOUT_VRS_ATTACHMENT_OPTIMAL;

      attachment_last_pass[attachment] = i;
    }
    // preserve_attachments和 load_op store有什么区别？
    for (int j = 0; j < pass->preserve_attachments.size(); j++) {
      int32_t attachment = pass->preserve_attachments[j];

      ERR_FAIL_COND_V_MSG(attachment == ATTACHMENT_UNUSED, RDD::RenderPassID(),
                          "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), preserve attachment (" + itos(j) +
                              "). Preserve attachments can't be unused.");

      ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(),
                           "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), preserve attachment (" + itos(j) + ").");

      if (attachment_last_pass[attachment] != i) {
        // Preserve can still be used to keep depth or color from being discarded after use.
        attachment_last_pass[attachment] = i;
        subpasses[i].preserve_attachments.push_back(attachment);
      }
    }

    if (r_samples) {
      r_samples->push_back(texture_samples);
    }
    // 默认renderpass之间有前后的依赖关系，这与rendergraph中的不同吗@？
    // 可能需要更细粒度的控制
    if (i > 0) {
      RDD::SubpassDependency dependency;
      dependency.src_subpass = i - 1;
      dependency.dst_subpass = i;
      dependency.src_stages = (RDD::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | RDD::PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | RDD::PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
      dependency.dst_stages = (RDD::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | RDD::PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | RDD::PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
                               RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
      dependency.src_access = (RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
      dependency.dst_access = (RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_READ_BIT | RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | RDD::BARRIER_ACCESS_INPUT_ATTACHMENT_READ_BIT);
      subpass_dependencies.push_back(dependency);
    }
  }

  RDD::RenderPassID render_pass = driver->render_pass_create(attachments, subpasses, subpass_dependencies, p_view_count);
  ERR_FAIL_COND_V(!render_pass, RDD::RenderPassID());

  return render_pass;
}

uint64_t lain::RenderingDevice::get_memory_usage(MemoryType p_type) const {
  switch(p_type){
    case MemoryType::MEMORY_TEXTURES:
      return texture_memory;
    case MemoryType::MEMORY_BUFFERS:
      return buffer_memory;
    case MemoryType::MEMORY_TOTAL:
      return driver->get_total_memory_used();
    default:{
      DEV_ASSERT(false);
      return 0;
    }

  }
}

RenderingDevice* lain::RenderingDevice::create_local_device() {
  RenderingDevice* rd = memnew(RenderingDevice);
  rd->initialize(context);
  return rd;
}

void RenderingDevice::set_resource_name(RID p_id, const String& p_name) {
  if (texture_owner.owns(p_id)) {
    Texture* texture = texture_owner.get_or_null(p_id);
    driver->set_object_name(RDD::OBJECT_TYPE_TEXTURE, texture->driver_id, p_name);
  } else if (framebuffer_owner.owns(p_id)) {
    //Framebuffer *framebuffer = framebuffer_owner.get_or_null(p_id);
    // Not implemented for now as the relationship between Framebuffer and RenderPass is very complex.
  } else if (sampler_owner.owns(p_id)) {
    RDD::SamplerID sampler_driver_id = *sampler_owner.get_or_null(p_id);
    driver->set_object_name(RDD::OBJECT_TYPE_SAMPLER, sampler_driver_id, p_name);
  } else if (vertex_buffer_owner.owns(p_id)) {
    Buffer* vertex_buffer = vertex_buffer_owner.get_or_null(p_id);
    driver->set_object_name(RDD::OBJECT_TYPE_BUFFER, vertex_buffer->driver_id, p_name);
  } else if (index_buffer_owner.owns(p_id)) {
    IndexBuffer* index_buffer = index_buffer_owner.get_or_null(p_id);
    driver->set_object_name(RDD::OBJECT_TYPE_BUFFER, index_buffer->driver_id, p_name);
  } else if (shader_owner.owns(p_id)) {
    Shader* shader = shader_owner.get_or_null(p_id);
    driver->set_object_name(RDD::OBJECT_TYPE_SHADER, shader->driver_id, p_name);
  } else if (uniform_buffer_owner.owns(p_id)) {
    Buffer* uniform_buffer = uniform_buffer_owner.get_or_null(p_id);
    driver->set_object_name(RDD::OBJECT_TYPE_BUFFER, uniform_buffer->driver_id, p_name);
  } else if (texture_buffer_owner.owns(p_id)) {
    Buffer* texture_buffer = texture_buffer_owner.get_or_null(p_id);
    driver->set_object_name(RDD::OBJECT_TYPE_BUFFER, texture_buffer->driver_id, p_name);
  } else if (storage_buffer_owner.owns(p_id)) {
    Buffer* storage_buffer = storage_buffer_owner.get_or_null(p_id);
    driver->set_object_name(RDD::OBJECT_TYPE_BUFFER, storage_buffer->driver_id, p_name);
  } else if (uniform_set_owner.owns(p_id)) {
    UniformSet* uniform_set = uniform_set_owner.get_or_null(p_id);
    driver->set_object_name(RDD::OBJECT_TYPE_UNIFORM_SET, uniform_set->driver_id, p_name);
  } else if (render_pipeline_owner.owns(p_id)) {
    RenderPipeline* pipeline = render_pipeline_owner.get_or_null(p_id);
    driver->set_object_name(RDD::OBJECT_TYPE_PIPELINE, pipeline->driver_id, p_name);
  } else if (compute_pipeline_owner.owns(p_id)) {
    ComputePipeline* pipeline = compute_pipeline_owner.get_or_null(p_id);
    driver->set_object_name(RDD::OBJECT_TYPE_PIPELINE, pipeline->driver_id, p_name);
  } else {
    ERR_PRINT("Attempted to name invalid ID: " + itos(p_id.get_id()));
    return;
  }
#ifdef DEV_ENABLED
  resource_names[p_id] = p_name;
#endif
}
void RenderingDevice::draw_command_begin_label(String p_label_name, const Color &p_color) {
	ERR_RENDER_THREAD_GUARD();

	if (!context->is_debug_utils_enabled()) {
		return;
	}

	draw_graph.begin_label(p_label_name, p_color);
}
void RenderingDevice::draw_command_end_label() {
	ERR_RENDER_THREAD_GUARD();

	draw_graph.end_label();
}

///************SHADER ************** */
///************SHADER ************** */
///************SHADER ************** */
// shader（记载了glsl的信息） 可以从spirv或者经过spirv编译后的binary构建。
// 从spirv构建其实也得编译，然后再构建
void RenderingDevice::shader_set_compile_to_spirv_function(ShaderCompileToSPIRVFunction p_function) {
  compile_to_spirv_function = p_function;
}

void RenderingDevice::shader_set_spirv_cache_function(ShaderCacheFunction p_function) {
  cache_function = p_function;
}

void RenderingDevice::shader_set_get_cache_key_function(ShaderSPIRVGetCacheKeyFunction p_function) {
  get_spirv_cache_key_function = p_function;
}

RID RenderingDevice::shader_create_from_spirv(const Vector<ShaderStageSPIRVData>& p_spirv, const String& p_shader_name) {
  Vector<uint8_t> bytecode = shader_compile_binary_from_spirv(p_spirv, p_shader_name);
  ERR_FAIL_COND_V(bytecode.is_empty(), RID());
  return shader_create_from_bytecode(bytecode);
}
RID RenderingDevice::shader_create_from_bytecode(const Vector<uint8_t>& p_shader_binary, RID p_placeholder) {
  _THREAD_SAFE_METHOD_

  ShaderDescription shader_desc;
  String name;
  RDD::ShaderID shader_id = driver->shader_create_from_bytecode(p_shader_binary, shader_desc, name);
  ERR_FAIL_COND_V(!shader_id, RID());

  // All good, let's create modules.

  RID id;
  if (p_placeholder.is_null()) {
    id = shader_owner.make_rid();
  } else {
    id = p_placeholder;
  }

  Shader* shader = shader_owner.get_or_null(id);
  ERR_FAIL_NULL_V(shader, RID());

  *((ShaderDescription*)shader) = shader_desc;  // ShaderDescription bundle.
  shader->name = name;
  shader->driver_id = shader_id;
  shader->layout_hash = driver->shader_get_layout_hash(shader_id);

  for (int i = 0; i < shader->uniform_sets.size(); i++) {
    uint32_t format = 0;  // No format, default.

    if (shader->uniform_sets[i].size()) {
      // Sort and hash.

      shader->uniform_sets.write[i].sort();

      UniformSetFormat usformat;
      usformat.uniforms = shader->uniform_sets[i];
      RBMap<UniformSetFormat, uint32_t>::Element* E = uniform_set_format_cache.find(usformat);
      if (E) {
        format = E->get();
      } else {
        format = uniform_set_format_cache.size() + 1;
        uniform_set_format_cache.insert(usformat, format);
      }
    }

    shader->set_formats.push_back(format);
  }

  for (ShaderStage stage : shader_desc.stages) {
    switch (stage) {
      case SHADER_STAGE_VERTEX:
        shader->stage_bits.set_flag(RDD::PIPELINE_STAGE_VERTEX_SHADER_BIT);
        break;
      case SHADER_STAGE_FRAGMENT:
        shader->stage_bits.set_flag(RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        break;
      case SHADER_STAGE_TESSELATION_CONTROL:
        shader->stage_bits.set_flag(RDD::PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT);
        break;
      case SHADER_STAGE_TESSELATION_EVALUATION:
        shader->stage_bits.set_flag(RDD::PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT);
        break;
      case SHADER_STAGE_COMPUTE:
        shader->stage_bits.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        break;
      default:
        DEV_ASSERT(false && "Unknown shader stage.");
        break;
    }
  }

#ifdef DEV_ENABLED
  set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
  return id;
}

RID RenderingDevice::shader_create_placeholder() {
	Shader shader;
	return shader_owner.make_rid(shader);
}
uint64_t RenderingDevice::shader_get_vertex_input_attribute_mask(RID p_shader) {
	_THREAD_SAFE_METHOD_

	const Shader *shader = shader_owner.get_or_null(p_shader);
	ERR_FAIL_NULL_V(shader, 0);
	return shader->vertex_input_mask;
}

/// Shader compile
Vector<uint8_t> RenderingDevice::shader_compile_spirv_from_source(ShaderStage p_stage, const String& p_source_code, ShaderLanguage p_language, String* r_error,
                                                                  bool p_allow_cache) {
  if (p_allow_cache && cache_function) {
    Vector<uint8_t> cache = cache_function(p_stage, p_source_code, p_language);
    if (cache.size()) {
      return cache;
    }
  }

  ERR_FAIL_NULL_V(compile_to_spirv_function, Vector<uint8_t>());
  // 这个function是glslang模组的 （see module/glslang）
  return compile_to_spirv_function(p_stage, p_source_code, p_language, r_error, this);
}
Vector<uint8_t> RenderingDevice::shader_compile_binary_from_spirv(const Vector<ShaderStageSPIRVData>& p_spirv, const String& p_shader_name) {
  return driver->shader_compile_binary_from_spirv(p_spirv, p_shader_name);
}

static const auto get_formatkey = [](RD::FramebufferFormatID p_id) -> const RD::FramebufferFormatKey& {
  return RD::get_singleton()->framebuffer_formats[p_id].E->key();
};

RID RenderingDevice::render_pipeline_create(RID p_shader, FramebufferFormatID p_framebuffer_format, VertexFormatID p_vertex_format, RenderPrimitive p_render_primitive,
                                            const PipelineRasterizationState& p_rasterization_state, const PipelineMultisampleState& p_multisample_state,
                                            const PipelineDepthStencilState& p_depth_stencil_state, const PipelineColorBlendState& p_blend_state,
                                            BitField<PipelineDynamicStateFlags> p_dynamic_state_flags, uint32_t p_for_render_pass,
                                            const Vector<PipelineSpecializationConstant>& p_specialization_constants) {
  _THREAD_SAFE_METHOD_

  // Needs a shader.
  Shader* shader = shader_owner.get_or_null(p_shader);
  ERR_FAIL_NULL_V(shader, RID());

  ERR_FAIL_COND_V_MSG(shader->is_compute, RID(), "Compute shaders can't be used in render pipelines");

  if (p_framebuffer_format == INVALID_ID) {
    // If nothing provided, use an empty one (no attachments).
    p_framebuffer_format = framebuffer_format_create(Vector<AttachmentFormat>());
  }
  ERR_FAIL_COND_V(!framebuffer_formats.has(p_framebuffer_format), RID());
  const FramebufferFormat& fb_format = framebuffer_formats[p_framebuffer_format];

  // Validate shader vs. framebuffer.
  {
    ERR_FAIL_COND_V_MSG(p_for_render_pass >= uint32_t(fb_format.E->key().passes.size()), RID(),
                        "Render pass requested for pipeline creation (" + itos(p_for_render_pass) + ") is out of bounds");
    const FramebufferPass& pass = fb_format.E->key().passes[p_for_render_pass];
    uint32_t output_mask = 0;
    for (int i = 0; i < pass.color_attachments.size(); i++) {
      if (pass.color_attachments[i] != ATTACHMENT_UNUSED) {
        output_mask |= 1 << i;
      }
    }  // fragment_output_mask 需要与framebuffer的color attachment 一致
    ERR_FAIL_COND_V_MSG(shader->fragment_output_mask != output_mask, RID(),
                        "Mismatch fragment shader output mask (" + itos(shader->fragment_output_mask) + ") and framebuffer color output mask (" + itos(output_mask) +
                            ") when binding both in render pipeline.");
  }

  RDD::VertexFormatID driver_vertex_format;
  if (p_vertex_format != INVALID_ID) {  // p_vertex_format指向VertexDescriptionCache，即内容和driver中的ID
    // Uses vertices, else it does not.
    ERR_FAIL_COND_V(!vertex_formats.has(p_vertex_format), RID());
    const VertexDescriptionInCache& vd = vertex_formats[p_vertex_format];
    driver_vertex_format = vertex_formats[p_vertex_format].driver_id;

    // Validate with inputs.
    for (uint32_t i = 0; i < 64; i++) {
      if (!(shader->vertex_input_mask & ((uint64_t)1) << i)) {
        continue;
      }
      bool found = false;
      for (int j = 0; j < vd.vertex_formats.size(); j++) {
        if (vd.vertex_formats[j].location == i) {
          found = true;
        }
      }

      ERR_FAIL_COND_V_MSG(!found, RID(), "Shader vertex input location (" + itos(i) + ") not provided in vertex input description for pipeline creation.");
    }

  } else {
    ERR_FAIL_COND_V_MSG(shader->vertex_input_mask != 0, RID(),
                        "Shader contains vertex inputs, but no vertex input description was "
                        "provided for pipeline creation.");
  }

  // 验证
  ERR_FAIL_INDEX_V(p_render_primitive, RENDER_PRIMITIVE_MAX, RID());

  ERR_FAIL_INDEX_V(p_rasterization_state.cull_mode, 3, RID());

  if (p_multisample_state.sample_mask.size()) {
    // Use sample mask.
    ERR_FAIL_COND_V((int)TEXTURE_SAMPLES_COUNT[p_multisample_state.sample_count] != p_multisample_state.sample_mask.size(), RID());
  }

  ERR_FAIL_INDEX_V(p_depth_stencil_state.depth_compare_operator, COMPARE_OP_MAX, RID());

  ERR_FAIL_INDEX_V(p_depth_stencil_state.front_op.fail, STENCIL_OP_MAX, RID());
  ERR_FAIL_INDEX_V(p_depth_stencil_state.front_op.pass, STENCIL_OP_MAX, RID());
  ERR_FAIL_INDEX_V(p_depth_stencil_state.front_op.depth_fail, STENCIL_OP_MAX, RID());
  ERR_FAIL_INDEX_V(p_depth_stencil_state.front_op.compare, COMPARE_OP_MAX, RID());

  ERR_FAIL_INDEX_V(p_depth_stencil_state.back_op.fail, STENCIL_OP_MAX, RID());
  ERR_FAIL_INDEX_V(p_depth_stencil_state.back_op.pass, STENCIL_OP_MAX, RID());
  ERR_FAIL_INDEX_V(p_depth_stencil_state.back_op.depth_fail, STENCIL_OP_MAX, RID());
  ERR_FAIL_INDEX_V(p_depth_stencil_state.back_op.compare, COMPARE_OP_MAX, RID());

  ERR_FAIL_INDEX_V(p_blend_state.logic_op, LOGIC_OP_MAX, RID());

  const FramebufferPass& pass = fb_format.E->key().passes[p_for_render_pass];
  ERR_FAIL_COND_V(p_blend_state.attachments.size() < pass.color_attachments.size(), RID());
  // 验证
  for (int i = 0; i < pass.color_attachments.size(); i++) {
    if (pass.color_attachments[i] != ATTACHMENT_UNUSED) {
      ERR_FAIL_INDEX_V(p_blend_state.attachments[i].src_color_blend_factor, BLEND_FACTOR_MAX, RID());
      ERR_FAIL_INDEX_V(p_blend_state.attachments[i].dst_color_blend_factor, BLEND_FACTOR_MAX, RID());
      ERR_FAIL_INDEX_V(p_blend_state.attachments[i].color_blend_op, BLEND_OP_MAX, RID());

      ERR_FAIL_INDEX_V(p_blend_state.attachments[i].src_alpha_blend_factor, BLEND_FACTOR_MAX, RID());
      ERR_FAIL_INDEX_V(p_blend_state.attachments[i].dst_alpha_blend_factor, BLEND_FACTOR_MAX, RID());
      ERR_FAIL_INDEX_V(p_blend_state.attachments[i].alpha_blend_op, BLEND_OP_MAX, RID());
    }
  }

  for (int i = 0; i < shader->specialization_constants.size(); i++) {
    const ShaderSpecializationConstant& sc = shader->specialization_constants[i];
    for (int j = 0; j < p_specialization_constants.size(); j++) {
      const PipelineSpecializationConstant& psc = p_specialization_constants[j];
      if (psc.constant_id == sc.constant_id) {
        ERR_FAIL_COND_V_MSG(psc.type != sc.type, RID(), "Specialization constant provided for id (" + itos(sc.constant_id) + ") is of the wrong type.");
        break;
      }
    }
  }

  RenderPipeline pipeline;
  pipeline.driver_id =
      driver->render_pipeline_create(shader->driver_id, driver_vertex_format, p_render_primitive, p_rasterization_state, p_multisample_state, p_depth_stencil_state,
                                     p_blend_state, pass.color_attachments, p_dynamic_state_flags, fb_format.render_pass, p_for_render_pass, p_specialization_constants);
  ERR_FAIL_COND_V(!pipeline.driver_id, RID());

  if (pipeline_cache_enabled) {
    _update_pipeline_cache();
  }

  pipeline.shader = p_shader;
  pipeline.shader_driver_id = shader->driver_id;
  pipeline.shader_layout_hash = shader->layout_hash;
  pipeline.set_formats = shader->set_formats;
  pipeline.push_constant_size = shader->push_constant_size;
  pipeline.stage_bits = shader->stage_bits;

#ifdef DEBUG_ENABLED
  pipeline.validation.dynamic_state = p_dynamic_state_flags;
  pipeline.validation.framebuffer_format = p_framebuffer_format;
  pipeline.validation.render_pass = p_for_render_pass;
  pipeline.validation.vertex_format = p_vertex_format;
  pipeline.validation.uses_restart_indices = p_render_primitive == RENDER_PRIMITIVE_TRIANGLE_STRIPS_WITH_RESTART_INDEX;

  static const uint32_t primitive_divisor[RENDER_PRIMITIVE_MAX] = {1, 2, 1, 1, 1, 3, 1, 1, 1, 1, 1};
  pipeline.validation.primitive_divisor = primitive_divisor[p_render_primitive];
  static const uint32_t primitive_minimum[RENDER_PRIMITIVE_MAX] = {
      1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 1,
  };
  pipeline.validation.primitive_minimum = primitive_minimum[p_render_primitive];
#endif
  // Create ID to associate with this pipeline.
  RID id = render_pipeline_owner.make_rid(pipeline);
#ifdef DEV_ENABLED
  set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
  // Now add all the dependencies.
  _add_dependency(id, p_shader);
  return id;
}

bool RenderingDevice::render_pipeline_is_valid(RID p_pipeline) {
  _THREAD_SAFE_METHOD_
  return render_pipeline_owner.owns(p_pipeline);
}

RID RenderingDevice::compute_pipeline_create(RID p_shader, const Vector<PipelineSpecializationConstant>& p_specialization_constants) {
  _THREAD_SAFE_METHOD_

  // Needs a shader.
  Shader* shader = shader_owner.get_or_null(p_shader);
  ERR_FAIL_NULL_V(shader, RID());

  ERR_FAIL_COND_V_MSG(!shader->is_compute, RID(), "Non-compute shaders can't be used in compute pipelines");

  for (int i = 0; i < shader->specialization_constants.size(); i++) {
    const ShaderSpecializationConstant& sc = shader->specialization_constants[i];
    for (int j = 0; j < p_specialization_constants.size(); j++) {
      const PipelineSpecializationConstant& psc = p_specialization_constants[j];
      if (psc.constant_id == sc.constant_id) {
        ERR_FAIL_COND_V_MSG(psc.type != sc.type, RID(), "Specialization constant provided for id (" + itos(sc.constant_id) + ") is of the wrong type.");
        break;
      }
    }
  }

  ComputePipeline pipeline;
  pipeline.driver_id = driver->compute_pipeline_create(shader->driver_id, p_specialization_constants);
  ERR_FAIL_COND_V(!pipeline.driver_id, RID());

  if (pipeline_cache_enabled) {
    _update_pipeline_cache();
  }

  pipeline.shader = p_shader;
  pipeline.shader_driver_id = shader->driver_id;
  pipeline.shader_layout_hash = shader->layout_hash;
  pipeline.set_formats = shader->set_formats;
  pipeline.push_constant_size = shader->push_constant_size;
  pipeline.local_group_size[0] = shader->compute_local_size[0];
  pipeline.local_group_size[1] = shader->compute_local_size[1];
  pipeline.local_group_size[2] = shader->compute_local_size[2];

  // Create ID to associate with this pipeline.
  RID id = compute_pipeline_owner.make_rid(pipeline);
#ifdef DEV_ENABLED
  set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
  // Now add all the dependencies.
  _add_dependency(id, p_shader);
  return id;
}

/**************************/
/**** FRAME MANAGEMENT ****/
/**************************/
/// free RID,
/// 1. free_dependencies
/// 2. free_internal
void RenderingDevice::free(RID p_id) {
  _THREAD_SAFE_METHOD_

  _free_dependencies(p_id);  // Recursively erase dependencies first, to avoid potential API problems.
  _free_internal(p_id);
}

void lain::RenderingDevice::_free_pending_resources(int p_frame) {
  // Free in dependency usage order, so nothing weird happens.
  // Pipelines.
  while (frames[p_frame].render_pipelines_to_dispose_of.front()) {
    RenderPipeline* pipeline = &frames[p_frame].render_pipelines_to_dispose_of.front()->get();

    driver->pipeline_free(pipeline->driver_id);

    frames[p_frame].render_pipelines_to_dispose_of.pop_front();
  }

  while (frames[p_frame].compute_pipelines_to_dispose_of.front()) {
    ComputePipeline* pipeline = &frames[p_frame].compute_pipelines_to_dispose_of.front()->get();

    driver->pipeline_free(pipeline->driver_id);

    frames[p_frame].compute_pipelines_to_dispose_of.pop_front();
  }

  // Uniform sets.
  while (frames[p_frame].uniform_sets_to_dispose_of.front()) {
    UniformSet* uniform_set = &frames[p_frame].uniform_sets_to_dispose_of.front()->get();

    driver->uniform_set_free(uniform_set->driver_id);

    frames[p_frame].uniform_sets_to_dispose_of.pop_front();
  }

  // Shaders.
  while (frames[p_frame].shaders_to_dispose_of.front()) {
    Shader* shader = &frames[p_frame].shaders_to_dispose_of.front()->get();

    driver->shader_free(shader->driver_id);

    frames[p_frame].shaders_to_dispose_of.pop_front();
  }

  // Samplers.
  while (frames[p_frame].samplers_to_dispose_of.front()) {
    RDD::SamplerID sampler = frames[p_frame].samplers_to_dispose_of.front()->get();

    driver->sampler_free(sampler);

    frames[p_frame].samplers_to_dispose_of.pop_front();
  }

  // Framebuffers.
  while (frames[p_frame].framebuffers_to_dispose_of.front()) {
    Framebuffer* framebuffer = &frames[p_frame].framebuffers_to_dispose_of.front()->get();

    for (const KeyValue<Framebuffer::VersionKey, Framebuffer::Version>& E : framebuffer->framebuffers) {
      // First framebuffer, then render pass because it depends on it.
      driver->framebuffer_free(E.value.framebuffer);
      driver->render_pass_free(E.value.render_pass);
    }

    frames[p_frame].framebuffers_to_dispose_of.pop_front();
  }

  // Textures.
  while (frames[p_frame].textures_to_dispose_of.front()) {
    Texture* texture = &frames[p_frame].textures_to_dispose_of.front()->get();
    if (texture->bound) {
      WARN_PRINT("Deleted a texture while it was bound.");
    }

    _texture_free_shared_fallback(texture);

    texture_memory -= driver->texture_get_allocation_size(texture->driver_id);
    driver->texture_free(texture->driver_id);

    frames[p_frame].textures_to_dispose_of.pop_front();
  }

  // Buffers.
  while (frames[p_frame].buffers_to_dispose_of.front()) {
    Buffer& buffer = frames[p_frame].buffers_to_dispose_of.front()->get();
    driver->buffer_free(buffer.driver_id);
    buffer_memory -= buffer.size;

    frames[p_frame].buffers_to_dispose_of.pop_front();
  }
}

void RenderingDevice::_free_internal(RID p_id) {
#ifdef DEV_ENABLED
  String resource_name;
  if (resource_names.has(p_id)) {
    resource_name = resource_names[p_id];
    resource_names.erase(p_id);
  }
#endif

  // Push everything so it's disposed of next time this frame index is processed (means, it's safe to do it).
  if (texture_owner.owns(p_id)) {
    Texture* texture = texture_owner.get_or_null(p_id);
    RDG::ResourceTracker* draw_tracker = texture->draw_tracker;
    if (draw_tracker != nullptr) {
      draw_tracker->ref_count--;
      if (draw_tracker->ref_count == 0) {
        RDG::resource_tracker_free(draw_tracker);

        if (texture->owner.is_valid() && (texture->slice_type != TEXTURE_SLICE_MAX)) {
          // If this was a texture slice, erase the tracker from the map.
          Texture* owner_texture = texture_owner.get_or_null(texture->owner);
          if (owner_texture != nullptr) {
            owner_texture->slice_trackers.erase(texture->slice_rect);
          }
        }
      }
    }

    frames[frame].textures_to_dispose_of.push_back(*texture);
    texture_owner.free(p_id);
  } else if (framebuffer_owner.owns(p_id)) {
    Framebuffer* framebuffer = framebuffer_owner.get_or_null(p_id);
    frames[frame].framebuffers_to_dispose_of.push_back(*framebuffer);

    if (framebuffer->invalidated_callback != nullptr) {
      framebuffer->invalidated_callback(framebuffer->invalidated_callback_userdata);
    }

    framebuffer_owner.free(p_id);
  } else if (sampler_owner.owns(p_id)) {
    RDD::SamplerID sampler_driver_id = *sampler_owner.get_or_null(p_id);
    frames[frame].samplers_to_dispose_of.push_back(sampler_driver_id);
    sampler_owner.free(p_id);
  } else if (vertex_buffer_owner.owns(p_id)) {
    Buffer* vertex_buffer = vertex_buffer_owner.get_or_null(p_id);
    RDG::resource_tracker_free(vertex_buffer->draw_tracker);
    frames[frame].buffers_to_dispose_of.push_back(*vertex_buffer);
    vertex_buffer_owner.free(p_id);
  } else if (vertex_array_owner.owns(p_id)) {
    vertex_array_owner.free(p_id);
  } else if (index_buffer_owner.owns(p_id)) {
    IndexBuffer* index_buffer = index_buffer_owner.get_or_null(p_id);
    RDG::resource_tracker_free(index_buffer->draw_tracker);
    frames[frame].buffers_to_dispose_of.push_back(*index_buffer);
    index_buffer_owner.free(p_id);
  } else if (index_array_owner.owns(p_id)) {
    index_array_owner.free(p_id);
  } else if (shader_owner.owns(p_id)) {
    Shader* shader = shader_owner.get_or_null(p_id);
    if (shader->driver_id) {  // Not placeholder?
      frames[frame].shaders_to_dispose_of.push_back(*shader);
    }
    shader_owner.free(p_id);
  } else if (uniform_buffer_owner.owns(p_id)) {
    Buffer* uniform_buffer = uniform_buffer_owner.get_or_null(p_id);
    RDG::resource_tracker_free(uniform_buffer->draw_tracker);
    frames[frame].buffers_to_dispose_of.push_back(*uniform_buffer);
    uniform_buffer_owner.free(p_id);
  } else if (texture_buffer_owner.owns(p_id)) {
    Buffer* texture_buffer = texture_buffer_owner.get_or_null(p_id);
    RDG::resource_tracker_free(texture_buffer->draw_tracker);
    frames[frame].buffers_to_dispose_of.push_back(*texture_buffer);
    texture_buffer_owner.free(p_id);
  } else if (storage_buffer_owner.owns(p_id)) {
    Buffer* storage_buffer = storage_buffer_owner.get_or_null(p_id);
    RDG::resource_tracker_free(storage_buffer->draw_tracker);
    frames[frame].buffers_to_dispose_of.push_back(*storage_buffer);
    storage_buffer_owner.free(p_id);
  } else if (uniform_set_owner.owns(p_id)) {
    UniformSet* uniform_set = uniform_set_owner.get_or_null(p_id);
    frames[frame].uniform_sets_to_dispose_of.push_back(*uniform_set);
    uniform_set_owner.free(p_id);

    if (uniform_set->invalidated_callback != nullptr) {
      uniform_set->invalidated_callback(uniform_set->invalidated_callback_userdata);
    }
  } else if (render_pipeline_owner.owns(p_id)) {
    RenderPipeline* pipeline = render_pipeline_owner.get_or_null(p_id);
    frames[frame].render_pipelines_to_dispose_of.push_back(*pipeline);
    render_pipeline_owner.free(p_id);
  } else if (compute_pipeline_owner.owns(p_id)) {
    ComputePipeline* pipeline = compute_pipeline_owner.get_or_null(p_id);
    frames[frame].compute_pipelines_to_dispose_of.push_back(*pipeline);
    compute_pipeline_owner.free(p_id);
  } else {
#ifdef DEV_ENABLED
    ERR_PRINT("Attempted to free invalid ID: " + itos(p_id.get_id()) + " " + resource_name);
#else
    ERR_PRINT("Attempted to free invalid ID: " + itos(p_id.get_id()));
#endif
  }
}
///
/// ***** compute list ****
///

RenderingDevice::ComputeListID RenderingDevice::compute_list_begin() {
  _THREAD_SAFE_METHOD_

  ERR_FAIL_COND_V_MSG(compute_list != nullptr, INVALID_ID, "Only one draw/compute list can be active at the same time.");

  // Lock while compute_list is active.
  _THREAD_SAFE_LOCK_

  compute_list = memnew(ComputeList);

  draw_graph.add_compute_list_begin();

  return ID_TYPE_COMPUTE_LIST;
}
void RenderingDevice::compute_list_bind_compute_pipeline(ComputeListID p_list, RID p_compute_pipeline) {

  // Must be called within a compute list, the class mutex is locked during that time

  ERR_FAIL_COND(p_list != ID_TYPE_COMPUTE_LIST);
  ERR_FAIL_NULL(compute_list);

  ComputeList* cl = compute_list;

  const ComputePipeline* pipeline = compute_pipeline_owner.get_or_null(p_compute_pipeline);
  ERR_FAIL_NULL(pipeline);

  if (p_compute_pipeline == cl->state.pipeline) {
    return;  // Redundant state, return.
  }

  cl->state.pipeline = p_compute_pipeline;

  draw_graph.add_compute_list_bind_pipeline(pipeline->driver_id);
#ifdef DEBUG_ENABLED
  // Update compute pass pipeline info.
  cl->validation.pipeline_active = true;
  cl->validation.pipeline_push_constant_size = pipeline->push_constant_size;
#endif
  if (cl->state.pipeline_shader == pipeline->shader)
    return;
  // Shader changed, so descriptor sets may become incompatible.

  uint32_t pcount = pipeline->set_formats.size();  // Formats count in this pipeline.
  cl->state.set_count = MAX(cl->state.set_count, pcount);
  const uint32_t* pformats = pipeline->set_formats.ptr();  // Pipeline set formats.

  uint32_t first_invalid_set = UINT32_MAX;  // All valid by default. /// 从何处开始invalid，根据api_trait不同。
  switch (driver->api_trait_get(RDD::API_TRAIT_SHADER_CHANGE_INVALIDATION)) {
    case RDD::SHADER_CHANGE_INVALIDATION_ALL_BOUND_UNIFORM_SETS: {
      first_invalid_set = 0;
    } break;
    case RDD::SHADER_CHANGE_INVALIDATION_INCOMPATIBLE_SETS_PLUS_CASCADE: {
      for (uint32_t i = 0; i < pcount; i++) {
        if (cl->state.sets[i].pipeline_expected_format != pformats[i]) {
          first_invalid_set = i;
          break;
        }
      }
    } break;
    case RDD::SHADER_CHANGE_INVALIDATION_ALL_OR_NONE_ACCORDING_TO_LAYOUT_HASH: {
      if (cl->state.pipeline_shader_layout_hash != pipeline->shader_layout_hash) {
        first_invalid_set = 0;
      }
    } break;
  }

  for (uint32_t i = 0; i < pcount; i++) {
    cl->state.sets[i].bound = cl->state.sets[i].bound && i < first_invalid_set;
    cl->state.sets[i].pipeline_expected_format = pformats[i];  // 绑定管道时设置为期望
  }  // compute_list_bind_uniform_set时候修改实际的RID

  for (uint32_t i = pcount; i < cl->state.set_count; i++) {
    // Unbind the ones above (not used) if exist.
    cl->state.sets[i].bound = false;
  }

  cl->state.set_count = pcount;  // Update set count.

  if (pipeline->push_constant_size) {
#ifdef DEBUG_ENABLED
    cl->validation.pipeline_push_constant_supplied = false;
#endif
  }

  cl->state.pipeline_shader = pipeline->shader;
  cl->state.pipeline_shader_driver_id = pipeline->shader_driver_id;
  cl->state.pipeline_shader_layout_hash = pipeline->shader_layout_hash;
  cl->state.local_group_size[0] = pipeline->local_group_size[0];
  cl->state.local_group_size[1] = pipeline->local_group_size[1];
  cl->state.local_group_size[2] = pipeline->local_group_size[2];
}

void RenderingDevice::compute_list_bind_uniform_set(ComputeListID p_list, RID p_uniform_set, uint32_t p_index) {
  // Must be called within a compute list, the class mutex is locked during that time

  ERR_FAIL_COND(p_list != ID_TYPE_COMPUTE_LIST);
  ERR_FAIL_NULL(compute_list);

  ComputeList* cl = compute_list;

#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(
      p_index >= driver->limit_get(LIMIT_MAX_BOUND_UNIFORM_SETS) || p_index >= MAX_UNIFORM_SETS,
      "Attempting to bind a descriptor set (" + itos(p_index) + ") greater than what the hardware supports (" + itos(driver->limit_get(LIMIT_MAX_BOUND_UNIFORM_SETS)) + ").");
#endif

#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(!cl->validation.active, "Submitted Compute Lists can no longer be modified.");
#endif

  UniformSet* uniform_set = uniform_set_owner.get_or_null(p_uniform_set);
  ERR_FAIL_NULL(uniform_set);

  if (p_index > cl->state.set_count) {
    cl->state.set_count = p_index;
  }

  cl->state.sets[p_index].uniform_set_driver_id = uniform_set->driver_id;  // Update set pointer.
  cl->state.sets[p_index].bound = false;                                   // Needs rebind.
  cl->state.sets[p_index].uniform_set_format = uniform_set->format;        // 绑定uniform时不验证与pipeline 是否一致。
  cl->state.sets[p_index].uniform_set = p_uniform_set;

#if 0
	{ // Validate that textures bound are not attached as framebuffer bindings.
		uint32_t attachable_count = uniform_set->attachable_textures.size();
		const RID *attachable_ptr = uniform_set->attachable_textures.ptr();
		uint32_t bound_count = draw_list_bound_textures.size();
		const RID *bound_ptr = draw_list_bound_textures.ptr();
		for (uint32_t i = 0; i < attachable_count; i++) {
			for (uint32_t j = 0; j < bound_count; j++) {
				ERR_FAIL_COND_MSG(attachable_ptr[i] == bound_ptr[j],
						"Attempted to use the same texture in framebuffer attachment and a uniform set, this is not allowed.");
			}
		}
	}
#endif
}

void RenderingDevice::compute_list_set_push_constant(ComputeListID p_list, const void* p_data, uint32_t p_data_size) {
  ERR_FAIL_COND(p_list != ID_TYPE_COMPUTE_LIST);
  ERR_FAIL_NULL(compute_list);
  ERR_FAIL_COND_MSG(p_data_size > MAX_PUSH_CONSTANT_SIZE, "Push constants can't be bigger than 128 bytes to maintain compatibility.");

  ComputeList* cl = compute_list;

#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(!cl->validation.active, "Submitted Compute Lists can no longer be modified.");
#endif

#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(p_data_size != cl->validation.pipeline_push_constant_size, "This compute pipeline requires (" + itos(cl->validation.pipeline_push_constant_size) +
                                                                                   ") bytes of push constant data, supplied: (" + itos(p_data_size) + ")");
#endif

  draw_graph.add_compute_list_set_push_constant(cl->state.pipeline_shader_driver_id, p_data, p_data_size);

  // Store it in the state in case we need to restart the compute list. @?
  memcpy(cl->state.push_constant_data, p_data, p_data_size);
  cl->state.push_constant_size = p_data_size;

#ifdef DEBUG_ENABLED
  cl->validation.pipeline_push_constant_supplied = true;
#endif
}

void RenderingDevice::compute_list_dispatch(ComputeListID p_list, uint32_t p_x_groups, uint32_t p_y_groups, uint32_t p_z_groups) {
  // Must be called within a compute list, the class mutex is locked during that time

  ERR_FAIL_COND(p_list != ID_TYPE_COMPUTE_LIST);
  ERR_FAIL_NULL(compute_list);

  ComputeList* cl = compute_list;

#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(p_x_groups == 0, "Dispatch amount of X compute groups (" + itos(p_x_groups) + ") is zero.");
  ERR_FAIL_COND_MSG(p_z_groups == 0, "Dispatch amount of Z compute groups (" + itos(p_z_groups) + ") is zero.");
  ERR_FAIL_COND_MSG(p_y_groups == 0, "Dispatch amount of Y compute groups (" + itos(p_y_groups) + ") is zero.");
  ERR_FAIL_COND_MSG(
      p_x_groups > driver->limit_get(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_X),
      "Dispatch amount of X compute groups (" + itos(p_x_groups) + ") is larger than device limit (" + itos(driver->limit_get(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_X)) + ")");
  ERR_FAIL_COND_MSG(
      p_y_groups > driver->limit_get(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Y),
      "Dispatch amount of Y compute groups (" + itos(p_y_groups) + ") is larger than device limit (" + itos(driver->limit_get(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Y)) + ")");
  ERR_FAIL_COND_MSG(
      p_z_groups > driver->limit_get(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Z),
      "Dispatch amount of Z compute groups (" + itos(p_z_groups) + ") is larger than device limit (" + itos(driver->limit_get(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Z)) + ")");

  ERR_FAIL_COND_MSG(!cl->validation.active, "Submitted Compute Lists can no longer be modified.");
#endif

#ifdef DEBUG_ENABLED

  ERR_FAIL_COND_MSG(!cl->validation.pipeline_active, "No compute pipeline was set before attempting to draw.");

  if (cl->validation.pipeline_push_constant_size > 0) {
    // Using push constants, check that they were supplied.
    ERR_FAIL_COND_MSG(!cl->validation.pipeline_push_constant_supplied,
                      "The shader in this pipeline requires a push constant to be set before "
                      "drawing, but it's not present.");
  }

#endif

#ifdef DEBUG_ENABLED
  for (uint32_t i = 0; i < cl->state.set_count; i++) {
    if (cl->state.sets[i].pipeline_expected_format == 0) {
      // Nothing expected by this pipeline.
      continue;
    }
    // 需要相同
    if (cl->state.sets[i].pipeline_expected_format != cl->state.sets[i].uniform_set_format) {
      if (cl->state.sets[i].uniform_set_format == 0) {
        ERR_FAIL_MSG("Uniforms were never supplied for set (" + itos(i) + ") at the time of drawing, which are required by the pipeline.");
      } else if (uniform_set_owner.owns(cl->state.sets[i].uniform_set)) {
        UniformSet* us = uniform_set_owner.get_or_null(cl->state.sets[i].uniform_set);
        ERR_FAIL_MSG("Uniforms supplied for set (" + itos(i) + "):\n" + _shader_uniform_debug(us->shader_id, us->shader_set) +
                     "\nare not the same format as required by the pipeline shader. Pipeline "
                     "shader requires the following bindings:\n" +
                     _shader_uniform_debug(cl->state.pipeline_shader));
      } else {
        ERR_FAIL_MSG("Uniforms supplied for set (" + itos(i) +
                     ", which was just freed) are not the same format as required by the pipeline "
                     "shader. Pipeline shader "
                     "requires the following bindings:\n" +
                     _shader_uniform_debug(cl->state.pipeline_shader));
      }
    }
  }
#endif

  // Prepare descriptor sets if the API doesn't use pipeline barriers.
  if (!driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
    for (uint32_t i = 0; i < cl->state.set_count; i++) {
      if (cl->state.sets[i].pipeline_expected_format == 0) {
        // Nothing expected by this pipeline.
        continue;
      }

      draw_graph.add_compute_list_uniform_set_prepare_for_use(cl->state.pipeline_shader_driver_id, cl->state.sets[i].uniform_set_driver_id, i);
    }
  }

  // Bind descriptor sets.
  for (uint32_t i = 0; i < cl->state.set_count; i++) {
    if (cl->state.sets[i].pipeline_expected_format == 0) {
      continue;  // Nothing expected by this pipeline.
    }
    if (!cl->state.sets[i].bound) {
      // All good, see if this requires re-binding.
      draw_graph.add_compute_list_bind_uniform_set(cl->state.pipeline_shader_driver_id, cl->state.sets[i].uniform_set_driver_id, i);

      UniformSet* uniform_set = uniform_set_owner.get_or_null(cl->state.sets[i].uniform_set);
      _uniform_set_update_shared(uniform_set);  // 绑定时进行一次同步

      draw_graph.add_compute_list_usages(uniform_set->draw_trackers, uniform_set->draw_trackers_usage);

      cl->state.sets[i].bound = true;
    }
  }

  draw_graph.add_compute_list_dispatch(p_x_groups, p_y_groups, p_z_groups);
  cl->state.dispatch_count++;
}

void RenderingDevice::_uniform_set_update_shared(UniformSet* p_uniform_set) {
  for (UniformSet::SharedTexture shared : p_uniform_set->shared_textures_to_update) {
    Texture* texture = texture_owner.get_or_null(shared.texture);
    ERR_CONTINUE(texture == nullptr);
    _texture_update_shared_fallback(shared.texture, texture, shared.writing);
  }
}

RID lain::RenderingDevice::uniform_set_create(const Vector<Uniform>& p_uniforms, RID p_shader, uint32_t p_shader_set) {
  _THREAD_SAFE_METHOD_

  ERR_FAIL_COND_V(p_uniforms.is_empty(), RID());

  Shader* shader = shader_owner.get_or_null(p_shader);
  ERR_FAIL_NULL_V(shader, RID());

  ERR_FAIL_COND_V_MSG(p_shader_set >= (uint32_t)shader->uniform_sets.size() || shader->uniform_sets[p_shader_set].is_empty(), RID(),
                      "Desired set (" + itos(p_shader_set) + ") not used by shader.");
  // See that all sets in shader are satisfied.

  const Vector<ShaderUniform>& set = shader->uniform_sets[p_shader_set];

  uint32_t uniform_count = p_uniforms.size();
  const Uniform* uniforms = p_uniforms.ptr();  // 传入的

  uint32_t set_uniform_count = set.size();
  const ShaderUniform* set_uniforms = set.ptr();

  LocalVector<RDD::BoundUniform> driver_uniforms;
  driver_uniforms.resize(set_uniform_count);

  // Used for verification to make sure a uniform set does not use a framebuffer bound texture.
  LocalVector<UniformSet::AttachableTexture> attachable_textures;
  Vector<RDG::ResourceTracker*> draw_trackers;  // 在绑定过程中遇到的各种tracker
  Vector<RDG::ResourceUsage> draw_trackers_usage;
  HashMap<RID, RDG::ResourceUsage> untracked_usage;
  Vector<UniformSet::SharedTexture> shared_textures_to_update;  // fall back 所用的

  for (uint32_t i = 0; i < set_uniform_count; i++) {
    const ShaderUniform& set_uniform = set_uniforms[i];
    int uniform_idx = -1;
    // 找到相同绑定的uniform在 shader uniform set中的位置
    for (int j = 0; j < (int)uniform_count; j++) {
      if (uniforms[j].binding == set_uniform.binding) {
        uniform_idx = j;
      }
    }
    ERR_FAIL_COND_V_MSG(uniform_idx == -1, RID(),
                        "All the shader bindings for the given set must be covered by the uniforms provided. Binding (" + itos(set_uniform.binding) + "), set (" +
                            itos(p_shader_set) + ") was not provided.");

    const Uniform& uniform = uniforms[uniform_idx];
    // 验证 type
    if(uniform.uniform_type != set_uniform.type){
    ERR_FAIL_COND_V_MSG(uniform.uniform_type != set_uniform.type, RID(),
                        "Mismatch uniform type for binding (" + itos(set_uniform.binding) + "), set (" + itos(p_shader_set) + "). Expected '" +
                            SHADER_UNIFORM_NAMES[set_uniform.type] + "', supplied: '" + SHADER_UNIFORM_NAMES[uniform.uniform_type] + "'.");
    }

    RDD::BoundUniform& driver_uniform = driver_uniforms[i];
    driver_uniform.type = uniform.uniform_type;
    driver_uniform.binding = uniform.binding;

    switch (uniform.uniform_type) {
      case UNIFORM_TYPE_SAMPLER: {
        // Sampler[Num]
        if (uniform.get_id_count() != (uint32_t)set_uniform.length) {
          if (set_uniform.length > 1) {
            ERR_FAIL_V_MSG(RID(), "Sampler (binding: " + itos(uniform.binding) + ") is an array of (" + itos(set_uniform.length) +
                                      ") sampler elements, so it should be provided equal number of sampler IDs to satisfy it (IDs provided: " + itos(uniform.get_id_count()) +
                                      ").");
          } else {
            ERR_FAIL_V_MSG(
                RID(), "Sampler (binding: " + itos(uniform.binding) + ") should provide one ID referencing a sampler (IDs provided: " + itos(uniform.get_id_count()) + ").");
          }
        }

        for (uint32_t j = 0; j < uniform.get_id_count(); j++) {
          RDD::SamplerID* sampler_driver_id = sampler_owner.get_or_null(uniform.get_id(j));
          ERR_FAIL_NULL_V_MSG(sampler_driver_id, RID(), "Sampler (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") is not a valid sampler.");

          driver_uniform.ids.push_back(*sampler_driver_id);  // 竟然能这样擦除掉吗
        }
      } break;
      case UNIFORM_TYPE_SAMPLER_WITH_TEXTURE: {  // 每两个
        if (uniform.get_id_count() != (uint32_t)set_uniform.length * 2) {
          if (set_uniform.length > 1) {
            ERR_FAIL_V_MSG(RID(), "SamplerTexture (binding: " + itos(uniform.binding) + ") is an array of (" + itos(set_uniform.length) +
                                      ") sampler&texture elements, so it should provided twice the amount of IDs (sampler,texture pairs) to satisfy it (IDs provided: " +
                                      itos(uniform.get_id_count()) + ").");
          } else {
            ERR_FAIL_V_MSG(RID(), "SamplerTexture (binding: " + itos(uniform.binding) +
                                      ") should provide two IDs referencing a sampler and then a texture (IDs provided: " + itos(uniform.get_id_count()) + ").");
          }
        }

        for (uint32_t j = 0; j < uniform.get_id_count(); j += 2) {
          RDD::SamplerID* sampler_driver_id = sampler_owner.get_or_null(uniform.get_id(j + 0));
          ERR_FAIL_NULL_V_MSG(sampler_driver_id, RID(), "SamplerBuffer (binding: " + itos(uniform.binding) + ", index " + itos(j + 1) + ") is not a valid sampler.");

          RID texture_id = uniform.get_id(j + 1);
          Texture* texture = texture_owner.get_or_null(texture_id);
          ERR_FAIL_NULL_V_MSG(texture, RID(), "Texture (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") is not a valid texture.");
          // 需要验证texture的usage
          ERR_FAIL_COND_V_MSG(
              !(texture->usage_flags & TEXTURE_USAGE_SAMPLING_BIT), RID(),
              "Texture (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") needs the TEXTURE_USAGE_SAMPLING_BIT usage flag set in order to be used as uniform.");

          if ((texture->usage_flags & (TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | TEXTURE_USAGE_INPUT_ATTACHMENT_BIT))) {
            UniformSet::AttachableTexture attachable_texture;
            attachable_texture.bind = set_uniform.binding;
            attachable_texture.texture = texture->owner.is_valid() ? texture->owner : uniform.get_id(j + 1);
            attachable_textures.push_back(attachable_texture);
          }

          RDD::TextureID driver_id = texture->driver_id;
          RDG::ResourceTracker* tracker = texture->draw_tracker;
          if (texture->shared_fallback != nullptr && texture->shared_fallback->texture.id != 0) {
            driver_id = texture->shared_fallback->texture;
            tracker = texture->shared_fallback->texture_tracker;
            shared_textures_to_update.push_back({false, texture_id});
          }

          if (tracker != nullptr) {
            draw_trackers.push_back(tracker);
            draw_trackers_usage.push_back(RDG::RESOURCE_USAGE_TEXTURE_SAMPLE);
          } else {
            untracked_usage[texture_id] = RDG::RESOURCE_USAGE_TEXTURE_SAMPLE;
          }

          DEV_ASSERT(!texture->owner.is_valid() || texture_owner.get_or_null(texture->owner));

          driver_uniform.ids.push_back(*sampler_driver_id);
          driver_uniform.ids.push_back(driver_id);
        }
      } break;
      case UNIFORM_TYPE_TEXTURE: {
        if (uniform.get_id_count() != (uint32_t)set_uniform.length) {
          if (set_uniform.length > 1) {
            ERR_FAIL_V_MSG(RID(), "Texture (binding: " + itos(uniform.binding) + ") is an array of (" + itos(set_uniform.length) +
                                      ") textures, so it should be provided equal number of texture IDs to satisfy it (IDs provided: " + itos(uniform.get_id_count()) + ").");
          } else {
            ERR_FAIL_V_MSG(
                RID(), "Texture (binding: " + itos(uniform.binding) + ") should provide one ID referencing a texture (IDs provided: " + itos(uniform.get_id_count()) + ").");
          }
        }

        for (uint32_t j = 0; j < uniform.get_id_count(); j++) {
          RID texture_id = uniform.get_id(j);
          Texture* texture = texture_owner.get_or_null(texture_id);
          ERR_FAIL_NULL_V_MSG(texture, RID(), "Texture (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") is not a valid texture.");

          ERR_FAIL_COND_V_MSG(
              !(texture->usage_flags & TEXTURE_USAGE_SAMPLING_BIT), RID(),
              "Texture (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") needs the TEXTURE_USAGE_SAMPLING_BIT usage flag set in order to be used as uniform.");

          if ((texture->usage_flags & (TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | TEXTURE_USAGE_INPUT_ATTACHMENT_BIT))) {
            UniformSet::AttachableTexture attachable_texture;
            attachable_texture.bind = set_uniform.binding;
            attachable_texture.texture = texture->owner.is_valid() ? texture->owner : uniform.get_id(j);
            attachable_textures.push_back(attachable_texture);
          }

          RDD::TextureID driver_id = texture->driver_id;
          RDG::ResourceTracker* tracker = texture->draw_tracker;
          if (texture->shared_fallback != nullptr && texture->shared_fallback->texture.id != 0) {
            driver_id = texture->shared_fallback->texture;
            tracker = texture->shared_fallback->texture_tracker;
            shared_textures_to_update.push_back({false, texture_id});
          }

          if (tracker != nullptr) {
            draw_trackers.push_back(tracker);
            draw_trackers_usage.push_back(RDG::RESOURCE_USAGE_TEXTURE_SAMPLE);
          } else {
            untracked_usage[texture_id] = RDG::RESOURCE_USAGE_TEXTURE_SAMPLE;
          }

          DEV_ASSERT(!texture->owner.is_valid() || texture_owner.get_or_null(texture->owner));

          driver_uniform.ids.push_back(driver_id);
        }
      } break;
      case UNIFORM_TYPE_IMAGE: {  // 这种可能写
        if (uniform.get_id_count() != (uint32_t)set_uniform.length) {
          if (set_uniform.length > 1) {
            ERR_FAIL_V_MSG(RID(), "Image (binding: " + itos(uniform.binding) + ") is an array of (" + itos(set_uniform.length) +
                                      ") textures, so it should be provided equal number of texture IDs to satisfy it (IDs provided: " + itos(uniform.get_id_count()) + ").");
          } else {
            ERR_FAIL_V_MSG(RID(),
                           "Image (binding: " + itos(uniform.binding) + ") should provide one ID referencing a texture (IDs provided: " + itos(uniform.get_id_count()) + ").");
          }
        }

        for (uint32_t j = 0; j < uniform.get_id_count(); j++) {
          RID texture_id = uniform.get_id(j);
          Texture* texture = texture_owner.get_or_null(texture_id);

          ERR_FAIL_NULL_V_MSG(texture, RID(), "Image (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") is not a valid texture.");

          ERR_FAIL_COND_V_MSG(
              !(texture->usage_flags & TEXTURE_USAGE_STORAGE_BIT), RID(),
              "Image (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") needs the TEXTURE_USAGE_STORAGE_BIT usage flag set in order to be used as uniform.");

          if (texture->owner.is_null() && texture->shared_fallback != nullptr) {
            shared_textures_to_update.push_back({true, texture_id});
          }
          // 纹理必须是可变的，因为需要布局转换。
          if (_texture_make_mutable(texture, texture_id)) {  // 这种是读写，所以假定mutable
            // The texture must be mutable as a layout transition will be required.
            draw_graph.add_synchronization();
          }

          if (texture->draw_tracker != nullptr) {
            draw_trackers.push_back(texture->draw_tracker);

            if (set_uniform.writable) {
              draw_trackers_usage.push_back(RDG::RESOURCE_USAGE_STORAGE_IMAGE_READ_WRITE);
            } else {
              draw_trackers_usage.push_back(RDG::RESOURCE_USAGE_STORAGE_IMAGE_READ);
            }
          }

          DEV_ASSERT(!texture->owner.is_valid() || texture_owner.get_or_null(texture->owner));

          driver_uniform.ids.push_back(texture->driver_id);
        }
      } break;
      case UNIFORM_TYPE_TEXTURE_BUFFER: {
        if (uniform.get_id_count() != (uint32_t)set_uniform.length) {
          if (set_uniform.length > 1) {
            ERR_FAIL_V_MSG(RID(), "Buffer (binding: " + itos(uniform.binding) + ") is an array of (" + itos(set_uniform.length) +
                                      ") texture buffer elements, so it should be provided equal number of texture buffer IDs to satisfy it (IDs provided: " +
                                      itos(uniform.get_id_count()) + ").");
          } else {
            ERR_FAIL_V_MSG(RID(), "Buffer (binding: " + itos(uniform.binding) +
                                      ") should provide one ID referencing a texture buffer (IDs provided: " + itos(uniform.get_id_count()) + ").");
          }
        }

        for (uint32_t j = 0; j < uniform.get_id_count(); j++) {
          RID buffer_id = uniform.get_id(j);
          Buffer* buffer = texture_buffer_owner.get_or_null(buffer_id);
          ERR_FAIL_NULL_V_MSG(buffer, RID(), "Texture Buffer (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") is not a valid texture buffer.");

          if (set_uniform.writable && _buffer_make_mutable(buffer, buffer_id)) {
            // The buffer must be mutable if it's used for writing.
            draw_graph.add_synchronization();
          }

          if (buffer->draw_tracker != nullptr) {
            draw_trackers.push_back(buffer->draw_tracker);

            if (set_uniform.writable) {
              draw_trackers_usage.push_back(RDG::RESOURCE_USAGE_TEXTURE_BUFFER_READ_WRITE);
            } else {
              draw_trackers_usage.push_back(RDG::RESOURCE_USAGE_TEXTURE_BUFFER_READ);
            }
          } else {
            untracked_usage[buffer_id] = RDG::RESOURCE_USAGE_TEXTURE_BUFFER_READ;
          }

          driver_uniform.ids.push_back(buffer->driver_id);
        }
      } break;
      case UNIFORM_TYPE_SAMPLER_WITH_TEXTURE_BUFFER: {
        if (uniform.get_id_count() != (uint32_t)set_uniform.length * 2) {
          if (set_uniform.length > 1) {
            ERR_FAIL_V_MSG(RID(), "SamplerBuffer (binding: " + itos(uniform.binding) + ") is an array of (" + itos(set_uniform.length) +
                                      ") sampler buffer elements, so it should provided twice the amount of IDs (sampler,buffer pairs) to satisfy it (IDs provided: " +
                                      itos(uniform.get_id_count()) + ").");
          } else {
            ERR_FAIL_V_MSG(RID(), "SamplerBuffer (binding: " + itos(uniform.binding) +
                                      ") should provide two IDs referencing a sampler and then a texture buffer (IDs provided: " + itos(uniform.get_id_count()) + ").");
          }
        }

        for (uint32_t j = 0; j < uniform.get_id_count(); j += 2) {
          RDD::SamplerID* sampler_driver_id = sampler_owner.get_or_null(uniform.get_id(j + 0));
          ERR_FAIL_NULL_V_MSG(sampler_driver_id, RID(), "SamplerBuffer (binding: " + itos(uniform.binding) + ", index " + itos(j + 1) + ") is not a valid sampler.");

          RID buffer_id = uniform.get_id(j + 1);
          Buffer* buffer = texture_buffer_owner.get_or_null(buffer_id);
          ERR_FAIL_NULL_V_MSG(buffer, RID(), "SamplerBuffer (binding: " + itos(uniform.binding) + ", index " + itos(j + 1) + ") is not a valid texture buffer.");

          if (buffer->draw_tracker != nullptr) {
            draw_trackers.push_back(buffer->draw_tracker);
            draw_trackers_usage.push_back(RDG::RESOURCE_USAGE_TEXTURE_BUFFER_READ);
          } else {
            untracked_usage[buffer_id] = RDG::RESOURCE_USAGE_TEXTURE_BUFFER_READ;
          }

          driver_uniform.ids.push_back(*sampler_driver_id);
          driver_uniform.ids.push_back(buffer->driver_id);
        }
      } break;
      case UNIFORM_TYPE_IMAGE_BUFFER: {
        // @Todo.
      } break;
      case UNIFORM_TYPE_UNIFORM_BUFFER: {
        ERR_FAIL_COND_V_MSG(uniform.get_id_count() != 1, RID(),
                            "Uniform buffer supplied (binding: " + itos(uniform.binding) + ") must provide one ID (" + itos(uniform.get_id_count()) + " provided).");

        RID buffer_id = uniform.get_id(0);
        Buffer* buffer = uniform_buffer_owner.get_or_null(buffer_id);
        ERR_FAIL_NULL_V_MSG(buffer, RID(), "Uniform buffer supplied (binding: " + itos(uniform.binding) + ") is invalid.");

        ERR_FAIL_COND_V_MSG(buffer->size < (uint32_t)set_uniform.length, RID(),
                            "Uniform buffer supplied (binding: " + itos(uniform.binding) + ") size (" + itos(buffer->size) + " is smaller than size of shader uniform: (" +
                                itos(set_uniform.length) + ").");

        if (buffer->draw_tracker != nullptr) {
          draw_trackers.push_back(buffer->draw_tracker);
          draw_trackers_usage.push_back(RDG::RESOURCE_USAGE_UNIFORM_BUFFER_READ);
        } else {
          untracked_usage[buffer_id] = RDG::RESOURCE_USAGE_UNIFORM_BUFFER_READ;
        }

        driver_uniform.ids.push_back(buffer->driver_id);
      } break;
      case UNIFORM_TYPE_STORAGE_BUFFER: {
        ERR_FAIL_COND_V_MSG(uniform.get_id_count() != 1, RID(),
                            "Storage buffer supplied (binding: " + itos(uniform.binding) + ") must provide one ID (" + itos(uniform.get_id_count()) + " provided).");

        Buffer* buffer = nullptr;

        RID buffer_id = uniform.get_id(0);
        if (storage_buffer_owner.owns(buffer_id)) {
          buffer = storage_buffer_owner.get_or_null(buffer_id);
        } else if (vertex_buffer_owner.owns(buffer_id)) {
          buffer = vertex_buffer_owner.get_or_null(buffer_id);

          ERR_FAIL_COND_V_MSG(!(buffer->usage.has_flag(RDD::BUFFER_USAGE_STORAGE_BIT)), RID(),
                              "Vertex buffer supplied (binding: " + itos(uniform.binding) + ") was not created with storage flag.");
        }
        ERR_FAIL_NULL_V_MSG(buffer, RID(), "Storage buffer supplied (binding: " + itos(uniform.binding) + ") is invalid.");

        // If 0, then it's sized on link time.
        ERR_FAIL_COND_V_MSG(set_uniform.length > 0 && buffer->size != (uint32_t)set_uniform.length, RID(),
                            "Storage buffer supplied (binding: " + itos(uniform.binding) + ") size (" + itos(buffer->size) + " does not match size of shader uniform: (" +
                                itos(set_uniform.length) + ").");

        if (set_uniform.writable && _buffer_make_mutable(buffer, buffer_id)) {
          // The buffer must be mutable if it's used for writing.
          draw_graph.add_synchronization();
        }

        if (buffer->draw_tracker != nullptr) {
          draw_trackers.push_back(buffer->draw_tracker);

          if (set_uniform.writable) {
            draw_trackers_usage.push_back(RDG::RESOURCE_USAGE_STORAGE_BUFFER_READ_WRITE);
          } else {
            draw_trackers_usage.push_back(RDG::RESOURCE_USAGE_STORAGE_BUFFER_READ);
          }
        } else {
          untracked_usage[buffer_id] = RDG::RESOURCE_USAGE_STORAGE_BUFFER_READ;
        }

        driver_uniform.ids.push_back(buffer->driver_id);
      } break;
      case UNIFORM_TYPE_INPUT_ATTACHMENT: {
        ERR_FAIL_COND_V_MSG(shader->is_compute, RID(), "InputAttachment (binding: " + itos(uniform.binding) + ") supplied for compute shader (this is not allowed).");

        if (uniform.get_id_count() != (uint32_t)set_uniform.length) {
          if (set_uniform.length > 1) {
            ERR_FAIL_V_MSG(RID(), "InputAttachment (binding: " + itos(uniform.binding) + ") is an array of (" + itos(set_uniform.length) +
                                      ") textures, so it should be provided equal number of texture IDs to satisfy it (IDs provided: " + itos(uniform.get_id_count()) + ").");
          } else {
            ERR_FAIL_V_MSG(RID(), "InputAttachment (binding: " + itos(uniform.binding) +
                                      ") should provide one ID referencing a texture (IDs provided: " + itos(uniform.get_id_count()) + ").");
          }
        }

        for (uint32_t j = 0; j < uniform.get_id_count(); j++) {
          RID texture_id = uniform.get_id(j);
          Texture* texture = texture_owner.get_or_null(texture_id);

          ERR_FAIL_NULL_V_MSG(texture, RID(), "InputAttachment (binding: " + itos(uniform.binding) + ", index " + itos(j) + ") is not a valid texture.");

          ERR_FAIL_COND_V_MSG(!(texture->usage_flags & TEXTURE_USAGE_SAMPLING_BIT), RID(),
                              "InputAttachment (binding: " + itos(uniform.binding) + ", index " + itos(j) +
                                  ") needs the TEXTURE_USAGE_SAMPLING_BIT usage flag set in order to be used as uniform.");

          DEV_ASSERT(!texture->owner.is_valid() || texture_owner.get_or_null(texture->owner));

          driver_uniform.ids.push_back(texture->driver_id);
        }
      } break;
      default: {
      }
    }
  }

  RDD::UniformSetID driver_uniform_set = driver->uniform_set_create(driver_uniforms, shader->driver_id, p_shader_set);
  ERR_FAIL_COND_V(!driver_uniform_set, RID());

  UniformSet uniform_set;
  uniform_set.driver_id = driver_uniform_set;
  uniform_set.format = shader->set_formats[p_shader_set];
  uniform_set.attachable_textures = attachable_textures;
  uniform_set.draw_trackers = draw_trackers;
  uniform_set.draw_trackers_usage = draw_trackers_usage;
  uniform_set.untracked_usage = untracked_usage;
  uniform_set.shared_textures_to_update = shared_textures_to_update;
  uniform_set.shader_set = p_shader_set;
  uniform_set.shader_id = p_shader;

  RID id = uniform_set_owner.make_rid(uniform_set);
#ifdef DEV_ENABLED
  set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
  // Add dependencies.
  _add_dependency(id, p_shader);
  for (uint32_t i = 0; i < uniform_count; i++) {
    const Uniform& uniform = uniforms[i];
    int id_count = uniform.get_id_count();
    for (int j = 0; j < id_count; j++) {
      _add_dependency(id, uniform.get_id(j));
    }
  }

  return id;
}
/*Uniform */

RID RenderingDevice::uniform_buffer_create(uint32_t p_size_bytes, const Vector<uint8_t> &p_data) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(p_data.size() && (uint32_t)p_data.size() != p_size_bytes, RID());

	Buffer buffer;
	buffer.size = p_size_bytes;
	buffer.usage = (RDD::BUFFER_USAGE_TRANSFER_TO_BIT | RDD::BUFFER_USAGE_UNIFORM_BIT);
	buffer.driver_id = driver->buffer_create(buffer.size, buffer.usage, RDD::MEMORY_ALLOCATION_TYPE_GPU);
	ERR_FAIL_COND_V(!buffer.driver_id, RID());

	// Uniform buffers are assumed to be immutable unless they don't have initial data.
	if (p_data.is_empty()) {
		buffer.draw_tracker = RDG::resource_tracker_create();
		buffer.draw_tracker->buffer_driver_id = buffer.driver_id;
	}

	if (p_data.size()) {
		_buffer_update(&buffer, RID(), 0, p_data.ptr(), p_data.size());
	}

	buffer_memory += buffer.size;

	RID id = uniform_buffer_owner.make_rid(buffer);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	return id;
}
bool RenderingDevice::uniform_set_is_valid(RID p_uniform_set) {
  _THREAD_SAFE_METHOD_
  return uniform_set_owner.owns(p_uniform_set);
}

void RenderingDevice::uniform_set_set_invalidation_callback(RID p_uniform_set, InvalidationCallback p_callback, void *p_userdata) {
	UniformSet *us = uniform_set_owner.get_or_null(p_uniform_set);
	ERR_FAIL_NULL(us);
	us->invalidated_callback = p_callback;
	us->invalidated_callback_userdata = p_userdata;
}
void RenderingDevice::_texture_update_shared_fallback(RID p_texture_rid, Texture* p_texture, bool p_for_writing) {
  if (p_texture->shared_fallback == nullptr) {
    // This texture does not use any of the shared texture fallbacks.
    return;
  }

  if (p_texture->owner.is_valid()) {
    Texture* owner_texture = texture_owner.get_or_null(p_texture->owner);
    ERR_FAIL_NULL(owner_texture);
    if (p_for_writing) {
      // Only the main texture is used for writing when using the shared fallback.
      owner_texture->shared_fallback->revision++;
      // 如果是读，更新内容
    } else if (p_texture->shared_fallback->revision != owner_texture->shared_fallback->revision) {
      // Copy the contents of the main texture into the shared texture fallback slice. Update the revision.
      _texture_copy_shared(p_texture->owner, owner_texture, p_texture_rid, p_texture);
      p_texture->shared_fallback->revision = owner_texture->shared_fallback->revision;
    }
  } else if (p_for_writing) {
    // Increment the revision of the texture so shared texture fallback slices must be updated.
    p_texture->shared_fallback->revision++;
  }
}

void RenderingDevice::_texture_copy_shared(RID p_src_texture_rid, Texture* p_src_texture, RID p_dst_texture_rid, Texture* p_dst_texture) {
  // The only type of copying allowed is from the main texture to the slice texture, as slice textures are not allowed to be used for writing when using this fallback.
  DEV_ASSERT(p_src_texture != nullptr);
  DEV_ASSERT(p_dst_texture != nullptr);
  DEV_ASSERT(p_src_texture->owner.is_null());
  DEV_ASSERT(p_dst_texture->owner == p_src_texture_rid);

  bool src_made_mutable = _texture_make_mutable(p_src_texture, p_src_texture_rid);
  bool dst_made_mutable = _texture_make_mutable(p_dst_texture, p_dst_texture_rid);
  if (src_made_mutable || dst_made_mutable) {
    draw_graph.add_synchronization();
  }

  if (p_dst_texture->shared_fallback->raw_reinterpretation) {
    // If one of the textures is a main texture and they have a reinterpret buffer, we prefer using that as it's guaranteed to be big enough to hold
    // anything and it's how the shared textures that don't use slices are created.
    bool src_has_buffer = p_src_texture->shared_fallback->buffer.id != 0;
    bool dst_has_buffer = p_dst_texture->shared_fallback->buffer.id != 0;
    bool from_src = p_src_texture->owner.is_null() && src_has_buffer;
    bool from_dst = p_dst_texture->owner.is_null() && dst_has_buffer;
    if (!from_src && !from_dst) {
      // If neither texture passed the condition, we just pick whichever texture has a reinterpretation buffer.
      from_src = src_has_buffer;
      from_dst = dst_has_buffer;
    }

    // Pick the buffer and tracker to use from the right texture.
    RDD::BufferID shared_buffer;
    RDG::ResourceTracker* shared_buffer_tracker = nullptr;
    if (from_src) {
      shared_buffer = p_src_texture->shared_fallback->buffer;
      shared_buffer_tracker = p_src_texture->shared_fallback->buffer_tracker;
    } else if (from_dst) {
      shared_buffer = p_dst_texture->shared_fallback->buffer;
      shared_buffer_tracker = p_dst_texture->shared_fallback->buffer_tracker;
    } else {
      DEV_ASSERT(false && "This path should not be reachable.");
    }

    // FIXME: When using reinterpretation buffers, the only texture aspect supported is color. Depth or stencil contents won't get copied.
    RDD::BufferTextureCopyRegion get_data_region;
    RDG::RecordedBufferToTextureCopy update_copy;
    RDD::TextureCopyableLayout first_copyable_layout;
    RDD::TextureCopyableLayout copyable_layout;
    RDD::TextureSubresource texture_subresource;
    texture_subresource.aspect = RDD::TEXTURE_ASPECT_COLOR;
    texture_subresource.layer = 0;
    texture_subresource.mipmap = 0;
    driver->texture_get_copyable_layout(p_dst_texture->shared_fallback->texture, texture_subresource, &first_copyable_layout);

    // Copying each mipmap from main texture to a buffer and then to the slice texture.
    thread_local LocalVector<RDD::BufferTextureCopyRegion> get_data_vector;
    thread_local LocalVector<RDG::RecordedBufferToTextureCopy> update_vector;
    get_data_vector.clear();
    update_vector.clear();
    for (uint32_t i = 0; i < p_dst_texture->mipmaps; i++) {
      driver->texture_get_copyable_layout(p_dst_texture->shared_fallback->texture, texture_subresource, &copyable_layout);

      uint32_t mipmap = p_dst_texture->base_mipmap + i;
      get_data_region.buffer_offset = copyable_layout.offset - first_copyable_layout.offset;
      get_data_region.texture_subresources.aspect = RDD::TEXTURE_ASPECT_COLOR_BIT;
      get_data_region.texture_subresources.base_layer = p_dst_texture->base_layer;
      get_data_region.texture_subresources.mipmap = mipmap;
      get_data_region.texture_subresources.layer_count = p_dst_texture->layers;
      get_data_region.texture_region_size.x = MAX(1U, p_src_texture->width >> mipmap);
      get_data_region.texture_region_size.y = MAX(1U, p_src_texture->height >> mipmap);
      get_data_region.texture_region_size.z = MAX(1U, p_src_texture->depth >> mipmap);
      get_data_vector.push_back(get_data_region);

      update_copy.from_buffer = shared_buffer;
      update_copy.region.buffer_offset = get_data_region.buffer_offset;
      update_copy.region.texture_subresources.aspect = RDD::TEXTURE_ASPECT_COLOR_BIT;
      update_copy.region.texture_subresources.base_layer = texture_subresource.layer;
      update_copy.region.texture_subresources.mipmap = texture_subresource.mipmap;
      update_copy.region.texture_subresources.layer_count = get_data_region.texture_subresources.layer_count;
      update_copy.region.texture_region_size.x = get_data_region.texture_region_size.x;
      update_copy.region.texture_region_size.y = get_data_region.texture_region_size.y;
      update_copy.region.texture_region_size.z = get_data_region.texture_region_size.z;
      update_vector.push_back(update_copy);

      texture_subresource.mipmap++;
    }

    draw_graph.add_texture_get_data(p_src_texture->driver_id, p_src_texture->draw_tracker, shared_buffer, get_data_vector, shared_buffer_tracker);
    draw_graph.add_texture_update(p_dst_texture->shared_fallback->texture, p_dst_texture->shared_fallback->texture_tracker, update_vector, shared_buffer_tracker);
  } else {
    // Raw reinterpretation is not required. Use a regular texture copy.
    RDD::TextureCopyRegion copy_region;
    copy_region.src_subresources.aspect = p_src_texture->read_aspect_flags;
    copy_region.src_subresources.base_layer = p_dst_texture->base_layer;
    copy_region.src_subresources.layer_count = p_dst_texture->layers;
    copy_region.dst_subresources.aspect = p_dst_texture->read_aspect_flags;
    copy_region.dst_subresources.base_layer = 0;
    copy_region.dst_subresources.layer_count = copy_region.src_subresources.layer_count;

    // Copying each mipmap from main texture to to the slice texture.
    thread_local LocalVector<RDD::TextureCopyRegion> region_vector;
    region_vector.clear();
    for (uint32_t i = 0; i < p_dst_texture->mipmaps; i++) {
      uint32_t mipmap = p_dst_texture->base_mipmap + i;
      copy_region.src_subresources.mipmap = mipmap;
      copy_region.dst_subresources.mipmap = i;
      copy_region.size.x = MAX(1U, p_src_texture->width >> mipmap);
      copy_region.size.y = MAX(1U, p_src_texture->height >> mipmap);
      copy_region.size.z = MAX(1U, p_src_texture->depth >> mipmap);
      region_vector.push_back(copy_region);
    }

    draw_graph.add_texture_copy(p_src_texture->driver_id, p_src_texture->draw_tracker, p_dst_texture->shared_fallback->texture,
                                p_dst_texture->shared_fallback->texture_tracker, region_vector);
  }
}

/***********************/
/**** COMMAND GRAPH ****/
/***********************/

bool RenderingDevice::_texture_make_mutable(Texture* p_texture, RID p_texture_id) {
  if (p_texture->draw_tracker != nullptr) {
    // Texture already has a tracker.
    return false;
  } else {
    if (p_texture->owner.is_valid()) {
      // Texture has an owner.
      Texture* owner_texture = texture_owner.get_or_null(p_texture->owner);
      ERR_FAIL_NULL_V(owner_texture, false);

      if (owner_texture->draw_tracker != nullptr) {
        // Create a tracker for this dependency in particular.
        if (p_texture->slice_type == TEXTURE_SLICE_MAX) {
          // Shared texture.
          p_texture->draw_tracker = owner_texture->draw_tracker;
          p_texture->draw_tracker->ref_count++;
        } else {
          // Slice texture.
          HashMap<Rect2i, RDG::ResourceTracker*>::ConstIterator draw_tracker_iterator = owner_texture->slice_trackers.find(p_texture->slice_rect);
          RDG::ResourceTracker* draw_tracker = nullptr;
          if (draw_tracker_iterator != owner_texture->slice_trackers.end()) {
            // Reuse the tracker at the matching rectangle.
            draw_tracker = draw_tracker_iterator->value;
          } else {
            // Create a new tracker and store it on the map.
            draw_tracker = RDG::resource_tracker_create();
            draw_tracker->parent = owner_texture->draw_tracker;
            draw_tracker->texture_driver_id = p_texture->driver_id;
            draw_tracker->texture_subresources = p_texture->barrier_range();
            draw_tracker->texture_usage = p_texture->usage_flags;
            draw_tracker->slice_or_dirty_rect = p_texture->slice_rect;
            owner_texture->slice_trackers[p_texture->slice_rect] = draw_tracker;
          }

          p_texture->slice_trackers.clear();  // 是切片
          p_texture->draw_tracker = draw_tracker;
          p_texture->draw_tracker->ref_count++;
        }  // is slice

        if (p_texture_id.is_valid()) {
          _dependencies_make_mutable(p_texture_id, p_texture->draw_tracker);
        }
      } else {
        // Delegate this to the owner instead, as it'll make all its dependencies mutable.
        _texture_make_mutable(owner_texture, p_texture->owner);
      }
    } else {
      // Regular texture.
      p_texture->draw_tracker = RDG::resource_tracker_create();
      p_texture->draw_tracker->texture_driver_id = p_texture->driver_id;
      p_texture->draw_tracker->texture_subresources = p_texture->barrier_range();
      p_texture->draw_tracker->texture_usage = p_texture->usage_flags;
      p_texture->draw_tracker->ref_count = 1;

      if (p_texture_id.is_valid()) {
        if (p_texture->has_initial_data) {
          // If the texture was initialized with initial data but wasn't made mutable from the start, assume the texture sampling usage.
          p_texture->draw_tracker->usage = RDG::RESOURCE_USAGE_TEXTURE_SAMPLE;
        }

        _dependencies_make_mutable(p_texture_id, p_texture->draw_tracker);
      }
    }

    return true;
  }
}
bool RenderingDevice::_buffer_make_mutable(Buffer* p_buffer, RID p_buffer_id) {
  if (p_buffer->draw_tracker != nullptr) {
    // Buffer already has a tracker.
    return false;
  } else {
    // Create a tracker for the buffer and make all its dependencies mutable.
    p_buffer->draw_tracker = RDG::resource_tracker_create();
    p_buffer->draw_tracker->buffer_driver_id = p_buffer->driver_id;
    if (p_buffer_id.is_valid()) {
      _dependencies_make_mutable(p_buffer_id, p_buffer->draw_tracker);
    }

    return true;
  }
}

bool RenderingDevice::_dependencies_make_mutable(RID p_id, RDG::ResourceTracker* p_resource_tracker) {
  bool made_mutable = false;
  HashMap<RID, HashSet<RID>>::Iterator E = dependency_map.find(p_id);
  if (E) {
    for (RID rid : E->value) {  // 依赖该资源的资源必须mutable
      made_mutable = _dependency_make_mutable(rid, p_id, p_resource_tracker) || made_mutable;
    }
  }

  return made_mutable;  // 只要有资源改变
}
bool RenderingDevice::_dependency_make_mutable(RID p_id, RID p_resource_id, RDG::ResourceTracker* p_resource_tracker) {
  if (texture_owner.owns(p_id)) {
    Texture* texture = texture_owner.get_or_null(p_id);
    return _texture_make_mutable(texture, p_id);
  } else if (vertex_array_owner.owns(p_id)) {
    VertexArray* vertex_array = vertex_array_owner.get_or_null(p_id);
    return _vertex_array_make_mutable(vertex_array, p_resource_id, p_resource_tracker);
  } else if (index_array_owner.owns(p_id)) {
    IndexArray* index_array = index_array_owner.get_or_null(p_id);
    return _index_array_make_mutable(index_array, p_resource_tracker);
  } else if (uniform_set_owner.owns(p_id)) {
    UniformSet* uniform_set = uniform_set_owner.get_or_null(p_id);
    return _uniform_set_make_mutable(uniform_set, p_resource_id, p_resource_tracker);
  } else {
    DEV_ASSERT(false && "Unknown resource type to make mutable.");
    return false;
  }
}

bool RenderingDevice::_vertex_array_make_mutable(VertexArray* p_vertex_array, RID p_resource_id, RDG::ResourceTracker* p_resource_tracker) {
  if (!p_vertex_array->untracked_buffers.has(p_resource_id)) {
    // Vertex array thinks the buffer is already tracked or does not use it.
    return false;
  } else {
    // Vertex array is aware of the buffer but it isn't being tracked.
    p_vertex_array->draw_trackers.push_back(p_resource_tracker);
    p_vertex_array->untracked_buffers.erase(p_resource_id);
    return true;
  }
}

bool RenderingDevice::_index_array_make_mutable(IndexArray* p_index_array, RDG::ResourceTracker* p_resource_tracker) {
  if (p_index_array->draw_tracker != nullptr) {
    // Index array already has a tracker.
    return false;
  } else {
    // Index array should assign the tracker from the buffer.
    p_index_array->draw_tracker = p_resource_tracker;
    return true;
  }
}

bool RenderingDevice::_uniform_set_make_mutable(UniformSet* p_uniform_set, RID p_resource_id, RDG::ResourceTracker* p_resource_tracker) {
  HashMap<RID, RDG::ResourceUsage>::Iterator E = p_uniform_set->untracked_usage.find(p_resource_id);
  if (!E) {
    // Uniform set thinks the resource is already tracked or does not use it.
    return false;
  } else {
    // Uniform set has seen the resource but hasn't added its tracker yet.
    p_uniform_set->draw_trackers.push_back(p_resource_tracker);
    p_uniform_set->draw_trackers_usage.push_back(E->value);
    p_uniform_set->untracked_usage.remove(E);
    return true;
  }
}

void RenderingDevice::_texture_create_reinterpret_buffer(Texture* p_texture) {
  uint64_t row_pitch_step = driver->api_trait_get(RDD::API_TRAIT_TEXTURE_DATA_ROW_PITCH_STEP);
  uint64_t transfer_alignment = driver->api_trait_get(RDD::API_TRAIT_TEXTURE_TRANSFER_ALIGNMENT);
  uint32_t pixel_bytes = get_image_format_pixel_size(p_texture->format);
  uint32_t row_pitch = STEPIFY(p_texture->width * pixel_bytes, row_pitch_step);                                        // 将纹理宽字节数与 row_pitch 对齐 //@? @Bug
  uint64_t buffer_size = STEPIFY(pixel_bytes * row_pitch * p_texture->height * p_texture->depth, transfer_alignment);  // 这不是乘了两遍 pixel_bytes 么？
  p_texture->shared_fallback->buffer =
      driver->buffer_create(buffer_size, RDD::BUFFER_USAGE_TRANSFER_FROM_BIT | RDD::BUFFER_USAGE_TRANSFER_TO_BIT, RDD::MEMORY_ALLOCATION_TYPE_GPU);
  buffer_memory += driver->buffer_get_allocation_size(p_texture->shared_fallback->buffer);

  RDG::ResourceTracker* tracker = RDG::resource_tracker_create();
  tracker->buffer_driver_id = p_texture->shared_fallback->buffer;
  p_texture->shared_fallback->buffer_tracker = tracker;
}

Vector<uint8_t> RenderingDevice::_texture_get_data(Texture* tex, uint32_t p_layer, bool p_2d) {
  uint32_t width, height, depth;
  uint32_t tight_mip_size = get_image_format_required_size(tex->format, tex->width, tex->height, p_2d ? 1 : tex->depth, tex->mipmaps, &width, &height, &depth);

  Vector<uint8_t> image_data;
  image_data.resize(tight_mip_size);

  uint32_t blockw, blockh;
  get_compressed_image_format_block_dimensions(tex->format, blockw, blockh);
  uint32_t block_size = get_compressed_image_format_block_byte_size(tex->format);
  uint32_t pixel_size = get_image_format_pixel_size(tex->format);

  {
    uint8_t* w = image_data.ptrw();

    uint32_t mipmap_offset = 0;
    for (uint32_t mm_i = 0; mm_i < tex->mipmaps; mm_i++) {
      uint32_t image_total = get_image_format_required_size(tex->format, tex->width, tex->height, p_2d ? 1 : tex->depth, mm_i + 1, &width, &height, &depth);

      uint8_t* write_ptr_mipmap = w + mipmap_offset;
      tight_mip_size = image_total - mipmap_offset;

      RDD::TextureSubresource subres;
      subres.aspect = RDD::TEXTURE_ASPECT_COLOR;
      subres.layer = p_layer;
      subres.mipmap = mm_i;
      RDD::TextureCopyableLayout layout;
      driver->texture_get_copyable_layout(tex->driver_id, subres, &layout);

      uint8_t* img_mem = driver->texture_map(tex->driver_id, subres);
      ERR_FAIL_NULL_V(img_mem, Vector<uint8_t>());

      for (uint32_t z = 0; z < depth; z++) {
        uint8_t* write_ptr = write_ptr_mipmap + z * tight_mip_size / depth;
        const uint8_t* slice_read_ptr = img_mem + z * layout.depth_pitch;

        if (block_size > 1) {
          // Compressed.
          uint32_t line_width = (block_size * (width / blockw));
          for (uint32_t y = 0; y < height / blockh; y++) {
            const uint8_t* rptr = slice_read_ptr + y * layout.row_pitch;
            uint8_t* wptr = write_ptr + y * line_width;

            memcpy(wptr, rptr, line_width);
          }

        } else {
          // Uncompressed.
          for (uint32_t y = 0; y < height; y++) {
            const uint8_t* rptr = slice_read_ptr + y * layout.row_pitch;
            uint8_t* wptr = write_ptr + y * pixel_size * width;
            memcpy(wptr, rptr, (uint64_t)pixel_size * width);
          }
        }
      }

      driver->texture_unmap(tex->driver_id);

      mipmap_offset = image_total;
    }
  }

  return image_data;
}

static _ALWAYS_INLINE_ void _copy_region(uint8_t const* __restrict p_src, uint8_t* __restrict p_dst, uint32_t p_src_x, uint32_t p_src_y, uint32_t p_src_w, uint32_t p_src_h,
                                         uint32_t p_src_full_w, uint32_t p_dst_pitch, uint32_t p_unit_size) {
  uint32_t src_offset = (p_src_y * p_src_full_w + p_src_x) * p_unit_size;
  uint32_t dst_offset = 0;
  for (uint32_t y = p_src_h; y > 0; y--) {
    uint8_t const* __restrict src = p_src + src_offset;
    uint8_t* __restrict dst = p_dst + dst_offset;
    for (uint32_t x = p_src_w * p_unit_size; x > 0; x--) {
      *dst = *src;
      src++;
      dst++;
    }
    src_offset += p_src_full_w * p_unit_size;
    dst_offset += p_dst_pitch;
  }
}

Error lain::RenderingDevice::_texture_update(RID p_texture, uint32_t p_layer, const Vector<uint8_t>& p_data, bool p_use_setup_queue, bool p_validate_can_update) {
  _THREAD_SAFE_METHOD_

  ERR_FAIL_COND_V_MSG((draw_list || compute_list) && !p_use_setup_queue, ERR_INVALID_PARAMETER, "Updating textures is forbidden during creation of a draw or compute list");

  Texture* texture = texture_owner.get_or_null(p_texture);
  ERR_FAIL_NULL_V(texture, ERR_INVALID_PARAMETER);

  if (texture->owner != RID()) {
    p_texture = texture->owner;
    texture = texture_owner.get_or_null(texture->owner);
    ERR_FAIL_NULL_V(texture, ERR_BUG);  // This is a bug.
  }

  ERR_FAIL_COND_V_MSG(texture->bound, ERR_CANT_ACQUIRE_RESOURCE,
                      "Texture can't be updated while a draw list that uses it as part of a framebuffer is being created. Ensure the draw list is finalized (and that the "
                      "color/depth texture using it is not set to `RenderingDevice.FINAL_ACTION_CONTINUE`) to update this texture.");

  ERR_FAIL_COND_V_MSG(p_validate_can_update && !(texture->usage_flags & TEXTURE_USAGE_CAN_UPDATE_BIT), ERR_INVALID_PARAMETER,
                      "Texture requires the `RenderingDevice.TEXTURE_USAGE_CAN_UPDATE_BIT` to be set to be updatable.");

  ERR_FAIL_COND_V(p_layer >= texture->layers, ERR_INVALID_PARAMETER);

  uint32_t width, height;
  uint32_t tight_mip_size = get_image_format_required_size(texture->format, texture->width, texture->height, texture->depth, texture->mipmaps, &width, &height);
  uint32_t required_size = tight_mip_size;
  uint32_t required_align = get_compressed_image_format_block_byte_size(texture->format);
  if (required_align == 1) {  // 如果不是压缩，那么对齐是文素大小
    required_align = get_image_format_pixel_size(texture->format);
  }
  required_align = STEPIFY(required_align, driver->api_trait_get(RDD::API_TRAIT_TEXTURE_TRANSFER_ALIGNMENT));

  ERR_FAIL_COND_V_MSG(required_size != (uint32_t)p_data.size(), ERR_INVALID_PARAMETER,
                      "Required size for texture update (" + itos(required_size) + ") does not match data supplied size (" + itos(p_data.size()) + ").");

  uint32_t region_size = texture_upload_region_size_px;

  const uint8_t* r = p_data.ptr();

  thread_local LocalVector<RDG::RecordedBufferToTextureCopy> command_buffer_to_texture_copies_vector;  // 用于向draw graph 提交 texture update数据
  command_buffer_to_texture_copies_vector.clear();

  if (p_use_setup_queue && driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
    // When using the setup queue directly, we transition the texture to the optimal layout.
    RDD::TextureBarrier tb;
    tb.texture = texture->driver_id;
    tb.dst_access = RDD::BARRIER_ACCESS_COPY_WRITE_BIT;
    tb.prev_layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
    tb.next_layout = RDD::TEXTURE_LAYOUT_COPY_DST_OPTIMAL;
    tb.subresources.aspect = texture->barrier_aspect_flags;
    tb.subresources.mipmap_count = texture->mipmaps;
    tb.subresources.base_layer = p_layer;  // 只复制一层p_layer 为什么不一次性全复制过去呢
    tb.subresources.layer_count = 1;
    // copy命令需要等待到 bottom 才能执行
    // 这个是立即提交的
    driver->command_pipeline_barrier(frames[frame].setup_command_buffer, RDD::PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, RDD::PIPELINE_STAGE_COPY_BIT, {}, {}, tb);
  } else if (!p_use_setup_queue) {
    // Indicate the texture will get modified for the shared texture fallback.
    _texture_update_shared_fallback(p_texture, texture, true);
  }

  uint32_t mipmap_offset = 0;

  uint32_t logic_width = texture->width;
  uint32_t logic_height = texture->height;
  // 每个mipmap的纹理
  for (uint32_t mm_i = 0; mm_i < texture->mipmaps; mm_i++) {
    uint32_t depth = 0;
    uint32_t image_total = get_image_format_required_size(texture->format, texture->width, texture->height, texture->depth, mm_i + 1, &width, &height, &depth);

    const uint8_t* read_ptr_mipmap = r + mipmap_offset;
    tight_mip_size = image_total - mipmap_offset;

    for (uint32_t z = 0; z < depth; z++) {  // For 3D textures, depth may be > 0.

      const uint8_t* read_ptr = read_ptr_mipmap + (tight_mip_size / depth) * z;

      for (uint32_t y = 0; y < height; y += region_size) {
        for (uint32_t x = 0; x < width; x += region_size) {
          uint32_t region_w = MIN(region_size, width - x);  //
          uint32_t region_h = MIN(region_size, height - y);

          uint32_t region_logic_w = MIN(region_size, logic_width - x);
          uint32_t region_logic_h = MIN(region_size, logic_height - y);

          uint32_t pixel_size = get_image_format_pixel_size(texture->format);
          uint32_t block_w = 0, block_h = 0;
          get_compressed_image_format_block_dimensions(texture->format, block_w, block_h);

          uint32_t region_pitch = (region_w * pixel_size * block_w) >> get_compressed_image_format_pixel_rshift(texture->format);
          uint32_t pitch_step = driver->api_trait_get(RDD::API_TRAIT_TEXTURE_DATA_ROW_PITCH_STEP);
          region_pitch = STEPIFY(region_pitch, pitch_step);
          uint32_t to_allocate = region_pitch * region_h;
          uint32_t alloc_offset = 0, alloc_size = 0;
          StagingRequiredAction required_action;
          Error err = _staging_buffer_allocate(to_allocate, required_align, alloc_offset, alloc_size, required_action, false);
          ERR_FAIL_COND_V(err, ERR_CANT_CREATE);

          if (!p_use_setup_queue && !command_buffer_to_texture_copies_vector.is_empty() && required_action == STAGING_REQUIRED_ACTION_FLUSH_AND_STALL_ALL) {
            if (_texture_make_mutable(texture, p_texture)) {
              // The texture must be mutable to be used as a copy destination.
              draw_graph.add_synchronization();
            }
            // 没有用成staging buffer，放到graph里
            // If we're using the draw queue and the staging buffer requires flushing everything, we submit the command early and clear the current vector.
            draw_graph.add_texture_update(texture->driver_id, texture->draw_tracker, command_buffer_to_texture_copies_vector);
            command_buffer_to_texture_copies_vector.clear();
          }

          _staging_buffer_execute_required_action(required_action);

          uint8_t* write_ptr;

          {  // Map.
            uint8_t* data_ptr = driver->buffer_map(staging_buffer_blocks[staging_buffer_current].driver_id);
            ERR_FAIL_NULL_V(data_ptr, ERR_CANT_CREATE);
            write_ptr = data_ptr;
            write_ptr += alloc_offset;
          }

          ERR_FAIL_COND_V(region_w % block_w, ERR_BUG);
          ERR_FAIL_COND_V(region_h % block_h, ERR_BUG);

          if (block_w != 1 || block_h != 1) {
            // Compressed image (blocks).
            // Must copy a block region.

            uint32_t block_size = get_compressed_image_format_block_byte_size(texture->format);
            // Re-create current variables in blocky format.
            uint32_t xb = x / block_w;
            uint32_t yb = y / block_h;
            uint32_t wb = width / block_w;
            //uint32_t hb = height / block_h;
            uint32_t region_wb = region_w / block_w;
            uint32_t region_hb = region_h / block_h;
            _copy_region(read_ptr, write_ptr, xb, yb, region_wb, region_hb, wb, region_pitch, block_size);  // 开始复制到缓冲区
          } else {
            // Regular image (pixels).
            // Must copy a pixel region.
            _copy_region(read_ptr, write_ptr, x, y, region_w, region_h, width, region_pitch, pixel_size);
          }

          {  // Unmap.
            driver->buffer_unmap(staging_buffer_blocks[staging_buffer_current].driver_id);
          }

          RDD::BufferTextureCopyRegion copy_region;
          copy_region.buffer_offset = alloc_offset;
          copy_region.texture_subresources.aspect = texture->read_aspect_flags;
          copy_region.texture_subresources.mipmap = mm_i;
          copy_region.texture_subresources.base_layer = p_layer;
          copy_region.texture_subresources.layer_count = 1;
          copy_region.texture_offset = Vector3i(x, y, z);
          copy_region.texture_region_size = Vector3i(region_logic_w, region_logic_h, 1);

          if (p_use_setup_queue) {  // 如果立即做，
            driver->command_copy_buffer_to_texture(frames[frame].setup_command_buffer, staging_buffer_blocks[staging_buffer_current].driver_id, texture->driver_id,
                                                   RDD::TEXTURE_LAYOUT_COPY_DST_OPTIMAL, copy_region);
          } else {
            RDG::RecordedBufferToTextureCopy buffer_to_texture_copy;
            buffer_to_texture_copy.from_buffer = staging_buffer_blocks[staging_buffer_current].driver_id;
            buffer_to_texture_copy.region = copy_region;
            command_buffer_to_texture_copies_vector.push_back(buffer_to_texture_copy);
          }

          staging_buffer_blocks.write[staging_buffer_current].fill_amount = alloc_offset + alloc_size;
        }
      }
    }

    mipmap_offset = image_total;
    logic_width = MAX(1u, logic_width >> 1);
    logic_height = MAX(1u, logic_height >> 1);
  }

  if (p_use_setup_queue && (texture->draw_tracker == nullptr) && driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
    // If the texture does not have a tracker, it means it must be transitioned to the sampling state.
    RDD::TextureBarrier tb;
    tb.texture = texture->driver_id;
    tb.src_access = RDD::BARRIER_ACCESS_COPY_WRITE_BIT;
    tb.prev_layout = RDD::TEXTURE_LAYOUT_COPY_DST_OPTIMAL;
    tb.next_layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    tb.subresources.aspect = texture->barrier_aspect_flags;
    tb.subresources.mipmap_count = texture->mipmaps;
    tb.subresources.base_layer = p_layer;
    tb.subresources.layer_count = 1;
    driver->command_pipeline_barrier(frames[frame].setup_command_buffer, RDD::PIPELINE_STAGE_COPY_BIT, RDD::PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {}, {}, tb);
  } else if (!p_use_setup_queue && !command_buffer_to_texture_copies_vector.is_empty()) {
    if (_texture_make_mutable(texture, p_texture)) {
      // The texture must be mutable to be used as a copy destination.
      draw_graph.add_synchronization();
    }

    draw_graph.add_texture_update(texture->driver_id, texture->draw_tracker, command_buffer_to_texture_copies_vector);
  }

  return OK;
}

void lain::RenderingDevice::_texture_check_shared_fallback(Texture* p_texture) {}

void RenderingDevice::compute_list_dispatch_threads(ComputeListID p_list, uint32_t p_x_threads, uint32_t p_y_threads, uint32_t p_z_threads) {
  ERR_FAIL_COND(p_list != ID_TYPE_COMPUTE_LIST);
  ERR_FAIL_NULL(compute_list);

#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(p_x_threads == 0, "Dispatch amount of X compute threads (" + itos(p_x_threads) + ") is zero.");
  ERR_FAIL_COND_MSG(p_y_threads == 0, "Dispatch amount of Y compute threads (" + itos(p_y_threads) + ") is zero.");
  ERR_FAIL_COND_MSG(p_z_threads == 0, "Dispatch amount of Z compute threads (" + itos(p_z_threads) + ") is zero.");
#endif

  ComputeList* cl = compute_list;

#ifdef DEBUG_ENABLED

  ERR_FAIL_COND_MSG(!cl->validation.pipeline_active, "No compute pipeline was set before attempting to draw.");

  if (cl->validation.pipeline_push_constant_size > 0) {
    // Using push constants, check that they were supplied.
    ERR_FAIL_COND_MSG(!cl->validation.pipeline_push_constant_supplied,
                      "The shader in this pipeline requires a push constant to be set before "
                      "drawing, but it's not present.");
  }

#endif

  compute_list_dispatch(p_list, Math::division_round_up(p_x_threads, cl->state.local_group_size[0]), Math::division_round_up(p_y_threads, cl->state.local_group_size[1]),
                        Math::division_round_up(p_z_threads, cl->state.local_group_size[2]));
}

void RenderingDevice::compute_list_dispatch_indirect(ComputeListID p_list, RID p_buffer, uint32_t p_offset) {
  ERR_FAIL_COND(p_list != ID_TYPE_COMPUTE_LIST);
  ERR_FAIL_NULL(compute_list);

  ComputeList* cl = compute_list;
  Buffer* buffer = storage_buffer_owner.get_or_null(p_buffer);
  ERR_FAIL_NULL(buffer);

  ERR_FAIL_COND_MSG(!buffer->usage.has_flag(RDD::BUFFER_USAGE_INDIRECT_BIT), "Buffer provided was not created to do indirect dispatch.");

  ERR_FAIL_COND_MSG(p_offset + 12 > buffer->size, "Offset provided (+12) is past the end of buffer.");

#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(!cl->validation.active, "Submitted Compute Lists can no longer be modified.");
#endif

#ifdef DEBUG_ENABLED

  ERR_FAIL_COND_MSG(!cl->validation.pipeline_active, "No compute pipeline was set before attempting to draw.");

  if (cl->validation.pipeline_push_constant_size > 0) {
    // Using push constants, check that they were supplied.
    ERR_FAIL_COND_MSG(!cl->validation.pipeline_push_constant_supplied,
                      "The shader in this pipeline requires a push constant to be set before "
                      "drawing, but it's not present.");
  }

#endif

#ifdef DEBUG_ENABLED
  for (uint32_t i = 0; i < cl->state.set_count; i++) {
    if (cl->state.sets[i].pipeline_expected_format == 0) {
      // Nothing expected by this pipeline.
      continue;
    }

    if (cl->state.sets[i].pipeline_expected_format != cl->state.sets[i].uniform_set_format) {
      if (cl->state.sets[i].uniform_set_format == 0) {
        ERR_FAIL_MSG("Uniforms were never supplied for set (" + itos(i) + ") at the time of drawing, which are required by the pipeline.");
      } else if (uniform_set_owner.owns(cl->state.sets[i].uniform_set)) {
        UniformSet* us = uniform_set_owner.get_or_null(cl->state.sets[i].uniform_set);
        ERR_FAIL_MSG("Uniforms supplied for set (" + itos(i) + "):\n" + _shader_uniform_debug(us->shader_id, us->shader_set) +
                     "\nare not the same format as required by the pipeline shader. Pipeline "
                     "shader requires the following bindings:\n" +
                     _shader_uniform_debug(cl->state.pipeline_shader));
      } else {
        ERR_FAIL_MSG("Uniforms supplied for set (" + itos(i) +
                     ", which was just freed) are not the same format as required by the pipeline "
                     "shader. Pipeline shader "
                     "requires the following bindings:\n" +
                     _shader_uniform_debug(cl->state.pipeline_shader));
      }
    }
  }
#endif

  // Prepare descriptor sets if the API doesn't use pipeline barriers.
  if (!driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
    for (uint32_t i = 0; i < cl->state.set_count; i++) {
      if (cl->state.sets[i].pipeline_expected_format == 0) {
        // Nothing expected by this pipeline.
        continue;
      }

      draw_graph.add_compute_list_uniform_set_prepare_for_use(cl->state.pipeline_shader_driver_id, cl->state.sets[i].uniform_set_driver_id, i);
    }
  }

  // Bind descriptor sets.
  for (uint32_t i = 0; i < cl->state.set_count; i++) {
    if (cl->state.sets[i].pipeline_expected_format == 0) {
      continue;  // Nothing expected by this pipeline.
    }
    if (!cl->state.sets[i].bound) {
      // All good, see if this requires re-binding.
      draw_graph.add_compute_list_bind_uniform_set(cl->state.pipeline_shader_driver_id, cl->state.sets[i].uniform_set_driver_id, i);

      UniformSet* uniform_set = uniform_set_owner.get_or_null(cl->state.sets[i].uniform_set);
      _uniform_set_update_shared(uniform_set);

      draw_graph.add_compute_list_usages(uniform_set->draw_trackers, uniform_set->draw_trackers_usage);

      cl->state.sets[i].bound = true;
    }
  }

  draw_graph.add_compute_list_dispatch_indirect(buffer->driver_id, p_offset);
  cl->state.dispatch_count++;

  if (buffer->draw_tracker != nullptr) {
    draw_graph.add_compute_list_usage(buffer->draw_tracker, RDG::RESOURCE_USAGE_INDIRECT_BUFFER_READ);
  }
}

void RenderingDevice::compute_list_add_barrier(ComputeListID p_list) {
  // Must be called within a compute list, the class mutex is locked during that time

  compute_list_barrier_state = compute_list->state;
  compute_list_end();
  compute_list_begin();

  if (compute_list_barrier_state.pipeline.is_valid()) {
    compute_list_bind_compute_pipeline(p_list, compute_list_barrier_state.pipeline);
  }

  for (uint32_t i = 0; i < compute_list_barrier_state.set_count; i++) {
    if (compute_list_barrier_state.sets[i].uniform_set.is_valid()) {
      compute_list_bind_uniform_set(p_list, compute_list_barrier_state.sets[i].uniform_set, i);
    }
  }

  if (compute_list_barrier_state.push_constant_size > 0) {
    compute_list_set_push_constant(p_list, compute_list_barrier_state.push_constant_data, compute_list_barrier_state.push_constant_size);
  }
}

void RenderingDevice::compute_list_end() {
  ERR_FAIL_NULL(compute_list);

  draw_graph.add_compute_list_end();

  memdelete(compute_list);
  compute_list = nullptr;

  // Compute_list is no longer active.
  _THREAD_SAFE_UNLOCK_
}

String RenderingDevice::_shader_uniform_debug(RID p_shader, int p_set) {
  String ret;
  const Shader* shader = shader_owner.get_or_null(p_shader);
  ERR_FAIL_INDEX_V(p_set, shader->uniform_sets.size(), String());
  ERR_FAIL_NULL_V(shader, String());
  const Vector<ShaderUniform>& uniform_set = shader->uniform_sets[p_set];

  for (int j = 0; j < uniform_set.size(); j++) {
    const ShaderUniform& ui = uniform_set[j];
    if (!ret.is_empty()) {
      ret += "\n";
    }
    ret += "Set: " + itos(p_set) + " Binding: " + itos(ui.binding) + " Type: " + SHADER_UNIFORM_NAMES[ui.type] + " Writable: " + (ui.writable ? "Y" : "N") +
           " Length: " + itos(ui.length);
  }
  return ret;
}

/*******************/
/**** DRAW LIST ****/
/*******************/

Error RenderingDevice::_draw_list_allocate(const Rect2i& p_viewport, uint32_t p_subpass) {
  // Lock while draw_list is active.
  _THREAD_SAFE_LOCK_

  draw_list = memnew(DrawList);
  draw_list->viewport = p_viewport;

  return OK;
}

RenderingDevice::DrawListID RenderingDevice::draw_list_begin_for_screen(WindowSystem::WindowID p_screen, const Color& p_clear_color) {
  _THREAD_SAFE_METHOD_

  ERR_FAIL_COND_V_MSG(draw_list != nullptr, INVALID_ID, "Only one draw list can be active at the same time.");
  ERR_FAIL_COND_V_MSG(compute_list != nullptr, INVALID_ID, "Only one draw/compute list can be active at the same time.");

  RenderingContextDriver::SurfaceID surface = context->surface_get_from_window(p_screen);
  HashMap<WindowSystem::WindowID, RDD::SwapChainID>::ConstIterator sc_it = screen_swap_chains.find(p_screen);
  HashMap<WindowSystem::WindowID, RDD::FramebufferID>::ConstIterator fb_it = screen_framebuffers.find(p_screen);
  ERR_FAIL_COND_V_MSG(surface == 0, 0, "A surface was not created for the screen.");
  ERR_FAIL_COND_V_MSG(sc_it == screen_swap_chains.end(), INVALID_ID, "Screen was never prepared.");
  ERR_FAIL_COND_V_MSG(fb_it == screen_framebuffers.end(), INVALID_ID, "Framebuffer was never prepared.");
  Rect2i viewport = Rect2i(0, 0, context->surface_get_width(surface), context->surface_get_height(surface));

  _draw_list_allocate(viewport, 0);
#ifdef DEBUG_ENABLED
  draw_list_framebuffer_format = screen_get_framebuffer_format(p_screen);
#endif
  draw_list_subpass_count = 1;

  RDD::RenderPassClearValue clear_value;
  clear_value.color = p_clear_color;

  RDD::RenderPassID render_pass = driver->swap_chain_get_render_pass(sc_it->value);
  draw_graph.add_draw_list_begin(render_pass, fb_it->value, viewport, clear_value, true, false);

  _draw_list_set_viewport(viewport);
  _draw_list_set_scissor(viewport);

  return int64_t(ID_TYPE_DRAW_LIST) << ID_BASE_SHIFT;
}

Error RenderingDevice::_draw_list_setup_framebuffer(Framebuffer* p_framebuffer, ColorInitialAction p_initial_color_action, ColorFinalAction p_final_color_action,
                                                    InitialAction p_initial_depth_action, FinalAction p_final_depth_action, RDD::FramebufferID* r_framebuffer,
                                                    RDD::RenderPassID* r_render_pass, uint32_t* r_subpass_count) {
  Framebuffer::VersionKey vk;
  vk.initial_color_action = p_initial_color_action;
  vk.final_color_action = p_final_color_action;
  vk.initial_depth_action = p_initial_depth_action;
  vk.final_depth_action = p_final_depth_action;
  vk.view_count = p_framebuffer->view_count;
  // framebuffer需要renderpass，所以version里包含framebuffer, renderpass, subpass_count
  // framebuffer format(也就是subpasses)是不变的
  if (!p_framebuffer->framebuffers.has(vk)) {
    // Need to create this version.
    Framebuffer::Version version;

    version.render_pass = _render_pass_create(get_formatkey(p_framebuffer->format_id).attachment_formats, get_formatkey(p_framebuffer->format_id).passes,
                                              p_initial_color_action, p_final_color_action, p_initial_depth_action, p_final_depth_action, p_framebuffer->view_count);

    LocalVector<RDD::TextureID> attachments;
    for (int i = 0; i < p_framebuffer->texture_ids.size(); i++) {
      Texture* texture = texture_owner.get_or_null(p_framebuffer->texture_ids[i]);
      if (texture) {
        attachments.push_back(texture->driver_id);
        if (!(texture->usage_flags & TEXTURE_USAGE_VRS_ATTACHMENT_BIT)) {  // VRS attachment will be a different size.
          ERR_FAIL_COND_V(texture->width != p_framebuffer->size.x, ERR_BUG);
          ERR_FAIL_COND_V(texture->height != p_framebuffer->size.y, ERR_BUG);
        }
      }
    }

    version.framebuffer = driver->framebuffer_create(version.render_pass, attachments, p_framebuffer->size.x, p_framebuffer->size.y);
    ERR_FAIL_COND_V(!version.framebuffer, ERR_CANT_CREATE);

    version.subpass_count = get_formatkey(p_framebuffer->format_id).passes.size();

    p_framebuffer->framebuffers.insert(vk, version);
  }
  const Framebuffer::Version& version = p_framebuffer->framebuffers[vk];
  *r_framebuffer = version.framebuffer;
  *r_render_pass = version.render_pass;
  *r_subpass_count = version.subpass_count;

  return OK;
}

Error RenderingDevice::_draw_list_render_pass_begin(Framebuffer* p_framebuffer, ColorInitialAction p_initial_color_action, ColorFinalAction p_final_color_action,
                                                    InitialAction p_initial_depth_action, FinalAction p_final_depth_action, const Vector<Color>& p_clear_colors,
                                                    float p_clear_depth, uint32_t p_clear_stencil, Point2i p_viewport_offset, Point2i p_viewport_size,
                                                    RDD::FramebufferID p_framebuffer_driver_id, RDD::RenderPassID p_render_pass) {
  thread_local LocalVector<RDD::RenderPassClearValue> clear_values;
  thread_local LocalVector<RDG::ResourceTracker*> resource_trackers;
  thread_local LocalVector<RDG::ResourceUsage> resource_usages;
  bool uses_color = false;
  bool uses_depth = false;
  clear_values.clear();
  clear_values.resize(p_framebuffer->texture_ids.size());
  resource_trackers.clear();
  resource_usages.clear();
  int clear_values_count = 0;
  {
    int color_index = 0;
    for (int i = 0; i < p_framebuffer->texture_ids.size(); i++) {  // 开始对每个texture进行设置
      RDD::RenderPassClearValue clear_value;

      RID texture_rid = p_framebuffer->texture_ids[i];
      Texture* texture = texture_owner.get_or_null(texture_rid);
      if (!texture) {
        color_index++;
        continue;
      }

      // Indicate the texture will get modified for the shared texture fallback.
      _texture_update_shared_fallback(texture_rid, texture, true);

      if (texture->usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT) {
        if (color_index < p_clear_colors.size()) {
          ERR_FAIL_INDEX_V(color_index, p_clear_colors.size(), ERR_BUG);  // A bug.
          clear_value.color = p_clear_colors[color_index];
          color_index++;
        }

        resource_trackers.push_back(texture->draw_tracker);
        resource_usages.push_back(RDG::RESOURCE_USAGE_ATTACHMENT_COLOR_READ_WRITE);
        uses_color = true;
      } else if (texture->usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        clear_value.depth = p_clear_depth;
        clear_value.stencil = p_clear_stencil;
        resource_trackers.push_back(texture->draw_tracker);
        resource_usages.push_back(RDG::RESOURCE_USAGE_ATTACHMENT_DEPTH_STENCIL_READ_WRITE);
        uses_depth = true;
      }

      clear_values[clear_values_count++] = clear_value;
    }
  }

  draw_graph.add_draw_list_begin(p_render_pass, p_framebuffer_driver_id, Rect2i(p_viewport_offset, p_viewport_size), clear_values, uses_color, uses_depth);
  draw_graph.add_draw_list_usages(resource_trackers, resource_usages);

  // Mark textures as bound.
  draw_list_bound_textures.clear();

  for (int i = 0; i < p_framebuffer->texture_ids.size(); i++) {
    Texture* texture = texture_owner.get_or_null(p_framebuffer->texture_ids[i]);
    if (!texture) {
      continue;
    }
    texture->bound = true;
    draw_list_bound_textures.push_back(p_framebuffer->texture_ids[i]);
  }

  return OK;
}

void RenderingDevice::_draw_list_set_viewport(Rect2i p_rect) {
  draw_graph.add_draw_list_set_viewport(p_rect);
}

void RenderingDevice::_draw_list_set_scissor(Rect2i p_rect) {
  draw_graph.add_draw_list_set_scissor(p_rect);
}

void RenderingDevice::_draw_list_insert_clear_region(DrawList* p_draw_list, Framebuffer* p_framebuffer, Point2i p_viewport_offset, Point2i p_viewport_size, bool p_clear_color,
                                                     const Vector<Color>& p_clear_colors, bool p_clear_depth, float p_depth, uint32_t p_stencil) {
  LocalVector<RDD::AttachmentClear> clear_attachments;
  int color_index = 0;
  int texture_index = 0;
  for (int i = 0; i < p_framebuffer->texture_ids.size(); i++) {
    Texture* texture = texture_owner.get_or_null(p_framebuffer->texture_ids[i]);

    if (!texture) {
      texture_index++;
      continue;
    }

    RDD::AttachmentClear clear_at;
    if (p_clear_color && (texture->usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT)) {
      Color clear_color = p_clear_colors[texture_index++];
      clear_at.value.color = clear_color;
      clear_at.color_attachment = color_index++;
      clear_at.aspect = RDD::TEXTURE_ASPECT_COLOR_BIT;
    } else if (p_clear_depth && (texture->usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
      clear_at.value.depth = p_depth;
      clear_at.value.stencil = p_stencil;
      clear_at.color_attachment = 0;
      clear_at.aspect = RDD::TEXTURE_ASPECT_DEPTH_BIT;
      if (format_has_stencil(texture->format)) {
        clear_at.aspect.set_flag(RDD::TEXTURE_ASPECT_STENCIL_BIT);
      }
    } else {
      ERR_CONTINUE(true);
    }
    clear_attachments.push_back(clear_at);
  }

  Rect2i rect = Rect2i(p_viewport_offset, p_viewport_size);
  draw_graph.add_draw_list_clear_attachments(clear_attachments, rect);
}


Error RenderingDevice::buffer_copy(RID p_src_buffer, RID p_dst_buffer, uint32_t p_src_offset, uint32_t p_dst_offset, uint32_t p_size) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V_MSG(draw_list, ERR_INVALID_PARAMETER,
			"Copying buffers is forbidden during creation of a draw list");
	ERR_FAIL_COND_V_MSG(compute_list, ERR_INVALID_PARAMETER,
			"Copying buffers is forbidden during creation of a compute list");

	Buffer *src_buffer = _get_buffer_from_owner(p_src_buffer);
	if (!src_buffer) {
		ERR_FAIL_V_MSG(ERR_INVALID_PARAMETER, "Source buffer argument is not a valid buffer of any type.");
	}

	Buffer *dst_buffer = _get_buffer_from_owner(p_dst_buffer);
	if (!dst_buffer) {
		ERR_FAIL_V_MSG(ERR_INVALID_PARAMETER, "Destination buffer argument is not a valid buffer of any type.");
	}

	// Validate the copy's dimensions for both buffers.
	ERR_FAIL_COND_V_MSG((p_size + p_src_offset) > src_buffer->size, ERR_INVALID_PARAMETER, "Size is larger than the source buffer.");
	ERR_FAIL_COND_V_MSG((p_size + p_dst_offset) > dst_buffer->size, ERR_INVALID_PARAMETER, "Size is larger than the destination buffer.");

	// Perform the copy.
	RDD::BufferCopyRegion region;
	region.src_offset = p_src_offset;
	region.dst_offset = p_dst_offset;
	region.size = p_size;

	if (_buffer_make_mutable(dst_buffer, p_dst_buffer)) {
		// The destination buffer must be mutable to be used as a copy destination.
		draw_graph.add_synchronization();
	}

	draw_graph.add_buffer_copy(src_buffer->driver_id, src_buffer->draw_tracker, dst_buffer->driver_id, dst_buffer->draw_tracker, region);

	return OK;
}


RID RenderingDevice::texture_buffer_create(uint32_t p_size_elements, DataFormat p_format, const Vector<uint8_t> &p_data) {
	_THREAD_SAFE_METHOD_

	uint32_t element_size = get_format_vertex_size(p_format);
	ERR_FAIL_COND_V_MSG(element_size == 0, RID(), "Format requested is not supported for texture buffers");
	uint64_t size_bytes = uint64_t(element_size) * p_size_elements;

	ERR_FAIL_COND_V(p_data.size() && (uint32_t)p_data.size() != size_bytes, RID());

	Buffer texture_buffer;
	texture_buffer.size = size_bytes;
	BitField<RDD::BufferUsageBits> usage = (RDD::BUFFER_USAGE_TRANSFER_FROM_BIT | RDD::BUFFER_USAGE_TRANSFER_TO_BIT | RDD::BUFFER_USAGE_TEXEL_BIT);
	texture_buffer.driver_id = driver->buffer_create(size_bytes, usage, RDD::MEMORY_ALLOCATION_TYPE_GPU);
	ERR_FAIL_COND_V(!texture_buffer.driver_id, RID());

	// Texture buffers are assumed to be immutable unless they don't have initial data.
	if (p_data.is_empty()) {
		texture_buffer.draw_tracker = RDG::resource_tracker_create();
		texture_buffer.draw_tracker->buffer_driver_id = texture_buffer.driver_id;
	}

	bool ok = driver->buffer_set_texel_format(texture_buffer.driver_id, p_format);
	if (!ok) {
		driver->buffer_free(texture_buffer.driver_id);
		ERR_FAIL_V(RID());
	}

	if (p_data.size()) {
		_buffer_update(&texture_buffer, RID(), 0, p_data.ptr(), p_data.size());
	}

	buffer_memory += size_bytes;

	RID id = texture_buffer_owner.make_rid(texture_buffer);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	return id;
}
void RenderingDevice::framebuffer_set_invalidation_callback(RID p_framebuffer, InvalidationCallback p_callback, void *p_userdata) {
	_THREAD_SAFE_METHOD_

	Framebuffer *framebuffer = framebuffer_owner.get_or_null(p_framebuffer);
	ERR_FAIL_NULL(framebuffer);

	framebuffer->invalidated_callback = p_callback;
	framebuffer->invalidated_callback_userdata = p_userdata;
}
RID RenderingDevice::storage_buffer_create(uint32_t p_size_bytes, const Vector<uint8_t> &p_data, BitField<StorageBufferUsage> p_usage) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(p_data.size() && (uint32_t)p_data.size() != p_size_bytes, RID());

	Buffer buffer;
	buffer.size = p_size_bytes;
	buffer.usage = (RDD::BUFFER_USAGE_TRANSFER_FROM_BIT | RDD::BUFFER_USAGE_TRANSFER_TO_BIT | RDD::BUFFER_USAGE_STORAGE_BIT);
	if (p_usage.has_flag(STORAGE_BUFFER_USAGE_DISPATCH_INDIRECT)) {
		buffer.usage.set_flag(RDD::BUFFER_USAGE_INDIRECT_BIT);
	}
	buffer.driver_id = driver->buffer_create(buffer.size, buffer.usage, RDD::MEMORY_ALLOCATION_TYPE_GPU);
	ERR_FAIL_COND_V(!buffer.driver_id, RID());

	// Storage buffers are assumed to be mutable.
	buffer.draw_tracker = RDG::resource_tracker_create();
	buffer.draw_tracker->buffer_driver_id = buffer.driver_id;

	if (p_data.size()) {
		_buffer_update(&buffer, RID(), 0, p_data.ptr(), p_data.size());
	}

	buffer_memory += buffer.size;

	RID id = storage_buffer_owner.make_rid(buffer);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	return id;
}


RID RenderingDevice::index_buffer_create(uint32_t p_index_count, IndexBufferFormat p_format, const Vector<uint8_t> &p_data, bool p_use_restart_indices) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(p_index_count == 0, RID());

	IndexBuffer index_buffer;
	index_buffer.format = p_format;
	index_buffer.supports_restart_indices = p_use_restart_indices;
	index_buffer.index_count = p_index_count;
	uint32_t size_bytes = p_index_count * ((p_format == INDEX_BUFFER_FORMAT_UINT16) ? 2 : 4);
#ifdef DEBUG_ENABLED
	if (p_data.size()) {
		index_buffer.max_index = 0;
		ERR_FAIL_COND_V_MSG((uint32_t)p_data.size() != size_bytes, RID(),
				"Default index buffer initializer array size (" + itos(p_data.size()) + ") does not match format required size (" + itos(size_bytes) + ").");
		const uint8_t *r = p_data.ptr();
		if (p_format == INDEX_BUFFER_FORMAT_UINT16) {
			const uint16_t *index16 = (const uint16_t *)r;
			for (uint32_t i = 0; i < p_index_count; i++) {
				if (p_use_restart_indices && index16[i] == 0xFFFF) {
					continue; // Restart index, ignore.
				}
				index_buffer.max_index = MAX(index16[i], index_buffer.max_index);
			}
		} else {
			const uint32_t *index32 = (const uint32_t *)r;
			for (uint32_t i = 0; i < p_index_count; i++) {
				if (p_use_restart_indices && index32[i] == 0xFFFFFFFF) {
					continue; // Restart index, ignore.
				}
				index_buffer.max_index = MAX(index32[i], index_buffer.max_index);
			}
		}
	} else {
		index_buffer.max_index = 0xFFFFFFFF;
	}
#else
	index_buffer.max_index = 0xFFFFFFFF;
#endif
	index_buffer.size = size_bytes;
	index_buffer.usage = (RDD::BUFFER_USAGE_TRANSFER_FROM_BIT | RDD::BUFFER_USAGE_TRANSFER_TO_BIT | RDD::BUFFER_USAGE_INDEX_BIT);
	index_buffer.driver_id = driver->buffer_create(index_buffer.size, index_buffer.usage, RDD::MEMORY_ALLOCATION_TYPE_GPU);
	ERR_FAIL_COND_V(!index_buffer.driver_id, RID());

	// Index buffers are assumed to be immutable unless they don't have initial data.
	if (p_data.is_empty()) {
		index_buffer.draw_tracker = RDG::resource_tracker_create();
		index_buffer.draw_tracker->buffer_driver_id = index_buffer.driver_id;
	}

	if (p_data.size()) {
		_buffer_update(&index_buffer, RID(), 0, p_data.ptr(), p_data.size());
	}

	buffer_memory += index_buffer.size;

	RID id = index_buffer_owner.make_rid(index_buffer);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	return id;
}

RID RenderingDevice::index_array_create(RID p_index_buffer, uint32_t p_index_offset, uint32_t p_index_count) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(!index_buffer_owner.owns(p_index_buffer), RID());

	IndexBuffer *index_buffer = index_buffer_owner.get_or_null(p_index_buffer);

	ERR_FAIL_COND_V(p_index_count == 0, RID());
	ERR_FAIL_COND_V(p_index_offset + p_index_count > index_buffer->index_count, RID());

	IndexArray index_array;
	index_array.max_index = index_buffer->max_index;
	index_array.driver_id = index_buffer->driver_id;
	index_array.draw_tracker = index_buffer->draw_tracker;
	index_array.offset = p_index_offset;
	index_array.indices = p_index_count;
	index_array.format = index_buffer->format;
	index_array.supports_restart_indices = index_buffer->supports_restart_indices;

	RID id = index_array_owner.make_rid(index_array);
	_add_dependency(id, p_index_buffer);
	return id;
}

RenderingDevice::DrawListID RenderingDevice::draw_list_begin(RID p_framebuffer, ColorInitialAction p_initial_color_action, ColorFinalAction p_final_color_action,
                                                             InitialAction p_initial_depth_action, FinalAction p_final_depth_action, const Vector<Color>& p_clear_color_values,
                                                             float p_clear_depth, uint32_t p_clear_stencil, const Rect2& p_region) {
  _THREAD_SAFE_METHOD_

  ERR_FAIL_COND_V_MSG(draw_list != nullptr, INVALID_ID, "Only one draw list can be active at the same time.");

  Framebuffer* framebuffer = framebuffer_owner.get_or_null(p_framebuffer);
  ERR_FAIL_NULL_V(framebuffer, INVALID_ID);

  Point2i viewport_offset;
  Point2i viewport_size = framebuffer->size;

  if (p_region != Rect2() && p_region != Rect2(Vector2(), viewport_size)) {  // Check custom region.
    Rect2i viewport(viewport_offset, viewport_size);
    Rect2i regioni = p_region;
    if (!((regioni.position.x >= viewport.position.x) && (regioni.position.y >= viewport.position.y) &&
          ((regioni.position.x + regioni.size.x) <= (viewport.position.x + viewport.size.x)) &&
          ((regioni.position.y + regioni.size.y) <= (viewport.position.y + viewport.size.y)))) {
      ERR_FAIL_V_MSG(INVALID_ID, "When supplying a custom region, it must be contained within the framebuffer rectangle");
    }

    viewport_offset = regioni.position;
    viewport_size = regioni.size;
  }
  // 验证clear values数量
  if (p_initial_color_action.has_initial_action(InitialAction::INITIAL_ACTION_CLEAR)) {
    int color_count = 0;
    for (int i = 0; i < framebuffer->texture_ids.size(); i++) {
      if (p_initial_color_action.get_initial_action(i) != INITIAL_ACTION_CLEAR)
        continue;
      Texture* texture = texture_owner.get_or_null(framebuffer->texture_ids[i]);
      if (!texture || (!(texture->usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) && !(i != 0 && texture->usage_flags & TEXTURE_USAGE_VRS_ATTACHMENT_BIT))) {
        if (!texture || !texture->is_resolve_buffer) {
          color_count++;
        }
      }
    }
    ERR_FAIL_COND_V_MSG(p_clear_color_values.size() != color_count, INVALID_ID,
                        "Clear color values supplied (" + itos(p_clear_color_values.size()) + ") differ from the amount required for framebuffer color attachments (" +
                            itos(color_count) + ").");
  }

  RDD::FramebufferID fb_driver_id;
  RDD::RenderPassID render_pass;
  // 根据versionkey 创建framebuffer和renderpass
  Error err = _draw_list_setup_framebuffer(framebuffer, p_initial_color_action, p_final_color_action, p_initial_depth_action, p_final_depth_action, &fb_driver_id,
                                           &render_pass, &draw_list_subpass_count);
  ERR_FAIL_COND_V(err != OK, INVALID_ID);
  // texture update，以及 draw graph 加入draw_list_begin
  err = _draw_list_render_pass_begin(framebuffer, p_initial_color_action, p_final_color_action, p_initial_depth_action, p_final_depth_action, p_clear_color_values,
                                     p_clear_depth, p_clear_stencil, viewport_offset, viewport_size, fb_driver_id, render_pass);

  if (err != OK) {
    return INVALID_ID;
  }

  draw_list_render_pass = render_pass;
  draw_list_vkframebuffer = fb_driver_id;

  _draw_list_allocate(Rect2i(viewport_offset, viewport_size), 0);
#ifdef DEBUG_ENABLED
  draw_list_framebuffer_format = framebuffer->format_id;
#endif
  draw_list_current_subpass = 0;

  _draw_list_set_viewport(Rect2i(viewport_offset, viewport_size));
  _draw_list_set_scissor(Rect2i(viewport_offset, viewport_size));

  return int64_t(ID_TYPE_DRAW_LIST) << ID_BASE_SHIFT;
}

RenderingDevice::DrawList* RenderingDevice::_get_draw_list_ptr(DrawListID p_id) {
  if (p_id < 0) {
    return nullptr;
  }

  if (!draw_list) {
    return nullptr;
  } else if (p_id == (int64_t(ID_TYPE_DRAW_LIST) << ID_BASE_SHIFT)) {
    return draw_list;
  } else {
    return nullptr;
  }
}

void RenderingDevice::draw_list_set_blend_constants(DrawListID p_list, const Color& p_color) {
  DrawList* dl = _get_draw_list_ptr(p_list);
  ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

  draw_graph.add_draw_list_set_blend_constants(p_color);
}

void RenderingDevice::draw_list_bind_render_pipeline(DrawListID p_list, RID p_render_pipeline) {
  DrawList* dl = _get_draw_list_ptr(p_list);
  ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

  const RenderPipeline* pipeline = render_pipeline_owner.get_or_null(p_render_pipeline);
  ERR_FAIL_NULL(pipeline);
#ifdef DEBUG_ENABLED
  ERR_FAIL_COND(pipeline->validation.framebuffer_format != draw_list_framebuffer_format && pipeline->validation.render_pass != draw_list_current_subpass);
#endif

  if (p_render_pipeline == dl->state.pipeline) {
    return;  // Redundant state, return.
  }

  dl->state.pipeline = p_render_pipeline;

  draw_graph.add_draw_list_bind_pipeline(pipeline->driver_id, pipeline->stage_bits);

  if (dl->state.pipeline_shader != pipeline->shader) {
    // Shader changed, so descriptor sets may become incompatible.

    uint32_t pcount = pipeline->set_formats.size();  // Formats count in this pipeline.
    dl->state.set_count = MAX(dl->state.set_count, pcount);
    const uint32_t* pformats = pipeline->set_formats.ptr();  // Pipeline set formats.
    // 标记第一个不一致的set，如果api是API_TRAIT_SHADER_CHANGE_INVALIDATION那么所有的都无效；如果是SHADER_CHANGE_INVALIDATION_INCOMPATIBLE_SETS_PLUS_CASCADE
    // 那么只有expected_format不同的才无效。
    // 如果是SHADER_CHANGE_INVALIDATION_ALL_OR_NONE_ACCORDING_TO_LAYOUT_HASH，那么只有layout_hash不同的才无效。
    uint32_t first_invalid_set = UINT32_MAX;  // All valid by default.
    switch (driver->api_trait_get(RDD::API_TRAIT_SHADER_CHANGE_INVALIDATION)) {
      case RDD::SHADER_CHANGE_INVALIDATION_ALL_BOUND_UNIFORM_SETS: {
        first_invalid_set = 0;
      } break;
      case RDD::SHADER_CHANGE_INVALIDATION_INCOMPATIBLE_SETS_PLUS_CASCADE: {
        for (uint32_t i = 0; i < pcount; i++) {
          if (dl->state.sets[i].pipeline_expected_format != pformats[i]) {
            first_invalid_set = i;
            break;
          }
        }
      } break;
      case RDD::SHADER_CHANGE_INVALIDATION_ALL_OR_NONE_ACCORDING_TO_LAYOUT_HASH: {
        if (dl->state.pipeline_shader_layout_hash != pipeline->shader_layout_hash) {
          first_invalid_set = 0;
        }
      } break;
    }

    for (uint32_t i = 0; i < pcount; i++) {
      dl->state.sets[i].bound = dl->state.sets[i].bound && i < first_invalid_set;  // 小于的bound不变, bound在draw_list_draw中会被重新绑定(add_draw_list_bind_uniform_set)
      dl->state.sets[i].pipeline_expected_format = pformats[i];
    }

    for (uint32_t i = pcount; i < dl->state.set_count; i++) {
      // Unbind the ones above (not used) if exist.
      dl->state.sets[i].bound = false;
    }

    dl->state.set_count = pcount;  // Update set count.

    if (pipeline->push_constant_size) {
#ifdef DEBUG_ENABLED
      dl->validation.pipeline_push_constant_supplied = false;
#endif
    }

    dl->state.pipeline_shader = pipeline->shader;
    dl->state.pipeline_shader_driver_id = pipeline->shader_driver_id;
    dl->state.pipeline_shader_layout_hash = pipeline->shader_layout_hash;
  }

#ifdef DEBUG_ENABLED
  // Update render pass pipeline info.
  dl->validation.pipeline_active = true;
  dl->validation.pipeline_dynamic_state = pipeline->validation.dynamic_state;
  dl->validation.pipeline_vertex_format = pipeline->validation.vertex_format;
  dl->validation.pipeline_uses_restart_indices = pipeline->validation.uses_restart_indices;
  dl->validation.pipeline_primitive_divisor = pipeline->validation.primitive_divisor;
  dl->validation.pipeline_primitive_minimum = pipeline->validation.primitive_minimum;
  dl->validation.pipeline_push_constant_size = pipeline->push_constant_size;
#endif
}

void RenderingDevice::draw_list_bind_uniform_set(DrawListID p_list, RID p_uniform_set, uint32_t p_index) {
#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(
      p_index >= driver->limit_get(LIMIT_MAX_BOUND_UNIFORM_SETS) || p_index >= MAX_UNIFORM_SETS,
      "Attempting to bind a descriptor set (" + itos(p_index) + ") greater than what the hardware supports (" + itos(driver->limit_get(LIMIT_MAX_BOUND_UNIFORM_SETS)) + ").");
#endif
  DrawList* dl = _get_draw_list_ptr(p_list);
  ERR_FAIL_NULL(dl);

#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

  const UniformSet* uniform_set = uniform_set_owner.get_or_null(p_uniform_set);
  ERR_FAIL_NULL(uniform_set);

  if (p_index > dl->state.set_count) {
    dl->state.set_count = p_index;
  }

  dl->state.sets[p_index].uniform_set_driver_id = uniform_set->driver_id;  // Update set pointer.
  dl->state.sets[p_index].bound = false;                                   // Needs rebind.
  dl->state.sets[p_index].uniform_set_format = uniform_set->format;
  dl->state.sets[p_index].uniform_set = p_uniform_set;

#ifdef DEBUG_ENABLED
  {  // Validate that textures bound are not attached as framebuffer bindings.
    uint32_t attachable_count = uniform_set->attachable_textures.size();
    const UniformSet::AttachableTexture* attachable_ptr = uniform_set->attachable_textures.ptr();
    uint32_t bound_count = draw_list_bound_textures.size();
    const RID* bound_ptr = draw_list_bound_textures.ptr();
    for (uint32_t i = 0; i < attachable_count; i++) {
      for (uint32_t j = 0; j < bound_count; j++) {
        ERR_FAIL_COND_MSG(attachable_ptr[i].texture == bound_ptr[j], "Attempted to use the same texture in framebuffer attachment and a uniform (set: " + itos(p_index) +
                                                                         ", binding: " + itos(attachable_ptr[i].bind) + "), this is not allowed.");
      }
    }
  }
#endif
}

void RenderingDevice::draw_list_bind_vertex_array(DrawListID p_list, RID p_vertex_array) {
  DrawList* dl = _get_draw_list_ptr(p_list);
  ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

  const VertexArray* vertex_array = vertex_array_owner.get_or_null(p_vertex_array);
  ERR_FAIL_NULL(vertex_array);

  if (dl->state.vertex_array == p_vertex_array) {
    return;  // Already set.
  }

  dl->state.vertex_array = p_vertex_array;

#ifdef DEBUG_ENABLED
  dl->validation.vertex_format = vertex_array->description;
  dl->validation.vertex_max_instances_allowed = vertex_array->max_instances_allowed;
#endif
  dl->validation.vertex_array_size = vertex_array->vertex_count;

  draw_graph.add_draw_list_bind_vertex_buffers(vertex_array->buffers, vertex_array->offsets);

  for (int i = 0; i < vertex_array->draw_trackers.size(); i++) {
    draw_graph.add_draw_list_usage(vertex_array->draw_trackers[i], RDG::RESOURCE_USAGE_VERTEX_BUFFER_READ);
  }
}

void RenderingDevice::draw_list_bind_index_array(DrawListID p_list, RID p_index_array) {
  DrawList* dl = _get_draw_list_ptr(p_list);
  ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

  const IndexArray* index_array = index_array_owner.get_or_null(p_index_array);
  ERR_FAIL_NULL(index_array);

  if (dl->state.index_array == p_index_array) {
    return;  // Already set.
  }

  dl->state.index_array = p_index_array;
#ifdef DEBUG_ENABLED
  dl->validation.index_array_max_index = index_array->max_index;
#endif
  dl->validation.index_array_count = index_array->indices;

  const uint64_t offset_bytes = index_array->offset * (index_array->format == INDEX_BUFFER_FORMAT_UINT16 ? sizeof(uint16_t) : sizeof(uint32_t));
  draw_graph.add_draw_list_bind_index_buffer(index_array->driver_id, index_array->format, offset_bytes);

  if (index_array->draw_tracker != nullptr) {
    draw_graph.add_draw_list_usage(index_array->draw_tracker, RDG::RESOURCE_USAGE_INDEX_BUFFER_READ);
  }
}

void RenderingDevice::draw_list_set_line_width(DrawListID p_list, float p_width) {
  DrawList* dl = _get_draw_list_ptr(p_list);
  ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

  draw_graph.add_draw_list_set_line_width(p_width);
}

void RenderingDevice::draw_list_set_push_constant(DrawListID p_list, const void* p_data, uint32_t p_data_size) {
  DrawList* dl = _get_draw_list_ptr(p_list);
  ERR_FAIL_NULL(dl);

#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(p_data_size != dl->validation.pipeline_push_constant_size, "This render pipeline requires (" + itos(dl->validation.pipeline_push_constant_size) +
                                                                                   ") bytes of push constant data, supplied: (" + itos(p_data_size) + ")");
#endif

  draw_graph.add_draw_list_set_push_constant(dl->state.pipeline_shader_driver_id, p_data, p_data_size);

#ifdef DEBUG_ENABLED
  dl->validation.pipeline_push_constant_supplied = true;
#endif
}

void RenderingDevice::draw_list_draw(DrawListID p_list, bool p_use_indices, uint32_t p_instances, uint32_t p_procedural_vertices) {
  DrawList* dl = _get_draw_list_ptr(p_list);
  ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(!dl->validation.pipeline_active, "No render pipeline was set before attempting to draw.");
  if (dl->validation.pipeline_vertex_format != INVALID_ID) {
    // Pipeline uses vertices, validate format.
    ERR_FAIL_COND_MSG(dl->validation.vertex_format == INVALID_ID, "No vertex array was bound, and render pipeline expects vertices.");
    // Make sure format is right.
    ERR_FAIL_COND_MSG(dl->validation.pipeline_vertex_format != dl->validation.vertex_format,
                      "The vertex format used to create the pipeline does not match the vertex format bound.");
    // Make sure number of instances is valid.
    ERR_FAIL_COND_MSG(p_instances > dl->validation.vertex_max_instances_allowed, "Number of instances requested (" + itos(p_instances) +
                                                                                     " is larger than the maximum number supported by the bound vertex array (" +
                                                                                     itos(dl->validation.vertex_max_instances_allowed) + ").");
  }

  if (dl->validation.pipeline_push_constant_size > 0) {
    // Using push constants, check that they were supplied.
    ERR_FAIL_COND_MSG(!dl->validation.pipeline_push_constant_supplied,
                      "The shader in this pipeline requires a push constant to be set before "
                      "drawing, but it's not present.");
  }

#endif

#ifdef DEBUG_ENABLED  // 验证uniform set
  for (uint32_t i = 0; i < dl->state.set_count; i++) {
    if (dl->state.sets[i].pipeline_expected_format == 0) {
      // Nothing expected by this pipeline.
      continue;
    }

    if (dl->state.sets[i].pipeline_expected_format != dl->state.sets[i].uniform_set_format) {
      if (dl->state.sets[i].uniform_set_format == 0) {
        ERR_FAIL_MSG("Uniforms were never supplied for set (" + itos(i) + ") at the time of drawing, which are required by the pipeline.");
      } else if (uniform_set_owner.owns(dl->state.sets[i].uniform_set)) {
        UniformSet* us = uniform_set_owner.get_or_null(dl->state.sets[i].uniform_set);
        ERR_FAIL_MSG("Uniforms supplied for set (" + itos(i) + "):\n" + _shader_uniform_debug(us->shader_id, us->shader_set) +
                     "\nare not the same format as required by the pipeline shader. Pipeline "
                     "shader requires the following bindings:\n" +
                     _shader_uniform_debug(dl->state.pipeline_shader));
      } else {
        ERR_FAIL_MSG("Uniforms supplied for set (" + itos(i) +
                     ", which was just freed) are not the same format as required by the pipeline "
                     "shader. Pipeline shader requires the following bindings:\n" +
                     _shader_uniform_debug(dl->state.pipeline_shader));
      }
    }
  }
#endif

  // Prepare descriptor sets if the API doesn't use pipeline barriers.
  if (!driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS)) {
    for (uint32_t i = 0; i < dl->state.set_count; i++) {
      if (dl->state.sets[i].pipeline_expected_format == 0) {
        // Nothing expected by this pipeline.
        continue;
      }

      draw_graph.add_draw_list_uniform_set_prepare_for_use(dl->state.pipeline_shader_driver_id, dl->state.sets[i].uniform_set_driver_id, i);
    }
  }

  // Bind descriptor sets.
  // 在draw时才进行绑定
  for (uint32_t i = 0; i < dl->state.set_count; i++) {
    if (dl->state.sets[i].pipeline_expected_format == 0) {
      continue;  // Nothing expected by this pipeline.
    }
    if (!dl->state.sets[i].bound) {
      // All good, see if this requires re-binding.
      draw_graph.add_draw_list_bind_uniform_set(dl->state.pipeline_shader_driver_id, dl->state.sets[i].uniform_set_driver_id, i);

      UniformSet* uniform_set = uniform_set_owner.get_or_null(dl->state.sets[i].uniform_set);
      _uniform_set_update_shared(uniform_set);

      draw_graph.add_draw_list_usages(uniform_set->draw_trackers, uniform_set->draw_trackers_usage);

      dl->state.sets[i].bound = true;
    }
  }

  if (p_use_indices) {
#ifdef DEBUG_ENABLED
    ERR_FAIL_COND_MSG(p_procedural_vertices > 0, "Procedural vertices can't be used together with indices.");

    ERR_FAIL_COND_MSG(!dl->validation.index_array_count, "Draw command requested indices, but no index buffer was set.");

    ERR_FAIL_COND_MSG(dl->validation.pipeline_uses_restart_indices != dl->validation.index_buffer_uses_restart_indices,
                      "The usage of restart indices in index buffer does not match the render "
                      "primitive in the pipeline.");
#endif
    uint32_t to_draw = dl->validation.index_array_count;

#ifdef DEBUG_ENABLED
    ERR_FAIL_COND_MSG(to_draw < dl->validation.pipeline_primitive_minimum, "Too few indices (" + itos(to_draw) + ") for the render primitive set in the render pipeline (" +
                                                                               itos(dl->validation.pipeline_primitive_minimum) + ").");

    ERR_FAIL_COND_MSG((to_draw % dl->validation.pipeline_primitive_divisor) != 0, "Index amount (" + itos(to_draw) +
                                                                                      ") must be a multiple of the amount of indices required by the render primitive (" +
                                                                                      itos(dl->validation.pipeline_primitive_divisor) + ").");
#endif

    draw_graph.add_draw_list_draw_indexed(to_draw, p_instances, 0);
  } else {
    uint32_t to_draw;

    if (p_procedural_vertices > 0) {
#ifdef DEBUG_ENABLED
      ERR_FAIL_COND_MSG(dl->validation.pipeline_vertex_format != INVALID_ID, "Procedural vertices requested, but pipeline expects a vertex array.");
#endif
      to_draw = p_procedural_vertices;
    } else {
#ifdef DEBUG_ENABLED
      ERR_FAIL_COND_MSG(dl->validation.pipeline_vertex_format == INVALID_ID, "Draw command lacks indices, but pipeline format does not use vertices.");
#endif
      to_draw = dl->validation.vertex_array_size;
    }

#ifdef DEBUG_ENABLED
    ERR_FAIL_COND_MSG(to_draw < dl->validation.pipeline_primitive_minimum, "Too few vertices (" + itos(to_draw) + ") for the render primitive set in the render pipeline (" +
                                                                               itos(dl->validation.pipeline_primitive_minimum) + ").");

    ERR_FAIL_COND_MSG((to_draw % dl->validation.pipeline_primitive_divisor) != 0, "Vertex amount (" + itos(to_draw) +
                                                                                      ") must be a multiple of the amount of vertices required by the render primitive (" +
                                                                                      itos(dl->validation.pipeline_primitive_divisor) + ").");
#endif

    draw_graph.add_draw_list_draw(to_draw, p_instances);
  }

  dl->state.draw_count++;
}

void RenderingDevice::draw_list_enable_scissor(DrawListID p_list, const Rect2& p_rect) {
  DrawList* dl = _get_draw_list_ptr(p_list);

  ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif
  Rect2i rect = p_rect;
  rect.position += dl->viewport.position;

  rect = dl->viewport.intersection(rect);

  if (rect.get_area() == 0) {
    return;
  }

  _draw_list_set_scissor(rect);
}

void RenderingDevice::draw_list_disable_scissor(DrawListID p_list) {
  DrawList* dl = _get_draw_list_ptr(p_list);
  ERR_FAIL_NULL(dl);
#ifdef DEBUG_ENABLED
  ERR_FAIL_COND_MSG(!dl->validation.active, "Submitted Draw Lists can no longer be modified.");
#endif

  _draw_list_set_scissor(dl->viewport);
}

uint32_t RenderingDevice::draw_list_get_current_pass() {
  return draw_list_current_subpass;
}

RenderingDevice::DrawListID RenderingDevice::draw_list_switch_to_next_pass() {
  _THREAD_SAFE_METHOD_
  ERR_FAIL_NULL_V(draw_list, INVALID_ID);
  ERR_FAIL_COND_V(draw_list_current_subpass >= draw_list_subpass_count - 1, INVALID_FORMAT_ID);

  draw_list_current_subpass++;

  Rect2i viewport;
  _draw_list_free(&viewport);

  draw_graph.add_draw_list_next_subpass(RDD::COMMAND_BUFFER_TYPE_PRIMARY);

  _draw_list_allocate(viewport, draw_list_current_subpass);

  return int64_t(ID_TYPE_DRAW_LIST) << ID_BASE_SHIFT;
}

void RenderingDevice::_draw_list_free(Rect2i* r_last_viewport) {
  if (r_last_viewport) {
    *r_last_viewport = draw_list->viewport;
  }
  // Just end the list.
  memdelete(draw_list);
  draw_list = nullptr;

  // Draw_list is no longer active.
  _THREAD_SAFE_UNLOCK_
}

void RenderingDevice::draw_list_end() {
  _THREAD_SAFE_METHOD_

  ERR_FAIL_NULL_MSG(draw_list, "Immediate draw list is already inactive.");

  draw_graph.add_draw_list_end();

  _draw_list_free();

  for (int i = 0; i < draw_list_bound_textures.size(); i++) {
    Texture* texture = texture_owner.get_or_null(draw_list_bound_textures[i]);
    ERR_CONTINUE(!texture);  // Wtf.
    if (texture->usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT) {
      texture->bound = false;
    }
    if (texture->usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      texture->bound = false;
    }
  }

  draw_list_bound_textures.clear();
}
/* TIME */
void lain::RenderingDevice::capture_timestamp(const String& p_name) {
  	ERR_FAIL_COND_MSG(draw_list != nullptr && draw_list->state.draw_count > 0, "Capturing timestamps during draw list creation is not allowed. Offending timestamp was: " + p_name);
	ERR_FAIL_COND_MSG(compute_list != nullptr && compute_list->state.dispatch_count > 0, "Capturing timestamps during compute list creation is not allowed. Offending timestamp was: " + p_name);
	ERR_FAIL_COND_MSG(frames[frame].timestamp_count >= max_timestamp_query_elements, vformat("Tried capturing more timestamps than the configured maximum (%d). You can increase this limit in the project settings under 'Debug/Settings' called 'Max Timestamp Query Elements'.", max_timestamp_query_elements));

	draw_graph.add_capture_timestamp(frames[frame].timestamp_pool, frames[frame].timestamp_count);

	frames[frame].timestamp_names[frames[frame].timestamp_count] = p_name;
	frames[frame].timestamp_cpu_values[frames[frame].timestamp_count] = OS::GetSingleton()->GetTicksUsec();
	frames[frame].timestamp_count++;
}

uint32_t RenderingDevice::get_captured_timestamps_count() const {
	return frames[frame].timestamp_result_count;
}

uint64_t RenderingDevice::get_captured_timestamps_frame() const {
	return frames[frame].index;
}

uint64_t RenderingDevice::get_captured_timestamp_gpu_time(uint32_t p_index) const {
	ERR_FAIL_UNSIGNED_INDEX_V(p_index, frames[frame].timestamp_result_count, 0);
	return driver->timestamp_query_result_to_time(frames[frame].timestamp_result_values[p_index]);
}

uint64_t RenderingDevice::get_captured_timestamp_cpu_time(uint32_t p_index) const {
	ERR_FAIL_UNSIGNED_INDEX_V(p_index, frames[frame].timestamp_result_count, 0);
	return frames[frame].timestamp_cpu_result_values[p_index];
}

String RenderingDevice::get_captured_timestamp_name(uint32_t p_index) const {
	ERR_FAIL_UNSIGNED_INDEX_V(p_index, frames[frame].timestamp_result_count, String());
	return frames[frame].timestamp_result_names[p_index];
}

uint64_t lain::RenderingDevice::limit_get(Limit p_limit) const {
  return driver->limit_get(p_limit);
}


RID RenderingDevice::sampler_create(const SamplerState &p_state) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_INDEX_V(p_state.repeat_u, SAMPLER_REPEAT_MODE_MAX, RID());
	ERR_FAIL_INDEX_V(p_state.repeat_v, SAMPLER_REPEAT_MODE_MAX, RID());
	ERR_FAIL_INDEX_V(p_state.repeat_w, SAMPLER_REPEAT_MODE_MAX, RID());
	ERR_FAIL_INDEX_V(p_state.compare_op, COMPARE_OP_MAX, RID());
	ERR_FAIL_INDEX_V(p_state.border_color, SAMPLER_BORDER_COLOR_MAX, RID());

	RDD::SamplerID sampler = driver->sampler_create(p_state);
	ERR_FAIL_COND_V(!sampler, RID());

	RID id = sampler_owner.make_rid(sampler);
#ifdef DEV_ENABLED
	set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
	return id;
}


void lain::RenderingDevice::swap_buffers() {
  _THREAD_SAFE_METHOD_

  _end_frame();
  _execute_frame(true);

  // Advance to the next frame and begin recording again.
  frame = (frame + 1) % frames.size();
  _begin_frame();
}

void lain::RenderingDevice::submit() {
  _THREAD_SAFE_METHOD_
  ERR_FAIL_COND_MSG(is_main_instance, "Only local devices can submit and sync.");
  ERR_FAIL_COND_MSG(local_device_processing, "device already submitted, call sync to wait until done.");
  _end_frame();
  _execute_frame(false);
  local_device_processing = true;
}

void RenderingDevice::sync() {
  _THREAD_SAFE_METHOD_
  ERR_FAIL_COND_MSG(is_main_instance, "Only local devices can submit and sync.");
  ERR_FAIL_COND_MSG(!local_device_processing, "sync can only be called after a submit");
  _begin_frame();
  local_device_processing = false;
}
/// *************
/// ** VERTEX **
/// *************
RID RenderingDevice::vertex_buffer_create(uint32_t p_size_bytes, const Vector<uint8_t>& p_data, bool p_use_as_storage) {
  _THREAD_SAFE_METHOD_
  // p_data 可能被初始化成一个 Vector<uint8_t>{}，因此如果p_data.size() == 0，不要求p_data.size() == p_size_bytes
  ERR_FAIL_COND_V(p_data.size() && (uint32_t)p_data.size() != p_size_bytes, RID());

  Buffer buffer;
  buffer.size = p_size_bytes;
  buffer.usage = RDD::BUFFER_USAGE_TRANSFER_FROM_BIT | RDD::BUFFER_USAGE_TRANSFER_TO_BIT | RDD::BUFFER_USAGE_VERTEX_BIT;
  if (p_use_as_storage) {
    buffer.usage.set_flag(RDD::BUFFER_USAGE_STORAGE_BIT);
  }
  buffer.driver_id = driver->buffer_create(buffer.size, buffer.usage, RDD::MEMORY_ALLOCATION_TYPE_GPU);
  ERR_FAIL_COND_V(!buffer.driver_id, RID());

  // Vertex buffers are assumed to be immutable unless they don't have initial data or they've been marked for storage explicitly.
  if (p_data.is_empty() || p_use_as_storage) {
    buffer.draw_tracker = RDG::resource_tracker_create();
    buffer.draw_tracker->buffer_driver_id = buffer.driver_id;
  }

  if (p_data.size()) {
    _buffer_update(&buffer, RID(), 0, p_data.ptr(), p_data.size());
  }

  buffer_memory += buffer.size;

  RID id = vertex_buffer_owner.make_rid(buffer);
#ifdef DEV_ENABLED
  set_resource_name(id, "RID:" + itos(id.get_id()));
#endif
  return id;
}

RD::VertexFormatID RenderingDevice::vertex_format_create(const Vector<VertexAttribute>& p_vertex_descriptions) {
  _THREAD_SAFE_METHOD_

  VertexDescriptionKey key;
  key.vertex_formats = p_vertex_descriptions;

  VertexFormatID* idptr = vertex_format_cache.getptr(key);
  if (idptr) {
    return *idptr;
  }

  HashSet<int> used_locations;  // 防止同location错误
  for (int i = 0; i < p_vertex_descriptions.size(); i++) {
    ERR_CONTINUE(p_vertex_descriptions[i].format >= DATA_FORMAT_MAX);
    ERR_FAIL_COND_V(used_locations.has(p_vertex_descriptions[i].location), INVALID_ID);

    ERR_FAIL_COND_V_MSG(get_format_vertex_size(p_vertex_descriptions[i].format) == 0, INVALID_ID,
                        "Data format for attachment (" + itos(i) + "), '" + FORMAT_NAMES[p_vertex_descriptions[i].format] + "', is not valid for a vertex array.");

    used_locations.insert(p_vertex_descriptions[i].location);
  }

  RDD::VertexFormatID driver_id = driver->vertex_format_create(p_vertex_descriptions);
  ERR_FAIL_COND_V(!driver_id, 0);

  VertexFormatID id = (vertex_format_cache.size() | ((int64_t)ID_TYPE_VERTEX_FORMAT << ID_BASE_SHIFT));
  vertex_format_cache[key] = id;
  vertex_formats[id].vertex_formats = p_vertex_descriptions;  // 保存到p_vertex_descriptions中
  vertex_formats[id].driver_id = driver_id;
  return id;
}

RID RenderingDevice::vertex_array_create(uint32_t p_vertex_count, VertexFormatID p_vertex_format, const Vector<RID>& p_src_buffers, const Vector<uint64_t>& p_offsets) {
  _THREAD_SAFE_METHOD_

  ERR_FAIL_COND_V(!vertex_formats.has(p_vertex_format), RID());
  const VertexDescriptionInCache& vd = vertex_formats[p_vertex_format];

  ERR_FAIL_COND_V(vd.vertex_formats.size() != p_src_buffers.size(), RID());

  for (int i = 0; i < p_src_buffers.size(); i++) {
    ERR_FAIL_COND_V(!vertex_buffer_owner.owns(p_src_buffers[i]), RID());
  }

  VertexArray vertex_array;

  if (p_offsets.is_empty()) {
    vertex_array.offsets.resize_zeroed(p_src_buffers.size());
  } else {
    ERR_FAIL_COND_V(p_offsets.size() != p_src_buffers.size(), RID());
    vertex_array.offsets = p_offsets;
  }

  vertex_array.vertex_count = p_vertex_count;
  vertex_array.description = p_vertex_format;
  vertex_array.max_instances_allowed = 0xFFFFFFFF;  // By default as many as you want.
  for (int i = 0; i < p_src_buffers.size(); i++) {
    Buffer* buffer = vertex_buffer_owner.get_or_null(p_src_buffers[i]);

    // Validate with buffer.
    {
      const VertexAttribute& atf = vd.vertex_formats[i];

      uint32_t element_size = get_format_vertex_size(atf.format);
      ERR_FAIL_COND_V(element_size == 0, RID());  // Should never happens since this was prevalidated.

      if (atf.frequency == VERTEX_FREQUENCY_VERTEX) {
        // Validate size for regular drawing.
        uint64_t total_size = uint64_t(atf.stride) * (p_vertex_count - 1) + atf.offset + element_size;
        ERR_FAIL_COND_V_MSG(total_size > buffer->size, RID(), "Attachment (" + itos(i) + ") will read past the end of the buffer.");

      } else {
        // Validate size for instances drawing.
        uint64_t available = buffer->size - atf.offset;
        ERR_FAIL_COND_V_MSG(available < element_size, RID(), "Attachment (" + itos(i) + ") uses instancing, but it's just too small.");

        uint32_t instances_allowed = available / atf.stride;
        vertex_array.max_instances_allowed = MIN(instances_allowed, vertex_array.max_instances_allowed);
      }
    }

    vertex_array.buffers.push_back(buffer->driver_id);

    if (buffer->draw_tracker != nullptr) {
      vertex_array.draw_trackers.push_back(buffer->draw_tracker);
    } else {
      vertex_array.untracked_buffers.insert(p_src_buffers[i]);
    }
  }

  RID id = vertex_array_owner.make_rid(vertex_array);
  for (int i = 0; i < p_src_buffers.size(); i++) {
    _add_dependency(id, p_src_buffers[i]);
  }

  return id;
}

Error RenderingDevice::buffer_clear(RID p_buffer, uint32_t p_offset, uint32_t p_size) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V_MSG((p_size % 4) != 0, ERR_INVALID_PARAMETER,
			"Size must be a multiple of four");
	ERR_FAIL_COND_V_MSG(draw_list, ERR_INVALID_PARAMETER,
			"Updating buffers in is forbidden during creation of a draw list");
	ERR_FAIL_COND_V_MSG(compute_list, ERR_INVALID_PARAMETER,
			"Updating buffers is forbidden during creation of a compute list");

	Buffer *buffer = _get_buffer_from_owner(p_buffer);
	if (!buffer) {
		ERR_FAIL_V_MSG(ERR_INVALID_PARAMETER, "Buffer argument is not a valid buffer of any type.");
	}

	ERR_FAIL_COND_V_MSG(p_offset + p_size > buffer->size, ERR_INVALID_PARAMETER,
			"Attempted to write buffer (" + itos((p_offset + p_size) - buffer->size) + " bytes) past the end.");

	if (_buffer_make_mutable(buffer, p_buffer)) {
		// The destination buffer must be mutable to be used as a clear destination.
		draw_graph.add_synchronization();
	}

	draw_graph.add_buffer_clear(buffer->driver_id, buffer->draw_tracker, p_offset, p_size);

	return OK;
}

Vector<uint8_t> lain::RenderingDevice::buffer_get_data(RID p_buffer, uint32_t p_offset, uint32_t p_size) {
  _THREAD_SAFE_METHOD_

  Buffer* buffer = _get_buffer_from_owner(p_buffer);
  if (!buffer) {
    ERR_FAIL_V_MSG(Vector<uint8_t>(), "Buffer is either invalid or this type of buffer can't be retrieved. Only Index and Vertex buffers allow retrieving.");
  }

  // Size of buffer to retrieve.
  if (!p_size) {
    p_size = buffer->size;
  } else {
    ERR_FAIL_COND_V_MSG(p_size + p_offset > buffer->size, Vector<uint8_t>(), "Size is larger than the buffer.");
  }

  RDD::BufferID tmp_buffer = driver->buffer_create(buffer->size, RDD::BUFFER_USAGE_TRANSFER_TO_BIT, RDD::MEMORY_ALLOCATION_TYPE_CPU);
  ERR_FAIL_COND_V(!tmp_buffer, Vector<uint8_t>());

  RDD::BufferCopyRegion region;
  region.src_offset = p_offset;
  region.size = p_size;

  draw_graph.add_buffer_get_data(buffer->driver_id, buffer->draw_tracker, tmp_buffer, region);

  // Flush everything so memory can be safely mapped.
  _flush_and_stall_for_all_frames();

  uint8_t* buffer_mem = driver->buffer_map(tmp_buffer);
  ERR_FAIL_NULL_V(buffer_mem, Vector<uint8_t>());

  Vector<uint8_t> buffer_data;
  {
    buffer_data.resize(p_size);
    uint8_t* w = buffer_data.ptrw();
    memcpy(w, buffer_mem, p_size);
  }

  driver->buffer_unmap(tmp_buffer);

  driver->buffer_free(tmp_buffer);

  return buffer_data;
}

uint64_t RenderingDevice::get_driver_resource(DriverResource p_resource, RID p_rid, uint64_t p_index) {
  _THREAD_SAFE_METHOD_

  uint64_t driver_id = 0;
  switch (p_resource) {
    case DRIVER_RESOURCE_LOGICAL_DEVICE:
    case DRIVER_RESOURCE_PHYSICAL_DEVICE:
    case DRIVER_RESOURCE_TOPMOST_OBJECT:
      break;
    case DRIVER_RESOURCE_COMMAND_QUEUE:
      driver_id = main_queue.id;
      break;
    case DRIVER_RESOURCE_QUEUE_FAMILY:
      driver_id = main_queue_family.id;
      break;
    case DRIVER_RESOURCE_TEXTURE:
    case DRIVER_RESOURCE_TEXTURE_VIEW:
    case DRIVER_RESOURCE_TEXTURE_DATA_FORMAT: {
      Texture* tex = texture_owner.get_or_null(p_rid);
      ERR_FAIL_NULL_V(tex, 0);

      driver_id = tex->driver_id.id;
    } break;
    case DRIVER_RESOURCE_SAMPLER: {
      RDD::SamplerID* sampler_driver_id = sampler_owner.get_or_null(p_rid);
      ERR_FAIL_NULL_V(sampler_driver_id, 0);

      driver_id = (*sampler_driver_id).id;
    } break;
    case DRIVER_RESOURCE_UNIFORM_SET: {
      UniformSet* uniform_set = uniform_set_owner.get_or_null(p_rid);
      ERR_FAIL_NULL_V(uniform_set, 0);

      driver_id = uniform_set->driver_id.id;
    } break;
    case DRIVER_RESOURCE_BUFFER: {
      Buffer* buffer = nullptr;
      if (vertex_buffer_owner.owns(p_rid)) {
        buffer = vertex_buffer_owner.get_or_null(p_rid);
      } else if (index_buffer_owner.owns(p_rid)) {
        buffer = index_buffer_owner.get_or_null(p_rid);
      } else if (uniform_buffer_owner.owns(p_rid)) {
        buffer = uniform_buffer_owner.get_or_null(p_rid);
      } else if (texture_buffer_owner.owns(p_rid)) {
        buffer = texture_buffer_owner.get_or_null(p_rid);
      } else if (storage_buffer_owner.owns(p_rid)) {
        buffer = storage_buffer_owner.get_or_null(p_rid);
      }
      ERR_FAIL_NULL_V(buffer, 0);

      driver_id = buffer->driver_id.id;
    } break;
    case DRIVER_RESOURCE_COMPUTE_PIPELINE: {
      ComputePipeline* compute_pipeline = compute_pipeline_owner.get_or_null(p_rid);
      ERR_FAIL_NULL_V(compute_pipeline, 0);

      driver_id = compute_pipeline->driver_id.id;
    } break;
    case DRIVER_RESOURCE_RENDER_PIPELINE: {
      RenderPipeline* render_pipeline = render_pipeline_owner.get_or_null(p_rid);
      ERR_FAIL_NULL_V(render_pipeline, 0);

      driver_id = render_pipeline->driver_id.id;
    } break;
    default: {
      ERR_FAIL_V(0);
    } break;
  }

  return driver->get_resource_native_handle(p_resource, driver_id);
}

String RenderingDevice::get_device_vendor_name() const {
  switch (device.vendor) {
    case RCD::VENDOR_NVIDIA:
      return NVIDIA_VENDOR_NAME;
    case RCD::VENDOR_AMD:
      return "AMD";
    case RCD::VENDOR_INTEL:
      return "Intel";
    case RCD::VENDOR_ARM:
      return "ARM";
    case RCD::VENDOR_QUALCOMM:
      return "Qualcomm";
    case RCD::VENDOR_APPLE:
      return "Apple";
  }
  return "";
}

String lain::RenderingDevice::get_device_name() const {
  return device.name;
}
RD::DeviceType lain::RenderingDevice::get_device_type() const {
  return DeviceType(device.type);
};

String lain::RenderingDevice::get_device_api_name() const {
  return driver->get_api_name();
}

bool lain::RenderingDevice::is_composite_alpha_supported() const {
  return false;
}

