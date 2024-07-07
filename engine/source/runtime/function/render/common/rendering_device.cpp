#include "rendering_device.h"
#include "_generated/serializer/all_serializer.h"
#include "core/engine/engine.h"
using namespace lain::graphics;
using namespace lain;
RenderingDevice* RenderingDevice::singleton = nullptr;
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

RID RenderingDevice::texture_create(const TextureFormat& p_format, const TextureView& p_view,
                                    const Vector<Vector<uint8_t>>& p_data) {
  _THREAD_SAFE_METHOD_

  // Some adjustments will happen.
  TextureFormat format = p_format;

  if (format.shareable_formats.size()) {
    ERR_FAIL_COND_V_MSG(format.shareable_formats.find(format.format) == -1, RID(),
                        "If supplied a list of shareable formats, the current format must be present in the list");
    ERR_FAIL_COND_V_MSG(p_view.format_override != DATA_FORMAT_MAX && format.shareable_formats.find(p_view.format_override) == -1,
                        RID(),
                        "If supplied a list of shareable formats, the current view format override must be present in the list");
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

  if (format.texture_type == TEXTURE_TYPE_1D_ARRAY || format.texture_type == TEXTURE_TYPE_2D_ARRAY ||
      format.texture_type == TEXTURE_TYPE_CUBE_ARRAY || format.texture_type == TEXTURE_TYPE_CUBE) {
    ERR_FAIL_COND_V_MSG(format.array_layers < 1, RID(),
                        "Amount of layers must be equal or greater than 1 for arrays and cubemaps.");
    ERR_FAIL_COND_V_MSG((format.texture_type == TEXTURE_TYPE_CUBE_ARRAY || format.texture_type == TEXTURE_TYPE_CUBE) &&
                            (format.array_layers % 6) != 0,
                        RID(), "Cubemap and cubemap array textures must provide a layer number that is multiple of 6");
  } else {
    format.array_layers = 1;
  }

  ERR_FAIL_INDEX_V(format.samples, TEXTURE_SAMPLES_MAX, RID());

  format.height = format.texture_type != TEXTURE_TYPE_1D && format.texture_type != TEXTURE_TYPE_1D_ARRAY ? format.height : 1;
  format.depth = format.texture_type == TEXTURE_TYPE_3D ? format.depth : 1;

  uint32_t required_mipmaps = get_image_required_mipmaps(format.width, format.height, format.depth);

  ERR_FAIL_COND_V_MSG(required_mipmaps < format.mipmaps, RID(),
                      "Too many mipmaps requested for texture format and dimensions (" + itos(format.mipmaps) +
                          "), maximum allowed: (" + itos(required_mipmaps) + ").");

  uint32_t forced_usage_bits = 0;
  if (p_data.size()) {
    ERR_FAIL_COND_V_MSG(p_data.size() != (int)format.array_layers, RID(),
                        "Default supplied data for image format is of invalid length (" + itos(p_data.size()) + "), should be (" +
                            itos(format.array_layers) + ").");

    for (uint32_t i = 0; i < format.array_layers; i++) {
      uint32_t required_size =
          get_image_format_required_size(format.format, format.width, format.height, format.depth, format.mipmaps);
      ERR_FAIL_COND_V_MSG((uint32_t)p_data[i].size() != required_size, RID(),
                          "Data for slice index " + itos(i) + " (mapped to layer " + itos(i) + ") differs in size (supplied: " +
                              itos(p_data[i].size()) + ") than what is required by the format (" + itos(required_size) + ").");
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
    auto valid = [&](RDD::TextureUsageBits usage) {
      if (format.usage_bits & usage && !supported_usage.has_flag(usage)) {
        ERR_FAIL_MSG("Format " + format_text + " does not support usage as " + Serializer::write(usage).string_value());
      }
    };
    valid(TEXTURE_USAGE_SAMPLING_BIT);
    valid(TEXTURE_USAGE_COLOR_ATTACHMENT_BIT);
    valid(TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    valid(TEXTURE_USAGE_STORAGE_BIT);
    valid(TEXTURE_USAGE_STORAGE_ATOMIC_BIT);
    valid(TEXTURE_USAGE_VRS_ATTACHMENT_BIT);
  }

  // Transfer and validate view info.

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
  texture.driver_id = driver->texture_create(format, tv);
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
  bool texture_mutable_by_default =
      texture.usage_flags & (TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                             TEXTURE_USAGE_STORAGE_BIT | TEXTURE_USAGE_STORAGE_ATOMIC_BIT | TEXTURE_USAGE_VRS_ATTACHMENT_BIT);
  if (p_data.is_empty() || texture_mutable_by_default) {
    //_texture_make_mutable(&texture, RID()); // w: for tracker
  }

  texture_memory += driver->texture_get_allocation_size(texture.driver_id);

  RID id = texture_owner.make_rid(texture);
  driver->set_object_name(RDD::OBJECT_TYPE_TEXTURE, texture.driver_id, itos(id.get_id()));

  if (p_data.size()) {
    for (uint32_t i = 0; i < p_format.array_layers; i++) {
      //_texture_update(id, i, p_data[i], true, false);
    }

    // if (texture.draw_tracker != nullptr) {
    //	// Draw tracker can assume the texture will be in transfer destination.
    //	texture.draw_tracker->usage = RDG::RESOURCE_USAGE_TRANSFER_TO;
    // }
  }

  return id;
}
/// --- screen ---
/// --- swap chain ---
Error RenderingDevice::screen_create(WindowSystem::WindowID p_screen) {
  _THREAD_SAFE_METHOD_

  RenderingContextDriver::SurfaceID surface = context->surface_get_from_window(p_screen);
  ERR_FAIL_COND_V_MSG(surface == 0, ERR_CANT_CREATE, "A surface was not created for the screen.");

  HashMap<WindowSystem::WindowID, RDD::SwapChainID>::ConstIterator it = screen_swap_chains.find(p_screen);
  ERR_FAIL_COND_V_MSG(it != screen_swap_chains.end(), ERR_CANT_CREATE, "A swap chain was already created for the screen.");

  RDD::SwapChainID swap_chain = driver->swap_chain_create(surface);
  ERR_FAIL_COND_V_MSG(swap_chain.id == 0, ERR_CANT_CREATE, "Unable to create swap chain.");

  Error err = driver->swap_chain_resize(main_queue, swap_chain, _get_swap_chain_desired_count());
  ERR_FAIL_COND_V_MSG(err != OK, ERR_CANT_CREATE, "Unable to resize the new swap chain.");

  screen_swap_chains[p_screen] = swap_chain;

  return OK;
}

uint32_t RenderingDevice::_get_swap_chain_desired_count() const {
  return MAX(3U, uint32_t(GLOBAL_GET("rendering/rendering_device/vsync/swapchain_image_count")));  // 这个也要调整吗
      // 如果这个不调可以做成const expr，许多东西（？）可以被优化
}

Error RenderingDevice::initialize(RenderingContextDriver* p_context, WindowSystem::WindowID p_main_window) {
  using RCD = RenderingContextDriver;
  Error err = OK;

  RCD::SurfaceID main_surface = 0;
  const bool main_instance = (singleton == this) && (p_main_window != WindowSystem::INVALID_WINDOW_ID);
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
  for (uint32_t i = 0; i < device_count; i++) {
    RenderingContextDriver::Device device_option = context->device_get(i);
    String name = device_option.name;
    String vendor = _get_device_vendor_name(device_option);
    String type = _get_device_type_name(device_option);
    bool present_supported = main_surface != 0 ? context->device_supports_present(i, main_surface) : false;
    print_verbose("  #" + itos(i) + ": " + vendor + " " + name + " - " + (present_supported ? "Supported" : "Unsupported") +
                  ", " + type);
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
  ERR_FAIL_COND_V_MSG((device_index < 0) || (device_index >= int32_t(device_count)), ERR_CANT_CREATE,
                      "None of the devices supports both graphics and present queues.");
  uint32_t frame_count = 1;
  if (main_surface != 0) {
    frame_count = MAX(3U, uint32_t(GLOBAL_GET("rendering/rendering_device/vsync/frame_queue_size")));
  }

  device = context->device_get(device_index);

  err = driver->initialize(device_index, frame_count);
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

  return err;
}
