#include "render_scene_buffers_rd.h"
#include "texture_storage.h"
#include "render_buffer_custom_data_rd.h"
using namespace lain;

void RenderSceneBuffersRD::configure(const RenderSceneBuffersConfiguration *p_config) {
	RendererRD::TextureStorage *texture_storage = RendererRD::TextureStorage::get_singleton();
  
	render_target = p_config->get_render_target();
	target_size = p_config->get_target_size();
	internal_size = p_config->get_internal_size();
	view_count = p_config->get_view_count();

	scaling_3d_mode = p_config->get_scaling_3d_mode();
	msaa_3d = p_config->get_msaa_3d();
	screen_space_aa = p_config->get_screen_space_aa();

	fsr_sharpness = p_config->get_fsr_sharpness();
	texture_mipmap_bias = p_config->get_texture_mipmap_bias();
	use_taa = p_config->get_use_taa();
	use_debanding = p_config->get_use_debanding();

	ERR_FAIL_COND_MSG(view_count == 0, "Must have at least 1 view");

	update_samplers();

	// cleanout any old buffers we had.
	cleanup();

	// At least one of these is required to be supported.
	RenderingDeviceCommons::DataFormat preferred_format[2] = { RD::DATA_FORMAT_D24_UNORM_S8_UINT, RD::DATA_FORMAT_D32_SFLOAT_S8_UINT };
	if (can_be_storage) {
		// Prefer higher precision on desktop.
		preferred_format[0] = RD::DATA_FORMAT_D32_SFLOAT_S8_UINT;
		preferred_format[1] = RD::DATA_FORMAT_D24_UNORM_S8_UINT;
	}

	// create our 3D render buffers
	{
		// Create our color buffer(s)
		uint32_t usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | (can_be_storage ? RD::TEXTURE_USAGE_STORAGE_BIT : 0) | RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT;
		usage_bits |= RD::TEXTURE_USAGE_INPUT_ATTACHMENT_BIT; // only needed when using subpasses in the mobile renderer

		// our internal texture should have MSAA support if applicable
		if (msaa_3d != RS::VIEWPORT_MSAA_DISABLED) {
			usage_bits |= RD::TEXTURE_USAGE_CAN_COPY_TO_BIT;
		}

		create_texture(RB_SCOPE_BUFFERS, RB_TEX_COLOR, base_data_format, usage_bits);
	}

	// Create our depth buffer
	{
		// TODO Lazy create this in case we've got an external depth buffer

		RD::DataFormat format;
		uint32_t usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT;

		if (msaa_3d == RS::VIEWPORT_MSAA_DISABLED) {
			usage_bits |= RD::TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			format = RD::get_singleton()->texture_is_format_supported_for_usage(preferred_format[0], usage_bits) ? preferred_format[0] : preferred_format[1];
		} else {
			format = RD::DATA_FORMAT_R32_SFLOAT;
			usage_bits |= RD::TEXTURE_USAGE_CAN_COPY_TO_BIT | (can_be_storage ? RD::TEXTURE_USAGE_STORAGE_BIT : 0);
		}

		create_texture(RB_SCOPE_BUFFERS, RB_TEX_DEPTH, format, usage_bits);
	}

	// Create our MSAA buffers
	if (msaa_3d == RS::VIEWPORT_MSAA_DISABLED) {
		texture_samples = RD::TEXTURE_SAMPLES_1;
	} else {
		RD::DataFormat format = base_data_format;
		uint32_t usage_bits = RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT | RD::TEXTURE_USAGE_SAMPLING_BIT;

		const RD::TextureSamples ts[RS::VIEWPORT_MSAA_MAX] = {
			RD::TEXTURE_SAMPLES_1,
			RD::TEXTURE_SAMPLES_2,
			RD::TEXTURE_SAMPLES_4,
			RD::TEXTURE_SAMPLES_8,
		};

		texture_samples = ts[msaa_3d];

		create_texture(RB_SCOPE_BUFFERS, RB_TEX_COLOR_MSAA, format, usage_bits, texture_samples);

		usage_bits = RD::TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT | RD::TEXTURE_USAGE_SAMPLING_BIT;
		format = RD::get_singleton()->texture_is_format_supported_for_usage(preferred_format[0], usage_bits) ? preferred_format[0] : preferred_format[1];

		create_texture(RB_SCOPE_BUFFERS, RB_TEX_DEPTH_MSAA, format, usage_bits, texture_samples);
	}

	// VRS (note, our vrs object will only be set if VRS is supported)
	// RID vrs_texture;
	// RS::ViewportVRSMode vrs_mode = texture_storage->render_target_get_vrs_mode(render_target);
	// if (vrs && vrs_mode != RS::VIEWPORT_VRS_DISABLED) {
	// 	uint32_t usage_bits = RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TEXTURE_USAGE_VRS_ATTACHMENT_BIT | RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT;
	// 	vrs_texture = create_texture(RB_SCOPE_VRS, RB_TEXTURE, RD::DATA_FORMAT_R8_UINT, usage_bits, RD::TEXTURE_SAMPLES_1, vrs->get_vrs_texture_size(internal_size));
	// }

	// (re-)configure any named buffers
	for (KeyValue<StringName, Ref<RenderBufferCustomDataRD>> &E : data_buffers) {
		E.value->configure(this);
	}
}

