#include "rendering_device.h"
#include "_generated/serializer/all_serializer.h"
#include "core/engine/engine.h"
#include "core/io/dir_access.h"
using namespace lain::graphics;
using namespace lain;
RenderingDevice *RenderingDevice::singleton = nullptr;
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
		for (const RID &F : E->value) {
			HashMap<RID, HashSet<RID>>::Iterator G = dependency_map.find(F);
			ERR_CONTINUE(!G);// 说明建图发生了错误，但是仍然continue
			ERR_CONTINUE(!G->value.has(p_id));
			G->value.erase(p_id); 
		}

		reverse_dependency_map.remove(E); // 不再需要挨个free了
	}
}

static String _get_device_vendor_name(const RenderingContextDriver::Device &p_device) {
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
static String _get_device_type_name(const RenderingContextDriver::Device &p_device) {
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
static uint32_t _get_device_type_score(const RenderingContextDriver::Device &p_device) {
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
bool RenderingDevice::FramebufferFormatKey::operator<(const RenderingDevice::FramebufferFormatKey &p_key) const {
	if (view_count != p_key.view_count) {
		return view_count < p_key.view_count;
	}

	uint32_t pass_size = passes.size();
	uint32_t key_pass_size = p_key.passes.size();
	if (pass_size != key_pass_size) {
		return pass_size < key_pass_size;
	}
	const FramebufferPass *pass_ptr = passes.ptr();
	const FramebufferPass *key_pass_ptr = p_key.passes.ptr();

	for (uint32_t i = 0; i < pass_size; i++) {
		{ // Compare color attachments.
			uint32_t attachment_size = pass_ptr[i].color_attachments.size();
			uint32_t key_attachment_size = key_pass_ptr[i].color_attachments.size();
			if (attachment_size != key_attachment_size) {
				return attachment_size < key_attachment_size;
			}
			const int32_t *pass_attachment_ptr = pass_ptr[i].color_attachments.ptr();
			const int32_t *key_pass_attachment_ptr = key_pass_ptr[i].color_attachments.ptr();

			for (uint32_t j = 0; j < attachment_size; j++) {
				if (pass_attachment_ptr[j] != key_pass_attachment_ptr[j]) {
					return pass_attachment_ptr[j] < key_pass_attachment_ptr[j];
				}
			}
		}
		{ // Compare input attachments.
			uint32_t attachment_size = pass_ptr[i].input_attachments.size();
			uint32_t key_attachment_size = key_pass_ptr[i].input_attachments.size();
			if (attachment_size != key_attachment_size) {
				return attachment_size < key_attachment_size;
			}
			const int32_t *pass_attachment_ptr = pass_ptr[i].input_attachments.ptr();
			const int32_t *key_pass_attachment_ptr = key_pass_ptr[i].input_attachments.ptr();

			for (uint32_t j = 0; j < attachment_size; j++) {
				if (pass_attachment_ptr[j] != key_pass_attachment_ptr[j]) {
					return pass_attachment_ptr[j] < key_pass_attachment_ptr[j];
				}
			}
		}
		{ // Compare resolve attachments.
			uint32_t attachment_size = pass_ptr[i].resolve_attachments.size();
			uint32_t key_attachment_size = key_pass_ptr[i].resolve_attachments.size();
			if (attachment_size != key_attachment_size) {
				return attachment_size < key_attachment_size;
			}
			const int32_t *pass_attachment_ptr = pass_ptr[i].resolve_attachments.ptr();
			const int32_t *key_pass_attachment_ptr = key_pass_ptr[i].resolve_attachments.ptr();

			for (uint32_t j = 0; j < attachment_size; j++) {
				if (pass_attachment_ptr[j] != key_pass_attachment_ptr[j]) {
					return pass_attachment_ptr[j] < key_pass_attachment_ptr[j];
				}
			}
		}
		{ // Compare preserve attachments.
			uint32_t attachment_size = pass_ptr[i].preserve_attachments.size();
			uint32_t key_attachment_size = key_pass_ptr[i].preserve_attachments.size();
			if (attachment_size != key_attachment_size) {
				return attachment_size < key_attachment_size;
			}
			const int32_t *pass_attachment_ptr = pass_ptr[i].preserve_attachments.ptr();
			const int32_t *key_pass_attachment_ptr = key_pass_ptr[i].preserve_attachments.ptr();

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

	const AttachmentFormat *af_a = attachment_formats.ptr();
	const AttachmentFormat *af_b = p_key.attachment_formats.ptr();
	for (int i = 0; i < as; i++) {
		const AttachmentFormat &a = af_a[i];
		const AttachmentFormat &b = af_b[i];
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

	return false; // Equal.
}

RID RenderingDevice::texture_create(const TextureFormat &p_format, const TextureView &p_view,
		const Vector<Vector<uint8_t>> &p_data) {
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
			texture.usage_flags & (TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | TEXTURE_USAGE_STORAGE_BIT | TEXTURE_USAGE_STORAGE_ATOMIC_BIT | TEXTURE_USAGE_VRS_ATTACHMENT_BIT);
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
	return MAX(3U, uint32_t(GLOBAL_GET("rendering/rendering_device/vsync/swapchain_image_count"))); // 这个也要调整吗
																									// 如果这个不调可以做成const expr，许多东西（？）可以被优化
}
/// @brief RenderingDevice的initialize，需要: 选择设备，初始化该设备的driver，新建queue（main，present）
// 获得surface，
/// @param p_context
/// @param p_main_window
/// @return
Error RenderingDevice::initialize(RenderingContextDriver *p_context, WindowSystem::WindowID p_main_window) {
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

	frame = 0;
	frames.resize(frame_count);
	max_timestamp_query_elements = GLOBAL_GET("debug/settings/profiler/max_timestamp_query_elements");



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
		// frames[i].command_buffer_pool.pool = frames[i].command_pool;
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
	if (main_instance && project_pipeline_cache_enable) {
		// Only the instance that is not a local device and is also the singleton is allowed to manage a pipeline cache.
		pipeline_cache_file_path = vformat("user://vulkan/pipelines.%s.%s",
				OS::GetSingleton()->GetCurrentRenderingMethod(),
				device.name.validate_filename().replace(" ", "_").to_lower());
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

	{ // 计算差别，若差别大于save_chunk_size_mb则保存
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

void RenderingDevice::_save_pipeline_cache(void *p_data) {
	RenderingDevice *self = static_cast<RenderingDevice *>(p_data);

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
	if (p_action.load_attach & 1 << index) {
		return RDD::ATTACHMENT_LOAD_OP_LOAD;
	}
	if (p_action.clear_attach & 1 << index) {
		return RDD::ATTACHMENT_LOAD_OP_CLEAR;
	}
	if (p_action.discard_attach & 1 << index) {
		return RDD::ATTACHMENT_LOAD_OP_DONT_CARE;
	}
	return RDD::ATTACHMENT_LOAD_OP_CLEAR; // default
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
				} else { // attachment需要相同的samples
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

RID RenderingDevice::render_pipeline_create(RID p_shader, FramebufferFormatID p_framebuffer_format, VertexFormatID p_vertex_format, RenderPrimitive p_render_primitive, const PipelineRasterizationState &p_rasterization_state, const PipelineMultisampleState &p_multisample_state, const PipelineDepthStencilState &p_depth_stencil_state, const PipelineColorBlendState &p_blend_state, BitField<PipelineDynamicStateFlags> p_dynamic_state_flags, uint32_t p_for_render_pass, const Vector<PipelineSpecializationConstant> &p_specialization_constants) {
	_THREAD_SAFE_METHOD_

	// Needs a shader.
	Shader *shader = shader_owner.get_or_null(p_shader);
	ERR_FAIL_NULL_V(shader, RID());

	ERR_FAIL_COND_V_MSG(shader->is_compute, RID(),
			"Compute shaders can't be used in render pipelines");

	if (p_framebuffer_format == INVALID_ID) {
		// If nothing provided, use an empty one (no attachments).
		p_framebuffer_format = framebuffer_format_create(Vector<AttachmentFormat>());
	}
	ERR_FAIL_COND_V(!framebuffer_formats.has(p_framebuffer_format), RID());
	const FramebufferFormat &fb_format = framebuffer_formats[p_framebuffer_format];

	// Validate shader vs. framebuffer.
	{
		ERR_FAIL_COND_V_MSG(p_for_render_pass >= uint32_t(fb_format.E->key().passes.size()), RID(), "Render pass requested for pipeline creation (" + itos(p_for_render_pass) + ") is out of bounds");
		const FramebufferPass &pass = fb_format.E->key().passes[p_for_render_pass];
		uint32_t output_mask = 0;
		for (int i = 0; i < pass.color_attachments.size(); i++) {
			if (pass.color_attachments[i] != ATTACHMENT_UNUSED) {
				output_mask |= 1 << i;
			}
		} // fragment_output_mask 需要与framebuffer的color attachment 一致
		ERR_FAIL_COND_V_MSG(shader->fragment_output_mask != output_mask, RID(),
				"Mismatch fragment shader output mask (" + itos(shader->fragment_output_mask) + ") and framebuffer color output mask (" + itos(output_mask) + ") when binding both in render pipeline.");
	}

	RDD::VertexFormatID driver_vertex_format;
	if (p_vertex_format != INVALID_ID) { // p_vertex_format指向VertexDescriptionCache，即内容和driver中的ID
		// Uses vertices, else it does not.
		ERR_FAIL_COND_V(!vertex_formats.has(p_vertex_format), RID());
		const VertexDescriptionCache &vd = vertex_formats[p_vertex_format];
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

			ERR_FAIL_COND_V_MSG(!found, RID(),
					"Shader vertex input location (" + itos(i) + ") not provided in vertex input description for pipeline creation.");
		}

	} else {
		ERR_FAIL_COND_V_MSG(shader->vertex_input_mask != 0, RID(),
				"Shader contains vertex inputs, but no vertex input description was provided for pipeline creation.");
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

	const FramebufferPass &pass = fb_format.E->key().passes[p_for_render_pass];
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
		const ShaderSpecializationConstant &sc = shader->specialization_constants[i];
		for (int j = 0; j < p_specialization_constants.size(); j++) {
			const PipelineSpecializationConstant &psc = p_specialization_constants[j];
			if (psc.constant_id == sc.constant_id) {
				ERR_FAIL_COND_V_MSG(psc.type != sc.type, RID(), "Specialization constant provided for id (" + itos(sc.constant_id) + ") is of the wrong type.");
				break;
			}
		}
	}

	RenderPipeline pipeline;
	pipeline.driver_id = driver->render_pipeline_create(
			shader->driver_id,
			driver_vertex_format,
			p_render_primitive,
			p_rasterization_state,
			p_multisample_state,
			p_depth_stencil_state,
			p_blend_state,
			pass.color_attachments,
			p_dynamic_state_flags,
			fb_format.render_pass,
			p_for_render_pass,
			p_specialization_constants);
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

	static const uint32_t primitive_divisor[RENDER_PRIMITIVE_MAX] = {
		1, 2, 1, 1, 1, 3, 1, 1, 1, 1, 1
	};
	pipeline.validation.primitive_divisor = primitive_divisor[p_render_primitive];
	static const uint32_t primitive_minimum[RENDER_PRIMITIVE_MAX] = {
		1,
		2,
		2,
		2,
		2,
		3,
		3,
		3,
		3,
		3,
		1,
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

RID RenderingDevice::compute_pipeline_create(RID p_shader, const Vector<PipelineSpecializationConstant> &p_specialization_constants) {
	_THREAD_SAFE_METHOD_

	// Needs a shader.
	Shader *shader = shader_owner.get_or_null(p_shader);
	ERR_FAIL_NULL_V(shader, RID());

	ERR_FAIL_COND_V_MSG(!shader->is_compute, RID(),
			"Non-compute shaders can't be used in compute pipelines");

	for (int i = 0; i < shader->specialization_constants.size(); i++) {
		const ShaderSpecializationConstant &sc = shader->specialization_constants[i];
		for (int j = 0; j < p_specialization_constants.size(); j++) {
			const PipelineSpecializationConstant &psc = p_specialization_constants[j];
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

	_free_dependencies(p_id); // Recursively erase dependencies first, to avoid potential API problems.
	_free_internal(p_id);
}

void RenderingDevice::_free_internal(RID p_id) {
#ifdef DEV_ENABLED
	String resource_name;
	if (resource_names.has(p_id)) {
		resource_name = resource_names[p_id];
		resource_names.erase(p_id);
	}
#endif

// 	// Push everything so it's disposed of next time this frame index is processed (means, it's safe to do it).
// 	if (texture_owner.owns(p_id)) {
// 		Texture *texture = texture_owner.get_or_null(p_id);
// 		RDG::ResourceTracker *draw_tracker = texture->draw_tracker;
// 		if (draw_tracker != nullptr) {
// 			draw_tracker->reference_count--;
// 			if (draw_tracker->reference_count == 0) {
// 				RDG::resource_tracker_free(draw_tracker);

// 				if (texture->owner.is_valid() && (texture->slice_type != TEXTURE_SLICE_MAX)) {
// 					// If this was a texture slice, erase the tracker from the map.
// 					Texture *owner_texture = texture_owner.get_or_null(texture->owner);
// 					if (owner_texture != nullptr) {
// 						owner_texture->slice_trackers.erase(texture->slice_rect);
// 					}
// 				}
// 			}
// 		}

// 		frames[frame].textures_to_dispose_of.push_back(*texture);
// 		texture_owner.free(p_id);
// 	} else if (framebuffer_owner.owns(p_id)) {
// 		Framebuffer *framebuffer = framebuffer_owner.get_or_null(p_id);
// 		frames[frame].framebuffers_to_dispose_of.push_back(*framebuffer);

// 		if (framebuffer->invalidated_callback != nullptr) {
// 			framebuffer->invalidated_callback(framebuffer->invalidated_callback_userdata);
// 		}

// 		framebuffer_owner.free(p_id);
// 	} else if (sampler_owner.owns(p_id)) {
// 		RDD::SamplerID sampler_driver_id = *sampler_owner.get_or_null(p_id);
// 		frames[frame].samplers_to_dispose_of.push_back(sampler_driver_id);
// 		sampler_owner.free(p_id);
// 	} else if (vertex_buffer_owner.owns(p_id)) {
// 		Buffer *vertex_buffer = vertex_buffer_owner.get_or_null(p_id);
// 		RDG::resource_tracker_free(vertex_buffer->draw_tracker);
// 		frames[frame].buffers_to_dispose_of.push_back(*vertex_buffer);
// 		vertex_buffer_owner.free(p_id);
// 	} else if (vertex_array_owner.owns(p_id)) {
// 		vertex_array_owner.free(p_id);
// 	} else if (index_buffer_owner.owns(p_id)) {
// 		IndexBuffer *index_buffer = index_buffer_owner.get_or_null(p_id);
// 		RDG::resource_tracker_free(index_buffer->draw_tracker);
// 		frames[frame].buffers_to_dispose_of.push_back(*index_buffer);
// 		index_buffer_owner.free(p_id);
// 	} else if (index_array_owner.owns(p_id)) {
// 		index_array_owner.free(p_id);
// 	} else if (shader_owner.owns(p_id)) {
// 		Shader *shader = shader_owner.get_or_null(p_id);
// 		if (shader->driver_id) { // Not placeholder?
// 			frames[frame].shaders_to_dispose_of.push_back(*shader);
// 		}
// 		shader_owner.free(p_id);
// 	} else if (uniform_buffer_owner.owns(p_id)) {
// 		Buffer *uniform_buffer = uniform_buffer_owner.get_or_null(p_id);
// 		RDG::resource_tracker_free(uniform_buffer->draw_tracker);
// 		frames[frame].buffers_to_dispose_of.push_back(*uniform_buffer);
// 		uniform_buffer_owner.free(p_id);
// 	} else if (texture_buffer_owner.owns(p_id)) {
// 		Buffer *texture_buffer = texture_buffer_owner.get_or_null(p_id);
// 		RDG::resource_tracker_free(texture_buffer->draw_tracker);
// 		frames[frame].buffers_to_dispose_of.push_back(*texture_buffer);
// 		texture_buffer_owner.free(p_id);
// 	} else if (storage_buffer_owner.owns(p_id)) {
// 		Buffer *storage_buffer = storage_buffer_owner.get_or_null(p_id);
// 		RDG::resource_tracker_free(storage_buffer->draw_tracker);
// 		frames[frame].buffers_to_dispose_of.push_back(*storage_buffer);
// 		storage_buffer_owner.free(p_id);
// 	} else if (uniform_set_owner.owns(p_id)) {
// 		UniformSet *uniform_set = uniform_set_owner.get_or_null(p_id);
// 		frames[frame].uniform_sets_to_dispose_of.push_back(*uniform_set);
// 		uniform_set_owner.free(p_id);

// 		if (uniform_set->invalidated_callback != nullptr) {
// 			uniform_set->invalidated_callback(uniform_set->invalidated_callback_userdata);
// 		}
// 	} else if (render_pipeline_owner.owns(p_id)) {
// 		RenderPipeline *pipeline = render_pipeline_owner.get_or_null(p_id);
// 		frames[frame].render_pipelines_to_dispose_of.push_back(*pipeline);
// 		render_pipeline_owner.free(p_id);
// 	} else if (compute_pipeline_owner.owns(p_id)) {
// 		ComputePipeline *pipeline = compute_pipeline_owner.get_or_null(p_id);
// 		frames[frame].compute_pipelines_to_dispose_of.push_back(*pipeline);
// 		compute_pipeline_owner.free(p_id);
// 	} else {
// #ifdef DEV_ENABLED
// 		ERR_PRINT("Attempted to free invalid ID: " + itos(p_id.get_id()) + " " + resource_name);
// #else
// 		ERR_PRINT("Attempted to free invalid ID: " + itos(p_id.get_id()));
// #endif
// 	}
}


/*******************/
/**** DRAW LIST ****/
/*******************/

Error RenderingDevice::_draw_list_allocate(const Rect2i &p_viewport, uint32_t p_subpass) {
	// Lock while draw_list is active.
	_THREAD_SAFE_LOCK_

	draw_list = memnew(DrawList);
	draw_list->viewport = p_viewport;

	return OK;
}


RenderingDevice::DrawListID RenderingDevice::draw_list_begin_for_screen(WindowSystem::WindowID p_screen, const Color &p_clear_color) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V_MSG(draw_list != nullptr, INVALID_ID, "Only one draw list can be active at the same time.");
	ERR_FAIL_COND_V_MSG(compute_list != nullptr, INVALID_ID, "Only one draw/compute list can be active at the same time.");

	RenderingContextDriver::SurfaceID surface = context->surface_get_from_window(p_screen);
	HashMap<WindowSystem::WindowID, RDD::SwapChainID>::ConstIterator sc_it = screen_swap_chains.find(p_screen);
	HashMap<WindowSystem::WindowID, RDD::FramebufferID>::ConstIterator fb_it = screen_framebuffers.find(p_screen);
	ERR_FAIL_COND_V_MSG(surface == 0, 0, "A surface was not created for the screen.");
	ERR_FAIL_COND_V_MSG(sc_it == screen_swap_chains.end(), INVALID_ID, "Screen was never prepared.");
	ERR_FAIL_COND_V_MSG(fb_it == screen_framebuffers.end(), INVALID_ID, "Framebuffer was never prepared.");
	// 这里使用了context的api，而非window_system的api
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

void RenderingDevice::_draw_list_set_viewport(Rect2i p_rect) {
	draw_graph.add_draw_list_set_viewport(p_rect);
}

void RenderingDevice::_draw_list_set_scissor(Rect2i p_rect) {
	draw_graph.add_draw_list_set_scissor(p_rect);
}