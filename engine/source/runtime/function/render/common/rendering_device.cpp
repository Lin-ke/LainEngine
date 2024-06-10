#include "rendering_device.h"
#include "_generated/serializer/all_serializer.h"
namespace lain::graphics {
	/*********************/
	/**** (RenderPass) ****/
	/*********************/


	RID RenderingDevice::texture_create(const TextureFormat& p_format, const TextureView& p_view, const Vector<Vector<uint8_t>>& p_data) {
		_THREAD_SAFE_METHOD_

			// Some adjustments will happen.
			TextureFormat format = p_format;

		if (format.shareable_formats.size()) {
			ERR_FAIL_COND_V_MSG(format.shareable_formats.find(format.format) == -1, RID(),
				"If supplied a list of shareable formats, the current format must be present in the list");
			ERR_FAIL_COND_V_MSG(p_view.format_override != DATA_FORMAT_MAX && format.shareable_formats.find(p_view.format_override) == -1, RID(),
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

		if (format.texture_type == TEXTURE_TYPE_1D_ARRAY || format.texture_type == TEXTURE_TYPE_2D_ARRAY || format.texture_type == TEXTURE_TYPE_CUBE_ARRAY || format.texture_type == TEXTURE_TYPE_CUBE) {
			ERR_FAIL_COND_V_MSG(format.array_layers < 1, RID(),
				"Amount of layers must be equal or greater than 1 for arrays and cubemaps.");
			ERR_FAIL_COND_V_MSG((format.texture_type == TEXTURE_TYPE_CUBE_ARRAY || format.texture_type == TEXTURE_TYPE_CUBE) && (format.array_layers % 6) != 0, RID(),
				"Cubemap and cubemap array textures must provide a layer number that is multiple of 6");
		}
		else {
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
					"Data for slice index " + itos(i) + " (mapped to layer " + itos(i) + ") differs in size (supplied: " + itos(p_data[i].size()) + ") than what is required by the format (" + itos(required_size) + ").");
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
					ERR_FAIL_V_MSG(RID(), "Format " + format_text + " does not support usage as " + Serializer::write(usage).string_value());
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
		}
		else {
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
		}
		else {
			texture.read_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_COLOR_BIT);
			texture.barrier_aspect_flags.set_flag(RDD::TEXTURE_ASPECT_COLOR_BIT);
		}

		texture.bound = false;

		// Textures are only assumed to be immutable if they have initial data and none of the other bits that indicate write usage are enabled.
		bool texture_mutable_by_default = texture.usage_flags & (TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | TEXTURE_USAGE_STORAGE_BIT | TEXTURE_USAGE_STORAGE_ATOMIC_BIT | TEXTURE_USAGE_VRS_ATTACHMENT_BIT);
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

			//if (texture.draw_tracker != nullptr) {
			//	// Draw tracker can assume the texture will be in transfer destination.
			//	texture.draw_tracker->usage = RDG::RESOURCE_USAGE_TRANSFER_TO;
			//}
		}

		return id;
	}
}