void RenderSceneBuffersRD::update_samplers() {}
void RenderSceneBuffersRD::cleanup() {
	// Free our data buffers (but don't destroy them)
	for (KeyValue<StringName, Ref<RenderBufferCustomDataRD>> &E : data_buffers) {
		E.value->free_data();
	}

	// Clear our named textures
	for (KeyValue<NTKey, NamedTexture> &E : named_textures) {
		free_named_texture(E.value);
	}
	named_textures.clear();

	// // Clear weight_buffer / blur textures.
	// for (WeightBuffers &weight_buffer : weight_buffers) {
	// 	if (weight_buffer.weight.is_valid()) {
	// 		RD::get_singleton()->free(weight_buffer.weight);
	// 		weight_buffer.weight = RID();
	// 	}
	// }
}

void RenderSceneBuffersRD::free_named_texture(NamedTexture &p_named_texture) {
	if (p_named_texture.texture.is_valid()) {
		RD::get_singleton()->free(p_named_texture.texture);
	}
	p_named_texture.texture = RID();
	p_named_texture.slices.clear(); // slices should be freed automatically as dependents...
}

RID RenderSceneBuffersRD::create_texture(const StringName& p_context, const StringName& p_texture_name, const RD::DataFormat p_data_format,
                                                           const uint32_t p_usage_bits, const RD::TextureSamples p_texture_samples, const Size2i p_size,
                                                           const uint32_t p_layers, const uint32_t p_mipmaps, bool p_unique) {
  // Keep some useful data, we use default values when these are 0.
	Size2i size = p_size == Size2i(0, 0) ? internal_size : p_size;
	uint32_t layers = p_layers == 0 ? view_count : p_layers;
	uint32_t mipmaps = p_mipmaps == 0 ? 1 : p_mipmaps;

	// Create our texture
	RD::TextureFormat tf;
	tf.format = p_data_format;
	if (layers > 1) {
		tf.texture_type = RD::TEXTURE_TYPE_2D_ARRAY;
	}

	tf.width = size.x;
	tf.height = size.y;
	tf.depth = 1;
	tf.array_layers = layers;
	tf.mipmaps = mipmaps;
	tf.usage_bits = p_usage_bits;
	tf.samples = p_texture_samples;

	return create_texture_from_format(p_context, p_texture_name, tf, RD::TextureView(), p_unique);
}


RID RenderSceneBuffersRD::create_texture_from_format(const StringName &p_context, const StringName &p_texture_name, const RD::TextureFormat &p_texture_format, RD::TextureView p_view, bool p_unique) {
	// TODO p_unique, if p_unique is true, this is a texture that can be shared. This will be implemented later as an optimization.

	NTKey key(p_context, p_texture_name);

	// check if this is a known texture
	if (named_textures.has(key)) {
		return named_textures[key].texture;
	}

	// Add a new entry..
	NamedTexture &named_texture = named_textures[key];
	named_texture.format = p_texture_format;
	named_texture.is_unique = p_unique;
	named_texture.texture = RD::get_singleton()->texture_create(p_texture_format, p_view);

	Array arr;
	arr.push_back(p_context);
	arr.push_back(p_texture_name);
  RD::get_singleton()->set_resource_name(named_texture.texture, String("RenderBuffer "+String(p_context)+"/"+String(p_texture_name)));
	update_sizes(named_texture);

	// The rest is lazy created..

	return named_texture.texture;
}

void RenderSceneBuffersRD::update_sizes(NamedTexture &p_named_texture) {
	ERR_FAIL_COND(p_named_texture.texture.is_null());

	p_named_texture.sizes.resize(p_named_texture.format.mipmaps);

	Size2i mipmap_size = Size2i(p_named_texture.format.width, p_named_texture.format.height);
	for (uint32_t mipmap = 0; mipmap < p_named_texture.format.mipmaps; mipmap++) {
		p_named_texture.sizes.ptrw()[mipmap] = mipmap_size;

		mipmap_size = Size2i(mipmap_size.width() >> 1, mipmap_size.height() >> 1).maxi(1);
	}
}

