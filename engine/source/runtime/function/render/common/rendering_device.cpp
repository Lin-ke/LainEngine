#include "rendering_device.h"
#include "_generated/serializer/all_serializer.h"
#include "core/engine/engine.h"
using namespace lain::graphics;
using namespace lain;
RenderingDevice* RenderingDevice::singleton = nullptr;
RenderingDevice::ShaderCompileToSPIRVFunction RenderingDevice::compile_to_spirv_function = nullptr;
RenderingDevice::ShaderCacheFunction RenderingDevice::cache_function = nullptr;
RenderingDevice::ShaderSPIRVGetCacheKeyFunction RenderingDevice::get_spirv_cache_key_function = nullptr;
void RenderingDevice::shader_set_compile_to_spirv_function(ShaderCompileToSPIRVFunction p_function) {
	compile_to_spirv_function = p_function;
}

void RenderingDevice::shader_set_spirv_cache_function(ShaderCacheFunction p_function) {
	cache_function = p_function;
}

void RenderingDevice::shader_set_get_cache_key_function(ShaderSPIRVGetCacheKeyFunction p_function) {
	get_spirv_cache_key_function = p_function;
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
///
/// --- screen ---
///
/// frame operations
void RenderingDevice::_flush_and_stall_for_all_frames() {
	// _stall_for_previous_frames();
	// _end_frame();
	// _execute_frame(false);
	// _begin_frame();
}


/// --- swap chain ---
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
		if (frames[frame].swap_chains_to_present[to_present_index] == it->value) { // 好到这个交换链
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
	return const_cast<RenderingDevice *>(this)->framebuffer_format_create(screen_attachment);
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

/// framebuffer
RenderingDevice::FramebufferFormatID RenderingDevice::framebuffer_format_create(const Vector<AttachmentFormat> &p_format, uint32_t p_view_count) {
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

RenderingDevice::FramebufferFormatID RenderingDevice::framebuffer_format_create_multipass(const Vector<AttachmentFormat> &p_attachments, const Vector<FramebufferPass> &p_passes, uint32_t p_view_count) {
	_THREAD_SAFE_METHOD_

	FramebufferFormatKey key;
	key.attachment_formats = p_attachments;
	key.passes = p_passes;
	key.view_count = p_view_count;

	const RBMap<FramebufferFormatKey, FramebufferFormatID>::Element *E = framebuffer_format_cache.find(key);
	if (E) {
		// Exists, return.
		return E->get();
	}

	Vector<TextureSamples> samples;
	RDD::RenderPassID render_pass = _render_pass_create(p_attachments, p_passes, {}, {}, INITIAL_ACTION_CLEAR, FINAL_ACTION_STORE, p_view_count, &samples); // Actions don't matter for this use case.

	if (!render_pass) { // Was likely invalid.
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

	const RBMap<FramebufferFormatKey, FramebufferFormatID>::Element *E = framebuffer_format_cache.find(key);
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

RID RenderingDevice::framebuffer_create_empty(const Size2i &p_size, TextureSamples p_samples, FramebufferFormatID p_format_check) {
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
  if(p_action.load_attach & 1 << index) {
    return RDD::ATTACHMENT_LOAD_OP_LOAD;
  }
  if(p_action.clear_attach & 1 << index) {
    return RDD::ATTACHMENT_LOAD_OP_CLEAR;
  }
  if(p_action.discard_attach & 1 << index) {
    return RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
  }
  return RDD::ATTACHMENT_LOAD_OP_CLEAR; // default
}
L_INLINE static RDD::AttachmentStoreOp color_action_to_store_op(RenderingDevice::ColorFinalAction p_action, int index) {
  if(p_action.store_attach & 1 << index) {
    return RDD::ATTACHMENT_STORE_OP_STORE;
  }
  if(p_action.discard_attach & 1 << index) {
    return RDD::ATTACHMENT_STORE_OP_DONT_CARE;
  }
  return RDD::ATTACHMENT_STORE_OP_STORE;
}

// 转移到RDD
RDD::RenderPassID RenderingDevice::_render_pass_create(const Vector<AttachmentFormat> &p_attachments, const Vector<FramebufferPass> &p_passes, ColorInitialAction p_initial_action, ColorFinalAction p_final_action, InitialAction p_initial_depth_action, FinalAction p_final_depth_action, uint32_t p_view_count, Vector<TextureSamples> *r_samples) {
	// NOTE:
	// Before the refactor to RenderingDevice-RenderingDeviceDriver, there was commented out code to
	// specify dependencies to external subpasses. Since it had been unused for a long timel it wasn't ported
	// to the new architecture.

	LocalVector<int32_t> attachment_last_pass;
	attachment_last_pass.resize(p_attachments.size());

	if (p_view_count > 1) { // 多视图
		const RDD::MultiviewCapabilities &capabilities = driver->get_multiview_capabilities();

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
		ERR_FAIL_COND_V_MSG(!(p_attachments[i].usage_flags & (TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | TEXTURE_USAGE_INPUT_ATTACHMENT_BIT | TEXTURE_USAGE_VRS_ATTACHMENT_BIT)),
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
	subpasses.resize(p_passes.size()); // 将RD的framebuffer pass转换为RDD的subpass
	LocalVector<RDD::SubpassDependency> subpass_dependencies;
  // 写subpass
	for (int i = 0; i < p_passes.size(); i++) {
		const FramebufferPass *pass = &p_passes[i];
		RDD::Subpass &subpass = subpasses[i];

		TextureSamples texture_samples = TEXTURE_SAMPLES_1;
		bool is_multisample_first = true;

		for (int j = 0; j < pass->color_attachments.size(); j++) {
			int32_t attachment = pass->color_attachments[j];
			RDD::AttachmentReference reference;
			if (attachment == ATTACHMENT_UNUSED) {
				reference.attachment = RDD::AttachmentReference::UNUSED;
				reference.layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
			} else {
				ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), color attachment (" + itos(j) + ").");
        // attachment必须有color attachment bit
				ERR_FAIL_COND_V_MSG(!(p_attachments[attachment].usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT), RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it's marked as depth, but it's not usable as color attachment.");
				ERR_FAIL_COND_V_MSG(attachment_last_pass[attachment] == i, RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it already was used for something else before in this pass.");

				if (is_multisample_first) {
					texture_samples = p_attachments[attachment].samples;
					is_multisample_first = false;
				} else {  // attachment需要相同的samples
					ERR_FAIL_COND_V_MSG(texture_samples != p_attachments[attachment].samples, RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), if an attachment is marked as multisample, all of them should be multisample and use the same number of samples.");
				}
				reference.attachment = attachment_remap[attachment];
				reference.layout = RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachment_last_pass[attachment] = i;
			}
			reference.aspect = RDD::TEXTURE_ASPECT_COLOR_BIT;
			subpass.color_references.push_back(reference);
		} // color_attachments, 并做好color_reference

		for (int j = 0; j < pass->input_attachments.size(); j++) {
			int32_t attachment = pass->input_attachments[j];
			RDD::AttachmentReference reference;
			if (attachment == ATTACHMENT_UNUSED) {
				reference.attachment = RDD::AttachmentReference::UNUSED;
				reference.layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
			} else {
				ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), input attachment (" + itos(j) + ").");
				ERR_FAIL_COND_V_MSG(!(p_attachments[attachment].usage_flags & TEXTURE_USAGE_INPUT_ATTACHMENT_BIT), RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it isn't marked as an input texture.");
				ERR_FAIL_COND_V_MSG(attachment_last_pass[attachment] == i, RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it already was used for something else before in this pass.");
				reference.attachment = attachment_remap[attachment];
				reference.layout = RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // input attachments
				attachment_last_pass[attachment] = i;
			}
			reference.aspect = RDD::TEXTURE_ASPECT_COLOR_BIT;
			subpass.input_references.push_back(reference);
		}
    // 所有color_attachments都要有resolve_attachments
		if (pass->resolve_attachments.size() > 0) {
			ERR_FAIL_COND_V_MSG(pass->resolve_attachments.size() != pass->color_attachments.size(), RDD::RenderPassID(), "The amount of resolve attachments (" + itos(pass->resolve_attachments.size()) + ") must resolve attachments (" + itos(pass->color_attachments.size()) + ").");
			ERR_FAIL_COND_V_MSG(texture_samples == TEXTURE_SAMPLES_1, RDD::RenderPassID(), "Resolve attachments specified, but color attachments are not multisample.");
		}
		for (int j = 0; j < pass->resolve_attachments.size(); j++) {
			int32_t attachment = pass->resolve_attachments[j];
			RDD::AttachmentReference reference;
			if (attachment == ATTACHMENT_UNUSED) {
				reference.attachment = RDD::AttachmentReference::UNUSED;
				reference.layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
			} else {
				ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), resolve attachment (" + itos(j) + ").");
				ERR_FAIL_COND_V_MSG(pass->color_attachments[j] == ATTACHMENT_UNUSED, RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), resolve attachment (" + itos(j) + "), the respective color attachment is marked as unused.");
				ERR_FAIL_COND_V_MSG(!(p_attachments[attachment].usage_flags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT), RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), resolve attachment, it isn't marked as a color texture.");
				ERR_FAIL_COND_V_MSG(attachment_last_pass[attachment] == i, RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it already was used for something else before in this pass.");
				bool multisample = p_attachments[attachment].samples > TEXTURE_SAMPLES_1;
				ERR_FAIL_COND_V_MSG(multisample, RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), resolve attachments can't be multisample.");
				reference.attachment = attachment_remap[attachment];
				reference.layout = RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				attachment_last_pass[attachment] = i;
			}
			reference.aspect = RDD::TEXTURE_ASPECT_COLOR_BIT;
			subpass.resolve_references.push_back(reference);
		}

		if (pass->depth_attachment != ATTACHMENT_UNUSED) {
			int32_t attachment = pass->depth_attachment;
			ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(), "Invalid framebuffer depth format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), depth attachment.");
			ERR_FAIL_COND_V_MSG(!(p_attachments[attachment].usage_flags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT), RDD::RenderPassID(), "Invalid framebuffer depth format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it's marked as depth, but it's not a depth attachment.");
			ERR_FAIL_COND_V_MSG(attachment_last_pass[attachment] == i, RDD::RenderPassID(), "Invalid framebuffer depth format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it already was used for something else before in this pass.");
			subpass.depth_stencil_reference.attachment = attachment_remap[attachment];
			subpass.depth_stencil_reference.layout = RDD::TEXTURE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachment_last_pass[attachment] = i;
      // depth attachment需要相同的samples
			if (is_multisample_first) {
				texture_samples = p_attachments[attachment].samples;
				is_multisample_first = false;
			} else { 
				ERR_FAIL_COND_V_MSG(texture_samples != p_attachments[attachment].samples, RDD::RenderPassID(), "Invalid framebuffer depth format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), if an attachment is marked as multisample, all of them should be multisample and use the same number of samples including the depth.");
			}

		} else {
			subpass.depth_stencil_reference.attachment = RDD::AttachmentReference::UNUSED;
			subpass.depth_stencil_reference.layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
		}

		if (pass->vrs_attachment != ATTACHMENT_UNUSED) {
			int32_t attachment = pass->vrs_attachment;
			ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(), "Invalid framebuffer VRS format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), VRS attachment.");
			ERR_FAIL_COND_V_MSG(!(p_attachments[attachment].usage_flags & TEXTURE_USAGE_VRS_ATTACHMENT_BIT), RDD::RenderPassID(), "Invalid framebuffer VRS format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it's marked as VRS, but it's not a VRS attachment.");
			ERR_FAIL_COND_V_MSG(attachment_last_pass[attachment] == i, RDD::RenderPassID(), "Invalid framebuffer VRS attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), it already was used for something else before in this pass.");

			subpass.vrs_reference.attachment = attachment_remap[attachment];
			subpass.vrs_reference.layout = RDD::TEXTURE_LAYOUT_VRS_ATTACHMENT_OPTIMAL;

			attachment_last_pass[attachment] = i;
		}
    // preserve_attachments和 load_op store有什么区别？
		for (int j = 0; j < pass->preserve_attachments.size(); j++) {
			int32_t attachment = pass->preserve_attachments[j];

			ERR_FAIL_COND_V_MSG(attachment == ATTACHMENT_UNUSED, RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), preserve attachment (" + itos(j) + "). Preserve attachments can't be unused.");

			ERR_FAIL_INDEX_V_MSG(attachment, p_attachments.size(), RDD::RenderPassID(), "Invalid framebuffer format attachment(" + itos(attachment) + "), in pass (" + itos(i) + "), preserve attachment (" + itos(j) + ").");

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
		if (i > 0) {
			RDD::SubpassDependency dependency;
			dependency.src_subpass = i - 1;
			dependency.dst_subpass = i;
			dependency.src_stages = (RDD::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | RDD::PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | RDD::PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
			dependency.dst_stages = (RDD::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | RDD::PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | RDD::PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | RDD::PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
			dependency.src_access = (RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
			dependency.dst_access = (RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_READ_BIT | RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | RDD::BARRIER_ACCESS_INPUT_ATTACHMENT_READ_BIT);
			subpass_dependencies.push_back(dependency);
		}
	}

	RDD::RenderPassID render_pass = driver->render_pass_create(attachments, subpasses, subpass_dependencies, p_view_count);
	ERR_FAIL_COND_V(!render_pass, RDD::RenderPassID());

	return render_pass;
}

void RenderingDevice::set_resource_name(RID p_id, const String &p_name) {
	if (texture_owner.owns(p_id)) {
		Texture *texture = texture_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_TEXTURE, texture->driver_id, p_name);
	} else if (framebuffer_owner.owns(p_id)) {
		//Framebuffer *framebuffer = framebuffer_owner.get_or_null(p_id);
		// Not implemented for now as the relationship between Framebuffer and RenderPass is very complex.
	} else if (sampler_owner.owns(p_id)) {
		RDD::SamplerID sampler_driver_id = *sampler_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_SAMPLER, sampler_driver_id, p_name);
	} else if (vertex_buffer_owner.owns(p_id)) {
		Buffer *vertex_buffer = vertex_buffer_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_BUFFER, vertex_buffer->driver_id, p_name);
	} else if (index_buffer_owner.owns(p_id)) {
		IndexBuffer *index_buffer = index_buffer_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_BUFFER, index_buffer->driver_id, p_name);
	} else if (shader_owner.owns(p_id)) {
		Shader *shader = shader_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_SHADER, shader->driver_id, p_name);
	} else if (uniform_buffer_owner.owns(p_id)) {
		Buffer *uniform_buffer = uniform_buffer_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_BUFFER, uniform_buffer->driver_id, p_name);
	} else if (texture_buffer_owner.owns(p_id)) {
		Buffer *texture_buffer = texture_buffer_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_BUFFER, texture_buffer->driver_id, p_name);
	} else if (storage_buffer_owner.owns(p_id)) {
		Buffer *storage_buffer = storage_buffer_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_BUFFER, storage_buffer->driver_id, p_name);
	} else if (uniform_set_owner.owns(p_id)) {
		UniformSet *uniform_set = uniform_set_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_UNIFORM_SET, uniform_set->driver_id, p_name);
	} else if (render_pipeline_owner.owns(p_id)) {
		RenderPipeline *pipeline = render_pipeline_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_PIPELINE, pipeline->driver_id, p_name);
	} else if (compute_pipeline_owner.owns(p_id)) {
		ComputePipeline *pipeline = compute_pipeline_owner.get_or_null(p_id);
		driver->set_object_name(RDD::OBJECT_TYPE_PIPELINE, pipeline->driver_id, p_name);
	} else {
		ERR_PRINT("Attempted to name invalid ID: " + itos(p_id.get_id()));
		return;
	}
#ifdef DEV_ENABLED
	resource_names[p_id] = p_name;
#endif
}

/// Shader compile 
Vector<uint8_t> RenderingDevice::shader_compile_spirv_from_source(ShaderStage p_stage, const String &p_source_code, ShaderLanguage p_language, String *r_error, bool p_allow_cache) {
	if (p_allow_cache && cache_function) {
		Vector<uint8_t> cache = cache_function(p_stage, p_source_code, p_language);
		if (cache.size()) {
			return cache;
		}
	}

	ERR_FAIL_NULL_V(compile_to_spirv_function, Vector<uint8_t>());

	return compile_to_spirv_function(p_stage, p_source_code, p_language, r_error, this);
}

