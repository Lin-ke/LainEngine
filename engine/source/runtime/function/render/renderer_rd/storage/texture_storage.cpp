#include "texture_storage.h"
#include "function/render/rendering_device/rendering_device.h"
#include "framebuffer_cache_rd.h"
using namespace lain::RendererRD;
using namespace lain;
TextureStorage* TextureStorage::singleton = nullptr;
TextureStorage::TextureStorage() {
  singleton = this;
  // create defalut textures
  
	{ //create default textures

		RD::TextureFormat tformat;
		tformat.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
		tformat.width = 4;
		tformat.height = 4;
		tformat.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT;
		tformat.texture_type = RD::TEXTURE_TYPE_2D;

		Vector<uint8_t> pv;
		pv.resize(16 * 4);
		for (int i = 0; i < 16; i++) {
			// Opaque white.
			pv.set(i * 4 + 0, 255);
			pv.set(i * 4 + 1, 255);
			pv.set(i * 4 + 2, 255);
			pv.set(i * 4 + 3, 255);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			vpv.push_back(pv);
			default_rd_textures[DEFAULT_RD_TEXTURE_WHITE] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}

		for (int i = 0; i < 16; i++) {
			// Opaque black.
			pv.set(i * 4 + 0, 0);
			pv.set(i * 4 + 1, 0);
			pv.set(i * 4 + 2, 0);
			pv.set(i * 4 + 3, 255);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			vpv.push_back(pv);
			default_rd_textures[DEFAULT_RD_TEXTURE_BLACK] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}

		for (int i = 0; i < 16; i++) {
			// Transparent black.
			pv.set(i * 4 + 0, 0);
			pv.set(i * 4 + 1, 0);
			pv.set(i * 4 + 2, 0);
			pv.set(i * 4 + 3, 0);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			vpv.push_back(pv);
			default_rd_textures[DEFAULT_RD_TEXTURE_TRANSPARENT] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}

		for (int i = 0; i < 16; i++) {
			// Opaque normal map "flat" color.
			pv.set(i * 4 + 0, 128);
			pv.set(i * 4 + 1, 128);
			pv.set(i * 4 + 2, 255);
			pv.set(i * 4 + 3, 255);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			vpv.push_back(pv);
			default_rd_textures[DEFAULT_RD_TEXTURE_NORMAL] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}

		for (int i = 0; i < 16; i++) {
			// Opaque flowmap "flat" color.
			pv.set(i * 4 + 0, 255);
			pv.set(i * 4 + 1, 128);
			pv.set(i * 4 + 2, 255);
			pv.set(i * 4 + 3, 255);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			vpv.push_back(pv);
			default_rd_textures[DEFAULT_RD_TEXTURE_ANISO] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}

		{
			RD::TextureFormat tf;
			tf.format = RD::DATA_FORMAT_D16_UNORM;
			tf.width = 4;
			tf.height = 4;
			tf.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT | RD::TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			tf.texture_type = RD::TEXTURE_TYPE_2D;

			Vector<uint8_t> sv;
			sv.resize(16 * 2);
			uint16_t *ptr = (uint16_t *)sv.ptrw();
			for (int i = 0; i < 16; i++) {
				ptr[i] = Math::make_half_float(1.0f);
			}

			Vector<Vector<uint8_t>> vpv;
			vpv.push_back(sv);
			default_rd_textures[DEFAULT_RD_TEXTURE_DEPTH] = RD::get_singleton()->texture_create(tf, RD::TextureView(), vpv);
		}

		for (int i = 0; i < 16; i++) {
			pv.set(i * 4 + 0, 0);
			pv.set(i * 4 + 1, 0);
			pv.set(i * 4 + 2, 0);
			pv.set(i * 4 + 3, 0);
		}

		default_rd_textures[DEFAULT_RD_TEXTURE_MULTIMESH_BUFFER] = RD::get_singleton()->texture_buffer_create(16, RD::DATA_FORMAT_R8G8B8A8_UNORM, pv);

		for (int i = 0; i < 16; i++) {
			pv.set(i * 4 + 0, 0);
			pv.set(i * 4 + 1, 0);
			pv.set(i * 4 + 2, 0);
			pv.set(i * 4 + 3, 0);
		}

		{
			tformat.format = RD::DATA_FORMAT_R8G8B8A8_UINT;
			Vector<Vector<uint8_t>> vpv;
			vpv.push_back(pv);
			default_rd_textures[DEFAULT_RD_TEXTURE_2D_UINT] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}
	}

	{ //create default black cubemap array

		RD::TextureFormat tformat;
		tformat.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
		tformat.width = 4;
		tformat.height = 4;
		tformat.array_layers = 6;
		tformat.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT;
		tformat.texture_type = RD::TEXTURE_TYPE_CUBE_ARRAY;

		Vector<uint8_t> pv;
		pv.resize(16 * 4);
		for (int i = 0; i < 16; i++) {
			pv.set(i * 4 + 0, 0);
			pv.set(i * 4 + 1, 0);
			pv.set(i * 4 + 2, 0);
			pv.set(i * 4 + 3, 0);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			for (int i = 0; i < 6; i++) {
				vpv.push_back(pv);
			}
			default_rd_textures[DEFAULT_RD_TEXTURE_CUBEMAP_ARRAY_BLACK] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}
	}

	{ //create default white cubemap array

		RD::TextureFormat tformat;
		tformat.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
		tformat.width = 4;
		tformat.height = 4;
		tformat.array_layers = 6;
		tformat.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT;
		tformat.texture_type = RD::TEXTURE_TYPE_CUBE_ARRAY;

		Vector<uint8_t> pv;
		pv.resize(16 * 4);
		for (int i = 0; i < 16; i++) {
			pv.set(i * 4 + 0, 255);
			pv.set(i * 4 + 1, 255);
			pv.set(i * 4 + 2, 255);
			pv.set(i * 4 + 3, 255);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			for (int i = 0; i < 6; i++) {
				vpv.push_back(pv);
			}
			default_rd_textures[DEFAULT_RD_TEXTURE_CUBEMAP_ARRAY_WHITE] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}
	}

	{ //create default black cubemap

		RD::TextureFormat tformat;
		tformat.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
		tformat.width = 4;
		tformat.height = 4;
		tformat.array_layers = 6;
		tformat.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT;
		tformat.texture_type = RD::TEXTURE_TYPE_CUBE;

		Vector<uint8_t> pv;
		pv.resize(16 * 4);
		for (int i = 0; i < 16; i++) {
			pv.set(i * 4 + 0, 0);
			pv.set(i * 4 + 1, 0);
			pv.set(i * 4 + 2, 0);
			pv.set(i * 4 + 3, 0);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			for (int i = 0; i < 6; i++) {
				vpv.push_back(pv);
			}
			default_rd_textures[DEFAULT_RD_TEXTURE_CUBEMAP_BLACK] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}
	}

	{ //create default white cubemap

		RD::TextureFormat tformat;
		tformat.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
		tformat.width = 4;
		tformat.height = 4;
		tformat.array_layers = 6;
		tformat.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT;
		tformat.texture_type = RD::TEXTURE_TYPE_CUBE;

		Vector<uint8_t> pv;
		pv.resize(16 * 4);
		for (int i = 0; i < 16; i++) {
			pv.set(i * 4 + 0, 255);
			pv.set(i * 4 + 1, 255);
			pv.set(i * 4 + 2, 255);
			pv.set(i * 4 + 3, 255);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			for (int i = 0; i < 6; i++) {
				vpv.push_back(pv);
			}
			default_rd_textures[DEFAULT_RD_TEXTURE_CUBEMAP_WHITE] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}
	}

	{ //create default 3D

		RD::TextureFormat tformat;
		tformat.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
		tformat.width = 4;
		tformat.height = 4;
		tformat.depth = 4;
		tformat.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT;
		tformat.texture_type = RD::TEXTURE_TYPE_3D;

		Vector<uint8_t> pv;
		pv.resize(64 * 4);
		for (int i = 0; i < 64; i++) {
			pv.set(i * 4 + 0, 0);
			pv.set(i * 4 + 1, 0);
			pv.set(i * 4 + 2, 0);
			pv.set(i * 4 + 3, 0);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			vpv.push_back(pv);
			default_rd_textures[DEFAULT_RD_TEXTURE_3D_BLACK] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}
		for (int i = 0; i < 64; i++) {
			pv.set(i * 4 + 0, 255);
			pv.set(i * 4 + 1, 255);
			pv.set(i * 4 + 2, 255);
			pv.set(i * 4 + 3, 255);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			vpv.push_back(pv);
			default_rd_textures[DEFAULT_RD_TEXTURE_3D_WHITE] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}
	}

	{ //create default array white

		RD::TextureFormat tformat;
		tformat.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
		tformat.width = 4;
		tformat.height = 4;
		tformat.array_layers = 1;
		tformat.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT;
		tformat.texture_type = RD::TEXTURE_TYPE_2D_ARRAY;

		Vector<uint8_t> pv;
		pv.resize(16 * 4);
		for (int i = 0; i < 16; i++) {
			pv.set(i * 4 + 0, 255);
			pv.set(i * 4 + 1, 255);
			pv.set(i * 4 + 2, 255);
			pv.set(i * 4 + 3, 255);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			vpv.push_back(pv);
			default_rd_textures[DEFAULT_RD_TEXTURE_2D_ARRAY_WHITE] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}
	}

	{ //create default array black

		RD::TextureFormat tformat;
		tformat.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
		tformat.width = 4;
		tformat.height = 4;
		tformat.array_layers = 1;
		tformat.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT;
		tformat.texture_type = RD::TEXTURE_TYPE_2D_ARRAY;

		Vector<uint8_t> pv;
		pv.resize(16 * 4);
		for (int i = 0; i < 16; i++) {
			pv.set(i * 4 + 0, 0);
			pv.set(i * 4 + 1, 0);
			pv.set(i * 4 + 2, 0);
			pv.set(i * 4 + 3, 0);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			vpv.push_back(pv);
			default_rd_textures[DEFAULT_RD_TEXTURE_2D_ARRAY_BLACK] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}
	}

	{ //create default array normal

		RD::TextureFormat tformat;
		tformat.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
		tformat.width = 4;
		tformat.height = 4;
		tformat.array_layers = 1;
		tformat.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT;
		tformat.texture_type = RD::TEXTURE_TYPE_2D_ARRAY;

		Vector<uint8_t> pv;
		pv.resize(16 * 4);
		for (int i = 0; i < 16; i++) {
			pv.set(i * 4 + 0, 128);
			pv.set(i * 4 + 1, 128);
			pv.set(i * 4 + 2, 255);
			pv.set(i * 4 + 3, 255);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			vpv.push_back(pv);
			default_rd_textures[DEFAULT_RD_TEXTURE_2D_ARRAY_NORMAL] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}
	}

	{ //create default array depth

		RD::TextureFormat tformat;
		tformat.format = RD::DATA_FORMAT_D16_UNORM;
		tformat.width = 4;
		tformat.height = 4;
		tformat.array_layers = 1;
		tformat.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT | RD::TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		tformat.texture_type = RD::TEXTURE_TYPE_2D_ARRAY;

		Vector<uint8_t> sv;
		sv.resize(16 * 2);
		uint16_t *ptr = (uint16_t *)sv.ptrw();
		for (int i = 0; i < 16; i++) {
			ptr[i] = Math::make_half_float(1.0f);
		}

		{
			Vector<Vector<uint8_t>> vsv;
			vsv.push_back(sv);
			default_rd_textures[DEFAULT_RD_TEXTURE_2D_ARRAY_DEPTH] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vsv);
		}
	}

	{ // default atlas texture
		RD::TextureFormat tformat;
		tformat.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
		tformat.width = 4;
		tformat.height = 4;
		tformat.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT;
		tformat.texture_type = RD::TEXTURE_TYPE_2D;

		Vector<uint8_t> pv;
		pv.resize(16 * 4);
		for (int i = 0; i < 16; i++) {
			pv.set(i * 4 + 0, 0);
			pv.set(i * 4 + 1, 0);
			pv.set(i * 4 + 2, 0);
			pv.set(i * 4 + 3, 255);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			vpv.push_back(pv);
			decal_atlas.texture = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
			decal_atlas.texture_srgb = decal_atlas.texture;
		}
	}

	{ //create default VRS

		RD::TextureFormat tformat;
		tformat.format = RD::DATA_FORMAT_R8_UINT;
		tformat.width = 4;
		tformat.height = 4;
		tformat.usage_bits = RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT | RD::TEXTURE_USAGE_VRS_ATTACHMENT_BIT;
		tformat.texture_type = RD::TEXTURE_TYPE_2D;
		if (!RD::get_singleton()->is_feature_supported(RD::SUPPORTS_ATTACHMENT_VRS)) {
			tformat.usage_bits = RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT;
		}

		Vector<uint8_t> pv;
		pv.resize(4 * 4);
		for (int i = 0; i < 4 * 4; i++) {
			pv.set(i, 0);
		}

		{
			Vector<Vector<uint8_t>> vpv;
			vpv.push_back(pv);
			default_rd_textures[DEFAULT_RD_TEXTURE_VRS] = RD::get_singleton()->texture_create(tformat, RD::TextureView(), vpv);
		}
	}

	// {
	// 	Vector<String> sdf_modes;
	// 	sdf_modes.push_back("\n#define MODE_LOAD\n");
	// 	sdf_modes.push_back("\n#define MODE_LOAD_SHRINK\n");
	// 	sdf_modes.push_back("\n#define MODE_PROCESS\n");
	// 	sdf_modes.push_back("\n#define MODE_PROCESS_OPTIMIZED\n");
	// 	sdf_modes.push_back("\n#define MODE_STORE\n");
	// 	sdf_modes.push_back("\n#define MODE_STORE_SHRINK\n");

	// 	rt_sdf.shader.initialize(sdf_modes);

	// 	rt_sdf.shader_version = rt_sdf.shader.version_create();

	// 	for (int i = 0; i < RenderTargetSDF::SHADER_MAX; i++) {
	// 		rt_sdf.pipelines[i] = RD::get_singleton()->compute_pipeline_create(rt_sdf.shader.version_get_shader(rt_sdf.shader_version, i));
	// 	}
	// }
}

TextureStorage::~TextureStorage() {
  singleton = nullptr;
}

bool lain::RendererRD::TextureStorage::owns_decal(RID p_rid) {
  return false;
}

bool lain::RendererRD::TextureStorage::owns_texture(RID p_rid) {
  return texture_owner.owns(p_rid); 
}

bool TextureStorage::free(RID p_rid) {
	if (owns_texture(p_rid)) {
		texture_free(p_rid);
		return true;
	} 
  // else if (owns_canvas_texture(p_rid)) {
	// 	canvas_texture_free(p_rid);
	// 	return true;
	// } else if (owns_decal(p_rid)) {
	// 	decal_free(p_rid);
	// 	return true;
	// } else if (owns_decal_instance(p_rid)) {
	// 	decal_instance_free(p_rid);
	// 	return true;
	// }
   else if (owns_render_target(p_rid)) {
		render_target_free(p_rid);
		return true;
	}

	return false;
}

Ref<Image> TextureStorage::_validate_texture_format(const Ref<Image>& p_image, TextureToRDFormat& r_format) {
  Ref<Image> image = p_image->duplicate();

  switch (p_image->get_format()) {
    case Image::FORMAT_L8: {
      r_format.format = RD::DATA_FORMAT_R8_UNORM;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;  //luminance
    case Image::FORMAT_LA8: {
      r_format.format = RD::DATA_FORMAT_R8G8_UNORM;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_G;
    } break;  //luminance-alpha
    case Image::FORMAT_R8: {
      r_format.format = RD::DATA_FORMAT_R8_UNORM;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;
    case Image::FORMAT_RG8: {
      r_format.format = RD::DATA_FORMAT_R8G8_UNORM;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;
    case Image::FORMAT_RGB8: {
      //this format is not mandatory for specification, check if supported first
      if (false &&
          RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_R8G8B8_UNORM, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT) &&
          RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_R8G8B8_SRGB, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_R8G8B8_UNORM;
        r_format.format_srgb = RD::DATA_FORMAT_R8G8B8_SRGB;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
        r_format.format_srgb = RD::DATA_FORMAT_R8G8B8A8_SRGB;
        image->convert(Image::FORMAT_RGBA8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;
    case Image::FORMAT_RGBA8: {
      r_format.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
      r_format.format_srgb = RD::DATA_FORMAT_R8G8B8A8_SRGB;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;
    } break;
    case Image::FORMAT_RGBA4444: {
      r_format.format = RD::DATA_FORMAT_B4G4R4A4_UNORM_PACK16;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_B;  //needs swizzle
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;
    } break;
    case Image::FORMAT_RGB565: {
      r_format.format = RD::DATA_FORMAT_B5G6R5_UNORM_PACK16;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;
    } break;
    case Image::FORMAT_RF: {
      r_format.format = RD::DATA_FORMAT_R32_SFLOAT;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;  //float
    case Image::FORMAT_RGF: {
      r_format.format = RD::DATA_FORMAT_R32G32_SFLOAT;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;
    case Image::FORMAT_RGBF: {
      //this format is not mandatory for specification, check if supported first
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_R32G32B32_SFLOAT, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_R32G32B32_SFLOAT;  // 如果支持就用，否则用RGBA
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R32G32B32A32_SFLOAT;
        image->convert(Image::FORMAT_RGBAF);
      }

      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;
    case Image::FORMAT_RGBAF: {
      r_format.format = RD::DATA_FORMAT_R32G32B32A32_SFLOAT;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;

    } break;
    case Image::FORMAT_RH: {
      r_format.format = RD::DATA_FORMAT_R16_SFLOAT;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;  //half float
    case Image::FORMAT_RGH: {
      r_format.format = RD::DATA_FORMAT_R16G16_SFLOAT;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;
    case Image::FORMAT_RGBH: {
      //this format is not mandatory for specification, check if supported first
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_R16G16B16_SFLOAT, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_R16G16B16_SFLOAT;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R16G16B16A16_SFLOAT;
        image->convert(Image::FORMAT_RGBAH);
      }

      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;
    case Image::FORMAT_RGBAH: {
      r_format.format = RD::DATA_FORMAT_R16G16B16A16_SFLOAT;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;

    } break;
    case Image::FORMAT_RGBE9995: {
      r_format.format = RD::DATA_FORMAT_E5B9G9R9_UFLOAT_PACK32;
      // TODO: Need to make a function in Image to swap bits for this.
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_IDENTITY;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_IDENTITY;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_IDENTITY;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_IDENTITY;
    } break;
    case Image::FORMAT_DXT1: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_BC1_RGB_UNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_BC1_RGB_UNORM_BLOCK;
        r_format.format_srgb = RD::DATA_FORMAT_BC1_RGB_SRGB_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
        r_format.format_srgb = RD::DATA_FORMAT_R8G8B8A8_SRGB;
        image->decompress();
        image->convert(Image::FORMAT_RGBA8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;  //s3tc bc1
    case Image::FORMAT_DXT3: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_BC2_UNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_BC2_UNORM_BLOCK;
        r_format.format_srgb = RD::DATA_FORMAT_BC2_SRGB_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
        r_format.format_srgb = RD::DATA_FORMAT_R8G8B8A8_SRGB;
        image->decompress();
        image->convert(Image::FORMAT_RGBA8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;

    } break;  //bc2
    case Image::FORMAT_DXT5: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_BC3_UNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_BC3_UNORM_BLOCK;
        r_format.format_srgb = RD::DATA_FORMAT_BC3_SRGB_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
        r_format.format_srgb = RD::DATA_FORMAT_R8G8B8A8_SRGB;
        image->decompress();
        image->convert(Image::FORMAT_RGBA8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;
    } break;  //bc3
    case Image::FORMAT_RGTC_R: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_BC4_UNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_BC4_UNORM_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8_UNORM;
        image->decompress();
        image->convert(Image::FORMAT_R8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;
    case Image::FORMAT_RGTC_RG: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_BC5_UNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_BC5_UNORM_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8G8_UNORM;
        image->decompress();
        image->convert(Image::FORMAT_RG8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;
    case Image::FORMAT_BPTC_RGBA: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_BC7_UNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_BC7_UNORM_BLOCK;
        r_format.format_srgb = RD::DATA_FORMAT_BC7_SRGB_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
        r_format.format_srgb = RD::DATA_FORMAT_R8G8B8A8_SRGB;
        image->decompress();
        image->convert(Image::FORMAT_RGBA8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;

    } break;  //btpc bc7
    case Image::FORMAT_BPTC_RGBF: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_BC6H_SFLOAT_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_BC6H_SFLOAT_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R16G16B16A16_SFLOAT;
        image->decompress();
        image->convert(Image::FORMAT_RGBAH);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;  //float bc6h
    case Image::FORMAT_BPTC_RGBFU: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_BC6H_UFLOAT_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_BC6H_UFLOAT_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R16G16B16A16_SFLOAT;
        image->decompress();
        image->convert(Image::FORMAT_RGBAH);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;  //unsigned float bc6hu
    case Image::FORMAT_ETC2_R11: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_EAC_R11_UNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_EAC_R11_UNORM_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8_UNORM;
        image->decompress();
        image->convert(Image::FORMAT_R8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;  //etc2
    case Image::FORMAT_ETC2_R11S: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_EAC_R11_SNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_EAC_R11_SNORM_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8_SNORM;
        image->decompress();
        image->convert(Image::FORMAT_R8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;  //signed: {} break; NOT srgb.
    case Image::FORMAT_ETC2_RG11: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_EAC_R11G11_UNORM_BLOCK,
                                                                     RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_EAC_R11G11_UNORM_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8G8_UNORM;
        image->decompress();
        image->convert(Image::FORMAT_RG8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;
    case Image::FORMAT_ETC2_RG11S: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_EAC_R11G11_SNORM_BLOCK,
                                                                     RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_EAC_R11G11_SNORM_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8G8_SNORM;
        image->decompress();
        image->convert(Image::FORMAT_RG8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;
    case Image::FORMAT_ETC:
    case Image::FORMAT_ETC2_RGB8: {
      //ETC2 is backwards compatible with ETC1, and all modern platforms support it
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
                                                                     RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        r_format.format_srgb = RD::DATA_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
        r_format.format_srgb = RD::DATA_FORMAT_R8G8B8A8_SRGB;
        image->decompress();
        image->convert(Image::FORMAT_RGBA8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;
    case Image::FORMAT_ETC2_RGBA8: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
                                                                     RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        r_format.format_srgb = RD::DATA_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
        r_format.format_srgb = RD::DATA_FORMAT_R8G8B8A8_SRGB;
        image->decompress();
        image->convert(Image::FORMAT_RGBA8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;
    } break;
    case Image::FORMAT_ETC2_RGB8A1: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
                                                                     RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
        r_format.format_srgb = RD::DATA_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
        r_format.format_srgb = RD::DATA_FORMAT_R8G8B8A8_SRGB;
        image->decompress();
        image->convert(Image::FORMAT_RGBA8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;
    } break;
    case Image::FORMAT_ETC2_RA_AS_RG: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
                                                                     RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        r_format.format_srgb = RD::DATA_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
        r_format.format_srgb = RD::DATA_FORMAT_R8G8B8A8_SRGB;
        image->decompress();
        image->convert(Image::FORMAT_RGBA8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_A;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;
    case Image::FORMAT_DXT5_RA_AS_RG: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_BC3_UNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_BC3_UNORM_BLOCK;
        r_format.format_srgb = RD::DATA_FORMAT_BC3_SRGB_BLOCK;
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
        r_format.format_srgb = RD::DATA_FORMAT_R8G8B8A8_SRGB;
        image->decompress();
        image->convert(Image::FORMAT_RGBA8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_A;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;
    case Image::FORMAT_ASTC_4x4:
    case Image::FORMAT_ASTC_4x4_HDR: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_ASTC_4x4_UNORM_BLOCK,
                                                                     RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_ASTC_4x4_UNORM_BLOCK;
        if (p_image->get_format() == Image::FORMAT_ASTC_4x4) {
          r_format.format_srgb = RD::DATA_FORMAT_ASTC_4x4_SRGB_BLOCK;
        }
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
        r_format.format_srgb = RD::DATA_FORMAT_R8G8B8A8_SRGB;
        image->decompress();
        image->convert(Image::FORMAT_RGBA8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;

    } break;  // astc 4x4
    case Image::FORMAT_ASTC_8x8:
    case Image::FORMAT_ASTC_8x8_HDR: {
      if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_ASTC_8x8_UNORM_BLOCK,
                                                                     RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
        r_format.format = RD::DATA_FORMAT_ASTC_8x8_UNORM_BLOCK;
        if (p_image->get_format() == Image::FORMAT_ASTC_8x8) {
          r_format.format_srgb = RD::DATA_FORMAT_ASTC_8x8_SRGB_BLOCK;
        }
      } else {
        //not supported, reconvert
        r_format.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
        r_format.format_srgb = RD::DATA_FORMAT_R8G8B8A8_SRGB;
        image->decompress();
        image->convert(Image::FORMAT_RGBA8);
      }
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;

    } break;  // astc 8x8

    default: {
    }
  }

  return image;
}
void lain::RendererRD::TextureStorage::_clear_render_target(RenderTarget* rt) {
  // clear overrides, we assume these are freed by the object that created them
  rt->overridden.color = RID();
  rt->overridden.depth = RID();
  rt->overridden.velocity = RID();
  rt->overridden.cached_slices.clear();  // these are automatically freed when their parent textures are freed so just clear

  // free in reverse dependency order
  if (rt->framebuffer_uniform_set.is_valid()) {
    rt->framebuffer_uniform_set = RID();  //chain deleted
  }

  if (rt->color.is_valid()) {
    RD::get_singleton()->free(rt->color);
  }
  rt->color_slices.clear();  // these are automatically freed.

  if (rt->color_multisample.is_valid()) {
    RD::get_singleton()->free(rt->color_multisample);
  }

  if (rt->backbuffer.is_valid()) {
    RD::get_singleton()->free(rt->backbuffer);
    rt->backbuffer = RID();
    rt->backbuffer_mipmaps.clear();
    rt->backbuffer_uniform_set = RID();  //chain deleted
  }

  // _render_target_clear_sdf(rt);

  rt->color = RID();
  rt->color_multisample = RID();
  if (rt->texture.is_valid()) {
    Texture* tex = get_texture(rt->texture);
    tex->render_target = nullptr;
  }
}
void TextureStorage::_update_render_target(RenderTarget* rt) {
  if (rt->texture.is_null()) {
    //create a placeholder until updated
    rt->texture = texture_allocate();
    texture_2d_placeholder_initialize(rt->texture);
    Texture* tex = get_texture(rt->texture);
    tex->is_render_target = true;
    tex->path = "Render Target (Internal)";
  } // 创建默认纹理

  _clear_render_target(rt);
  if (rt->size.width() == 0 || rt->size.height() == 0) {
    return;
  }
	// 下面重建 color， color_multisample， texture （和其中包含的 rd_texture rd_texture_srgb）
  if (rt->use_hdr) {
    // rt->color_format = RendererSceneRenderRD::get_singleton()->_render_buffers_get_color_format();
    rt->color_format = RD::DATA_FORMAT_R16G16B16A16_SFLOAT;
    // mobile 用 DATA_FORMAT_A2B10G10R10_UNORM_PACK32
    rt->color_format_srgb = rt->color_format;
    rt->image_format = rt->is_transparent ? Image::FORMAT_RGBAH : Image::FORMAT_RGBH;
  } else {
    rt->color_format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
    rt->color_format_srgb = RD::DATA_FORMAT_R8G8B8A8_SRGB;
    rt->image_format = rt->is_transparent ? Image::FORMAT_RGBA8 : Image::FORMAT_RGB8;
  }

  RD::TextureFormat rd_color_attachment_format;
  RD::TextureView rd_view;
  {  //attempt register
    rd_color_attachment_format.format = rt->color_format;
    rd_color_attachment_format.width = rt->size.width();
    rd_color_attachment_format.height = rt->size.height();
    rd_color_attachment_format.depth = 1;
    rd_color_attachment_format.array_layers =
        rt->view_count;  // for stereo we create two (or more) layers, need to see if we can make fallback work like this too if we don't have multiview
    rd_color_attachment_format.mipmaps = 1;
    if (rd_color_attachment_format.array_layers > 1) {  // why are we not using rt->texture_type ??
      rd_color_attachment_format.texture_type = RD::TEXTURE_TYPE_2D_ARRAY;
    } else {
      rd_color_attachment_format.texture_type = RD::TEXTURE_TYPE_2D;
    }
    rd_color_attachment_format.samples = RD::TEXTURE_SAMPLES_1;
    rd_color_attachment_format.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;
    rd_color_attachment_format.usage_bits |= RD::TEXTURE_USAGE_STORAGE_BIT;  // FIXME we need this only when FSR is enabled
    rd_color_attachment_format.shareable_formats.push_back(rt->color_format);
    rd_color_attachment_format.shareable_formats.push_back(rt->color_format_srgb);
    if (rt->msaa != RS::VIEWPORT_MSAA_DISABLED) {
      rd_color_attachment_format.is_resolve_buffer = true;
    }
  }

  // TODO see if we can lazy create this once we actually use it as we may not need to create this if we have an overridden color buffer...
  rt->color = RD::get_singleton()->texture_create(rd_color_attachment_format, rd_view);
  ERR_FAIL_COND(rt->color.is_null());

  if (rt->msaa != RS::VIEWPORT_MSAA_DISABLED) {
    // Use the texture format of the color attachment for the multisample color attachment.
    RD::TextureFormat rd_color_multisample_format = rd_color_attachment_format;
    const RD::TextureSamples texture_samples[RS::VIEWPORT_MSAA_MAX] = {
        RD::TEXTURE_SAMPLES_1,
        RD::TEXTURE_SAMPLES_2,
        RD::TEXTURE_SAMPLES_4,
        RD::TEXTURE_SAMPLES_8,
    };
    rd_color_multisample_format.samples = texture_samples[rt->msaa];
    rd_color_multisample_format.usage_bits = RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT;
    RD::TextureView rd_view_multisample;
    rd_color_multisample_format.is_resolve_buffer = false;
    rt->color_multisample = RD::get_singleton()->texture_create(rd_color_multisample_format, rd_view_multisample);
    ERR_FAIL_COND(rt->color_multisample.is_null());
  }

  {  //update texture

    Texture* tex = get_texture(rt->texture);

    //free existing textures
    if (RD::get_singleton()->texture_is_valid(tex->rd_texture)) {
      RD::get_singleton()->free(tex->rd_texture);
    }
    if (RD::get_singleton()->texture_is_valid(tex->rd_texture_srgb)) {
      RD::get_singleton()->free(tex->rd_texture_srgb);
    }

    tex->rd_texture = RID();
    tex->rd_texture_srgb = RID();
    tex->render_target = rt;

    //create shared textures to the color buffer,
    //so transparent can be supported
    RD::TextureView view;
    view.format_override = rt->color_format;
    if (!rt->is_transparent) {
      view.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    }
    tex->rd_texture = RD::get_singleton()->texture_create_shared(view, rt->color);
    if (rt->color_format_srgb != RD::DATA_FORMAT_MAX) {
      view.format_override = rt->color_format_srgb;
      tex->rd_texture_srgb = RD::get_singleton()->texture_create_shared(view, rt->color);
    }
    tex->rd_view = view;
    tex->width = rt->size.width();
    tex->height = rt->size.height();
    tex->width_2d = rt->size.width();
    tex->height_2d = rt->size.height();
    tex->rd_format = rt->color_format;
    tex->rd_format_srgb = rt->color_format_srgb;
    tex->format = rt->image_format;
    tex->validated_format = rt->use_hdr ? Image::FORMAT_RGBAH : Image::FORMAT_RGBA8;
  }
}

void lain::RendererRD::TextureStorage::_create_render_target_backbuffer(RenderTarget* rt) {
		ERR_FAIL_COND(rt->backbuffer.is_valid());

	uint32_t mipmaps_required = Image::get_image_required_mipmaps(rt->size.width(), rt->size.height(), Image::FORMAT_RGBA8);
	RD::TextureFormat tf;
	tf.format = rt->color_format;
	tf.width = rt->size.width();
	tf.height = rt->size.height();
	tf.texture_type = RD::TEXTURE_TYPE_2D;
	tf.usage_bits = RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT;
	tf.mipmaps = mipmaps_required;

	rt->backbuffer = RD::get_singleton()->texture_create(tf, RD::TextureView());
	RD::get_singleton()->set_resource_name(rt->backbuffer, "Render Target Back Buffer");
	rt->backbuffer_mipmap0 = RD::get_singleton()->texture_create_shared_from_slice(RD::TextureView(), rt->backbuffer, 0, 0);
	RD::get_singleton()->set_resource_name(rt->backbuffer_mipmap0, "Back Buffer slice mipmap 0");

	{
		Vector<RID> fb_tex;
		fb_tex.push_back(rt->backbuffer_mipmap0);
		rt->backbuffer_fb = RD::get_singleton()->framebuffer_create(fb_tex);
	}

	if (rt->framebuffer_uniform_set.is_valid() && RD::get_singleton()->uniform_set_is_valid(rt->framebuffer_uniform_set)) {
		//the new one will require the backbuffer.
		RD::get_singleton()->free(rt->framebuffer_uniform_set);
		rt->framebuffer_uniform_set = RID();
	}
	//create mipmaps
	for (uint32_t i = 1; i < mipmaps_required; i++) {
		RID mipmap = RD::get_singleton()->texture_create_shared_from_slice(RD::TextureView(), rt->backbuffer, 0, i);
		RD::get_singleton()->set_resource_name(mipmap, "Back Buffer slice mip: " + itos(i));

		rt->backbuffer_mipmaps.push_back(mipmap);
	}
}

RID lain::RendererRD::TextureStorage::render_target_create() {
	RenderTarget render_target;

	render_target.was_used = false;
	render_target.clear_requested = false;

	_update_render_target(&render_target);
	return render_target_owner.make_rid(render_target);
}

void TextureStorage::render_target_free(RID p_rid) {
	RenderTarget *rt = render_target_owner.get_or_null(p_rid);

	_clear_render_target(rt);

	if (rt->texture.is_valid()) {
		Texture *tex = get_texture(rt->texture);
		tex->is_render_target = false;
		texture_free(rt->texture);
	}

	render_target_owner.free(p_rid);
}

void lain::RendererRD::TextureStorage::render_target_set_position(RID p_render_target, int p_x, int p_y) {}

Point2i lain::RendererRD::TextureStorage::render_target_get_position(RID p_render_target) const {
  return Point2i();
}

void lain::RendererRD::TextureStorage::render_target_set_size(RID p_render_target, int p_width, int p_height, uint32_t p_view_count) {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL(rt);
	if (rt->size.x != p_width || rt->size.y != p_height || rt->view_count != p_view_count) {
		rt->size.x = p_width;
		rt->size.y = p_height;
		rt->view_count = p_view_count;
		_update_render_target(rt);
	}
}


Size2i TextureStorage::render_target_get_size(RID p_render_target) const {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt, Size2i());

	return rt->size;
}

RID TextureStorage::render_target_get_texture(RID p_render_target) {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt, RID());

	return rt->texture;
}

void TextureStorage::render_target_set_override(RID p_render_target, RID p_color_texture, RID p_depth_texture, RID p_velocity_texture) {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL(rt);

	rt->overridden.color = p_color_texture;
	rt->overridden.depth = p_depth_texture;
	rt->overridden.velocity = p_velocity_texture;
}

RID TextureStorage::render_target_get_override_color(RID p_render_target) const {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt, RID());

	return rt->overridden.color;
}

RID TextureStorage::render_target_get_override_depth(RID p_render_target) const {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt, RID());

	return rt->overridden.depth;
}


void TextureStorage::render_target_set_transparent(RID p_render_target, bool p_is_transparent) {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL(rt);
	rt->is_transparent = p_is_transparent;
	_update_render_target(rt);
}

bool TextureStorage::render_target_get_transparent(RID p_render_target) const {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt, false);

	return rt->is_transparent;
}

void TextureStorage::render_target_set_direct_to_screen(RID p_render_target, bool p_value) {
}

bool TextureStorage::render_target_get_direct_to_screen(RID p_render_target) const {
	return false;
}


bool TextureStorage::render_target_was_used(RID p_render_target) const {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt, false);
	return rt->was_used;
}

void TextureStorage::render_target_set_as_unused(RID p_render_target) {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL(rt);
	rt->was_used = false;
}

void TextureStorage::render_target_set_msaa(RID p_render_target, RS::ViewportMSAA p_msaa) {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL(rt);
	if (p_msaa == rt->msaa) {
		return;
	}

	rt->msaa = p_msaa;
	_update_render_target(rt);
}


RS::ViewportMSAA TextureStorage::render_target_get_msaa(RID p_render_target) const {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt, RS::VIEWPORT_MSAA_DISABLED);

	return rt->msaa;
}

void TextureStorage::render_target_set_msaa_needs_resolve(RID p_render_target, bool p_needs_resolve) {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL(rt);

	rt->msaa_needs_resolve = p_needs_resolve;
}


bool TextureStorage::render_target_get_msaa_needs_resolve(RID p_render_target) const {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt, false);

	return rt->msaa_needs_resolve;
}
// 做MSAA resolve
void TextureStorage::render_target_do_msaa_resolve(RID p_render_target) {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL(rt);
	if (!rt->msaa_needs_resolve) {
		return;
	}
	RD::get_singleton()->draw_list_begin(rt->get_framebuffer(), 
	RD::ColorInitialAction(), RD::ColorFinalAction(), RD::INITIAL_ACTION_LOAD, RD::FINAL_ACTION_DISCARD);
	RD::get_singleton()->draw_list_end();
	rt->msaa_needs_resolve = false;
}

void TextureStorage::render_target_set_use_hdr(RID p_render_target, bool p_use_hdr) {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL(rt);

	if (p_use_hdr == rt->use_hdr) {
		return;
	}

	rt->use_hdr = p_use_hdr;
	_update_render_target(rt);
}

bool TextureStorage::render_target_is_using_hdr(RID p_render_target) const {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt, false);

	return rt->use_hdr;
}

RID TextureStorage::render_target_get_override_depth_slice(RID p_render_target, const uint32_t p_layer) const {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt, RID());

	if (rt->overridden.depth.is_null()) {
		return RID();
	} else if (rt->view_count == 1) {
		return rt->overridden.depth;
	} else {
		RenderTarget::RTOverridden::SliceKey key(rt->overridden.depth, p_layer);

		if (!rt->overridden.cached_slices.has(key)) {
			rt->overridden.cached_slices[key] = RD::get_singleton()->texture_create_shared_from_slice(RD::TextureView(), rt->overridden.depth, p_layer, 0);
		}

		return rt->overridden.cached_slices[key];
	}
}

// void lain::RendererRD::TextureStorage::render_target_copy_to_back_buffer(RID p_render_target, const Rect2i& p_region, bool p_gen_mipmaps) {
// 	CopyEffects *copy_effects = CopyEffects::get_singleton();
// 	ERR_FAIL_NULL(copy_effects);

// 	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
// 	ERR_FAIL_NULL(rt);
// 	if (!rt->backbuffer.is_valid()) {
// 		_create_render_target_backbuffer(rt);
// 	}

// 	Rect2i region;
// 	if (p_region == Rect2i()) {
// 		region.size = rt->size;
// 	} else {
// 		region = Rect2i(Size2i(), rt->size).intersection(p_region);
// 		if (region.size == Size2i()) {
// 			return; //nothing to do
// 		}
// 	}

// 	// TODO figure out stereo support here

// 	if (RendererSceneRenderRD::get_singleton()->_render_buffers_can_be_storage()) {
// 		copy_effects->copy_to_rect(rt->color, rt->backbuffer_mipmap0, region, false, false, false, !rt->use_hdr, true);
// 	} else {
// 		Rect2 src_rect = Rect2(region);
// 		src_rect.position /= Size2(rt->size);
// 		src_rect.size /= Size2(rt->size);
// 		copy_effects->copy_to_fb_rect(rt->color, rt->backbuffer_fb, region, false, false, false, false, RID(), false, true, false, false, src_rect);
// 	}

// 	if (!p_gen_mipmaps) {
// 		return;
// 	}
// 	RD::get_singleton()->draw_command_begin_label("Gaussian Blur Mipmaps");
// 	//then mipmap blur
// 	RID prev_texture = rt->color; //use color, not backbuffer, as bb has mipmaps.

// 	Size2i texture_size = rt->size;

// 	for (int i = 0; i < rt->backbuffer_mipmaps.size(); i++) {
// 		region.position.x >>= 1;
// 		region.position.y >>= 1;
// 		region.size = Size2i(region.size.x >> 1, region.size.y >> 1).maxi(1);
// 		texture_size = Size2i(texture_size.x >> 1, texture_size.y >> 1).maxi(1);

// 		RID mipmap = rt->backbuffer_mipmaps[i];
// 		if (RendererSceneRenderRD::get_singleton()->_render_buffers_can_be_storage()) {
// 			copy_effects->gaussian_blur(prev_texture, mipmap, region, texture_size, !rt->use_hdr);
// 		} else {
// 			copy_effects->gaussian_blur_raster(prev_texture, mipmap, region, texture_size);
// 		}
// 		prev_texture = mipmap;
// 	}
// 	RD::get_singleton()->draw_command_end_label();
// }


void lain::RendererRD::TextureStorage::set_max_decals(uint32_t p_max_decals)
{
  	max_decals = p_max_decals;
	uint32_t decal_buffer_size = max_decals * sizeof(DecalData);
	decals = memnew_arr(DecalData, max_decals);
	decal_sort = memnew_arr(DecalInstanceSort, max_decals);
	decal_buffer = RD::get_singleton()->storage_buffer_create(decal_buffer_size);
}

// // 调用texture_update
void lain::RendererRD::TextureStorage::_texture_2d_update(RID p_texture, const Ref<Image>& p_image, int p_layer, bool p_immediate) {
  ERR_FAIL_COND(p_image.is_null() || p_image->is_empty());

  Texture* tex = texture_owner.get_or_null(p_texture);
  ERR_FAIL_NULL(tex);
  ERR_FAIL_COND(tex->is_render_target);
  ERR_FAIL_COND(p_image->get_width() != tex->width || p_image->get_height() != tex->height);
  ERR_FAIL_COND(p_image->get_format() != tex->format);

  if (tex->type == TextureStorage::TYPE_LAYERED) {
    ERR_FAIL_INDEX(p_layer, tex->layers);
  }

#ifdef TOOLS_ENABLED
  tex->image_cache_2d.unref();
#endif
  TextureToRDFormat f;
  Ref<Image> validated = _validate_texture_format(p_image, f);

  RD::get_singleton()->texture_update(tex->rd_texture, p_layer, validated->get_data());
}

void TextureStorage::_texture_format_from_rd(RD::DataFormat p_rd_format, TextureFromRDFormat& r_format) {
  switch (p_rd_format) {
    case RD::DATA_FORMAT_R8_UNORM: {
      r_format.image_format = Image::FORMAT_L8;
      r_format.rd_format = RD::DATA_FORMAT_R8_UNORM;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;  //luminance
    case RD::DATA_FORMAT_R8G8_UNORM: {
      r_format.image_format = Image::FORMAT_LA8;
      r_format.rd_format = RD::DATA_FORMAT_R8G8_UNORM;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_G;
    } break;  //luminance-alpha
    /* already maps to L8/LA8
		case RD::DATA_FORMAT_R8_UNORM: {
			r_format.image_format = Image::FORMAT_R8;
			r_format.rd_format = RD::DATA_FORMAT_R8_UNORM;
			r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
			r_format.swizzle_g = RD::TEXTURE_SWIZZLE_ZERO;
			r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
			r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
		} break;
		case RD::DATA_FORMAT_R8G8_UNORM: {
			r_format.image_format = Image::FORMAT_RG8;
			r_format.rd_format = RD::DATA_FORMAT_R8G8_UNORM;
			r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
			r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
			r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
			r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
		} break;
		*/
    case RD::DATA_FORMAT_R8G8B8_UNORM:
    case RD::DATA_FORMAT_R8G8B8_SRGB: {
      r_format.image_format = Image::FORMAT_RGB8;
      r_format.rd_format = RD::DATA_FORMAT_R8G8B8_UNORM;
      r_format.rd_format_srgb = RD::DATA_FORMAT_R8G8B8_SRGB;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;
    case RD::DATA_FORMAT_R8G8B8A8_UNORM:
    case RD::DATA_FORMAT_R8G8B8A8_SRGB: {
      r_format.image_format = Image::FORMAT_RGBA8;
      r_format.rd_format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
      r_format.rd_format_srgb = RD::DATA_FORMAT_R8G8B8A8_SRGB;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;
    } break;
    case RD::DATA_FORMAT_B4G4R4A4_UNORM_PACK16: {
      r_format.image_format = Image::FORMAT_RGBA4444;
      r_format.rd_format = RD::DATA_FORMAT_B4G4R4A4_UNORM_PACK16;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_B;  //needs swizzle
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;
    } break;
    case RD::DATA_FORMAT_B5G6R5_UNORM_PACK16: {
      r_format.image_format = Image::FORMAT_RGB565;
      r_format.rd_format = RD::DATA_FORMAT_B5G6R5_UNORM_PACK16;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;
    } break;
    case RD::DATA_FORMAT_R32_SFLOAT: {
      r_format.image_format = Image::FORMAT_RF;
      r_format.rd_format = RD::DATA_FORMAT_R32_SFLOAT;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;  //float
    case RD::DATA_FORMAT_R32G32_SFLOAT: {
      r_format.image_format = Image::FORMAT_RGF;
      r_format.rd_format = RD::DATA_FORMAT_R32G32_SFLOAT;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;
    case RD::DATA_FORMAT_R32G32B32_SFLOAT: {
      r_format.image_format = Image::FORMAT_RGBF;
      r_format.rd_format = RD::DATA_FORMAT_R32G32B32_SFLOAT;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;
    case RD::DATA_FORMAT_R32G32B32A32_SFLOAT: {
      r_format.image_format = Image::FORMAT_RGBF;
      r_format.rd_format = RD::DATA_FORMAT_R32G32B32A32_SFLOAT;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;

    } break;
    case RD::DATA_FORMAT_R16_SFLOAT: {
      r_format.image_format = Image::FORMAT_RH;
      r_format.rd_format = RD::DATA_FORMAT_R16_SFLOAT;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;  //half float
    case RD::DATA_FORMAT_R16G16_SFLOAT: {
      r_format.image_format = Image::FORMAT_RGH;
      r_format.rd_format = RD::DATA_FORMAT_R16G16_SFLOAT;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;
    case RD::DATA_FORMAT_R16G16B16_SFLOAT: {
      r_format.image_format = Image::FORMAT_RGBH;
      r_format.rd_format = RD::DATA_FORMAT_R16G16B16_SFLOAT;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;
    case RD::DATA_FORMAT_R16G16B16A16_SFLOAT: {
      r_format.image_format = Image::FORMAT_RGBAH;
      r_format.rd_format = RD::DATA_FORMAT_R16G16B16A16_SFLOAT;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;

    } break;
    case RD::DATA_FORMAT_E5B9G9R9_UFLOAT_PACK32: {
      r_format.image_format = Image::FORMAT_RGBE9995;
      r_format.rd_format = RD::DATA_FORMAT_E5B9G9R9_UFLOAT_PACK32;
      // TODO: Need to make a function in Image to swap bits for this.
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_IDENTITY;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_IDENTITY;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_IDENTITY;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_IDENTITY;
    } break;
    case RD::DATA_FORMAT_BC1_RGB_UNORM_BLOCK:
    case RD::DATA_FORMAT_BC1_RGB_SRGB_BLOCK: {
      r_format.image_format = Image::FORMAT_DXT1;
      r_format.rd_format = RD::DATA_FORMAT_BC1_RGB_UNORM_BLOCK;
      r_format.rd_format_srgb = RD::DATA_FORMAT_BC1_RGB_SRGB_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;  //s3tc bc1
    case RD::DATA_FORMAT_BC2_UNORM_BLOCK:
    case RD::DATA_FORMAT_BC2_SRGB_BLOCK: {
      r_format.image_format = Image::FORMAT_DXT3;
      r_format.rd_format = RD::DATA_FORMAT_BC2_UNORM_BLOCK;
      r_format.rd_format_srgb = RD::DATA_FORMAT_BC2_SRGB_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;

    } break;  //bc2
    case RD::DATA_FORMAT_BC3_UNORM_BLOCK:
    case RD::DATA_FORMAT_BC3_SRGB_BLOCK: {
      r_format.image_format = Image::FORMAT_DXT5;
      r_format.rd_format = RD::DATA_FORMAT_BC3_UNORM_BLOCK;
      r_format.rd_format_srgb = RD::DATA_FORMAT_BC3_SRGB_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;
    } break;  //bc3
    case RD::DATA_FORMAT_BC4_UNORM_BLOCK: {
      r_format.image_format = Image::FORMAT_RGTC_R;
      r_format.rd_format = RD::DATA_FORMAT_BC4_UNORM_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;
    case RD::DATA_FORMAT_BC5_UNORM_BLOCK: {
      r_format.image_format = Image::FORMAT_RGTC_RG;
      r_format.rd_format = RD::DATA_FORMAT_BC5_UNORM_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;
    case RD::DATA_FORMAT_BC7_UNORM_BLOCK:
    case RD::DATA_FORMAT_BC7_SRGB_BLOCK: {
      r_format.image_format = Image::FORMAT_BPTC_RGBA;
      r_format.rd_format = RD::DATA_FORMAT_BC7_UNORM_BLOCK;
      r_format.rd_format_srgb = RD::DATA_FORMAT_BC7_SRGB_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;

    } break;  //btpc bc7
    case RD::DATA_FORMAT_BC6H_SFLOAT_BLOCK: {
      r_format.image_format = Image::FORMAT_BPTC_RGBF;
      r_format.rd_format = RD::DATA_FORMAT_BC6H_SFLOAT_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;  //float bc6h
    case RD::DATA_FORMAT_BC6H_UFLOAT_BLOCK: {
      r_format.image_format = Image::FORMAT_BPTC_RGBFU;
      r_format.rd_format = RD::DATA_FORMAT_BC6H_UFLOAT_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;  //unsigned float bc6hu
    case RD::DATA_FORMAT_EAC_R11_UNORM_BLOCK: {
      r_format.image_format = Image::FORMAT_ETC2_R11;
      r_format.rd_format = RD::DATA_FORMAT_EAC_R11_UNORM_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;  //etc2
    case RD::DATA_FORMAT_EAC_R11_SNORM_BLOCK: {
      r_format.image_format = Image::FORMAT_ETC2_R11S;
      r_format.rd_format = RD::DATA_FORMAT_EAC_R11_SNORM_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;  //signed: {} break; NOT srgb.
    case RD::DATA_FORMAT_EAC_R11G11_UNORM_BLOCK: {
      r_format.image_format = Image::FORMAT_ETC2_RG11;
      r_format.rd_format = RD::DATA_FORMAT_EAC_R11G11_UNORM_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;
    case RD::DATA_FORMAT_EAC_R11G11_SNORM_BLOCK: {
      r_format.image_format = Image::FORMAT_ETC2_RG11S;
      r_format.rd_format = RD::DATA_FORMAT_EAC_R11G11_SNORM_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;
    case RD::DATA_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
    case RD::DATA_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: {
      r_format.image_format = Image::FORMAT_ETC2_RGB8;
      r_format.rd_format = RD::DATA_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
      r_format.rd_format_srgb = RD::DATA_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;

    } break;
    /* already maps to FORMAT_ETC2_RGBA8
		case RD::DATA_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
		case RD::DATA_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: {
			r_format.image_format = Image::FORMAT_ETC2_RGBA8;
			r_format.rd_format = RD::DATA_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
			r_format.rd_format_srgb = RD::DATA_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
			r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
			r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
			r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
			r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;
		} break;
		*/
    case RD::DATA_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
    case RD::DATA_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: {
      r_format.image_format = Image::FORMAT_ETC2_RGB8A1;
      r_format.rd_format = RD::DATA_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
      r_format.rd_format_srgb = RD::DATA_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;
    } break;
    case RD::DATA_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
    case RD::DATA_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: {
      r_format.image_format = Image::FORMAT_ETC2_RA_AS_RG;
      r_format.rd_format = RD::DATA_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
      r_format.rd_format_srgb = RD::DATA_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_A;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
    } break;
    /* already maps to FORMAT_DXT5
		case RD::DATA_FORMAT_BC3_UNORM_BLOCK:
		case RD::DATA_FORMAT_BC3_SRGB_BLOCK: {
			r_format.image_format = Image::FORMAT_DXT5_RA_AS_RG;
			r_format.rd_format = RD::DATA_FORMAT_BC3_UNORM_BLOCK;
			r_format.rd_format_srgb = RD::DATA_FORMAT_BC3_SRGB_BLOCK;
			r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
			r_format.swizzle_g = RD::TEXTURE_SWIZZLE_A;
			r_format.swizzle_b = RD::TEXTURE_SWIZZLE_ZERO;
			r_format.swizzle_a = RD::TEXTURE_SWIZZLE_ONE;
		} break;
		*/
    case RD::DATA_FORMAT_ASTC_4x4_UNORM_BLOCK: {
      // Q: Do we do as we do below, just create the sRGB variant?
      r_format.image_format = Image::FORMAT_ASTC_4x4;
      r_format.rd_format = RD::DATA_FORMAT_ASTC_4x4_UNORM_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;
    } break;
    case RD::DATA_FORMAT_ASTC_4x4_SRGB_BLOCK: {
      r_format.image_format = Image::FORMAT_ASTC_4x4_HDR;
      r_format.rd_format = RD::DATA_FORMAT_ASTC_4x4_UNORM_BLOCK;
      r_format.rd_format_srgb = RD::DATA_FORMAT_ASTC_4x4_SRGB_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;

    } break;  // astc 4x4
    case RD::DATA_FORMAT_ASTC_8x8_UNORM_BLOCK: {
      // Q: Do we do as we do below, just create the sRGB variant?
      r_format.image_format = Image::FORMAT_ASTC_8x8;
      r_format.rd_format = RD::DATA_FORMAT_ASTC_8x8_UNORM_BLOCK;
    } break;
    case RD::DATA_FORMAT_ASTC_8x8_SRGB_BLOCK: {
      r_format.image_format = Image::FORMAT_ASTC_8x8_HDR;
      r_format.rd_format = RD::DATA_FORMAT_ASTC_8x8_UNORM_BLOCK;
      r_format.rd_format_srgb = RD::DATA_FORMAT_ASTC_8x8_SRGB_BLOCK;
      r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
      r_format.swizzle_g = RD::TEXTURE_SWIZZLE_G;
      r_format.swizzle_b = RD::TEXTURE_SWIZZLE_B;
      r_format.swizzle_a = RD::TEXTURE_SWIZZLE_A;

    } break;  // astc 8x8

    default: {
      ERR_FAIL_MSG("Unsupported image format");
    }
  }
}

/// TEXTURE API ///
bool lain::RendererRD::TextureStorage::can_create_resources_async() const {
  return true;
}

RID lain::RendererRD::TextureStorage::texture_allocate() {
  return texture_owner.allocate_rid();
}

void lain::RendererRD::TextureStorage::texture_free(RID p_rid) {
  Texture* t = texture_owner.get_or_null(p_rid);
  ERR_FAIL_NULL(t);
  ERR_FAIL_COND(t->is_render_target);

  t->cleanup();
  // decal_atlas_remove_texture(p_rid); //@todo
  texture_owner.free(p_rid);
}

void TextureStorage::texture_2d_initialize(RID p_texture, const Ref<Image>& p_image) {
  ERR_FAIL_COND(p_image.is_null());

  TextureToRDFormat ret_format;
  Ref<Image> image = _validate_texture_format(p_image, ret_format);

  Texture texture;

  texture.type = TextureStorage::TYPE_2D;

  texture.width = p_image->get_width();
  texture.height = p_image->get_height();
  texture.layers = 1;
  texture.mipmaps = p_image->get_mipmap_count() + 1;
  texture.depth = 1;
  texture.format = p_image->get_format();
  texture.validated_format = image->get_format();

  texture.rd_type = RD::TEXTURE_TYPE_2D;
  texture.rd_format = ret_format.format;
  texture.rd_format_srgb = ret_format.format_srgb;

  RD::TextureFormat rd_format;
  RD::TextureView rd_view;
  {  //attempt register
    rd_format.format = texture.rd_format;
    rd_format.width = texture.width;
    rd_format.height = texture.height;
    rd_format.depth = 1;
    rd_format.array_layers = 1;
    rd_format.mipmaps = texture.mipmaps;
    rd_format.texture_type = texture.rd_type;
    rd_format.samples = RD::TEXTURE_SAMPLES_1;
    rd_format.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;
    if (texture.rd_format_srgb != RD::DATA_FORMAT_MAX) {
      rd_format.shareable_formats.push_back(texture.rd_format);
      rd_format.shareable_formats.push_back(texture.rd_format_srgb);
    }
  }
  {
    rd_view.swizzle_r = ret_format.swizzle_r;
    rd_view.swizzle_g = ret_format.swizzle_g;
    rd_view.swizzle_b = ret_format.swizzle_b;
    rd_view.swizzle_a = ret_format.swizzle_a;
  }
  Vector<uint8_t> data = image->get_data();  //use image data
  Vector<Vector<uint8_t>> data_slices;
  data_slices.push_back(data);
  texture.rd_texture = RD::get_singleton()->texture_create(rd_format, rd_view, data_slices);
  ERR_FAIL_COND(texture.rd_texture.is_null());
  if (texture.rd_format_srgb != RD::DATA_FORMAT_MAX) {
    rd_view.format_override = texture.rd_format_srgb;
    texture.rd_texture_srgb = RD::get_singleton()->texture_create_shared(rd_view, texture.rd_texture);
    if (texture.rd_texture_srgb.is_null()) {  //fall
      RD::get_singleton()->free(texture.rd_texture);
      ERR_FAIL_COND(texture.rd_texture_srgb.is_null());
    }
  }

  //used for 2D, overridable
  texture.width_2d = texture.width;
  texture.height_2d = texture.height;
  texture.is_render_target = false;
  texture.rd_view = rd_view;

  texture_owner.initialize_rid(p_texture, texture);
}

void TextureStorage::texture_2d_layered_initialize(RID p_texture, const Vector<Ref<Image>>& p_layers, RS::TextureLayeredType p_layered_type) {
  ERR_FAIL_COND(p_layers.is_empty());

  ERR_FAIL_COND(p_layered_type == RS::TEXTURE_LAYERED_CUBEMAP && p_layers.size() != 6);
  ERR_FAIL_COND(p_layered_type == RS::TEXTURE_LAYERED_CUBEMAP_ARRAY && (p_layers.size() < 6 || (p_layers.size() % 6) != 0));

  TextureToRDFormat ret_format;
  Vector<Ref<Image>> images;
  {
    int valid_width = 0;
    int valid_height = 0;
    bool valid_mipmaps = false;
    Image::Format valid_format = Image::FORMAT_MAX;

    for (int i = 0; i < p_layers.size(); i++) {
      ERR_FAIL_COND(p_layers[i]->is_empty());

      if (i == 0) {
        valid_width = p_layers[i]->get_width();
        valid_height = p_layers[i]->get_height();
        valid_format = p_layers[i]->get_format();
        valid_mipmaps = p_layers[i]->has_mipmaps();
      } else {
        ERR_FAIL_COND(p_layers[i]->get_width() != valid_width);
        ERR_FAIL_COND(p_layers[i]->get_height() != valid_height);
        ERR_FAIL_COND(p_layers[i]->get_format() != valid_format);
        ERR_FAIL_COND(p_layers[i]->has_mipmaps() != valid_mipmaps);
      }

      images.push_back(_validate_texture_format(p_layers[i], ret_format));
    }
  }

  Texture texture;

  texture.type = TextureStorage::TYPE_LAYERED;
  texture.layered_type = p_layered_type;

  texture.width = p_layers[0]->get_width();
  texture.height = p_layers[0]->get_height();
  texture.layers = p_layers.size();
  texture.mipmaps = p_layers[0]->get_mipmap_count() + 1;
  texture.depth = 1;
  texture.format = p_layers[0]->get_format();
  texture.validated_format = images[0]->get_format();

  switch (p_layered_type) {
    case RS::TEXTURE_LAYERED_2D_ARRAY: {
      texture.rd_type = RD::TEXTURE_TYPE_2D_ARRAY;
    } break;
    case RS::TEXTURE_LAYERED_CUBEMAP: {
      texture.rd_type = RD::TEXTURE_TYPE_CUBE;
    } break;
    case RS::TEXTURE_LAYERED_CUBEMAP_ARRAY: {
      texture.rd_type = RD::TEXTURE_TYPE_CUBE_ARRAY;
    } break;
    default:
      ERR_FAIL();  // Shouldn't happen, silence warnings.
  }

  texture.rd_format = ret_format.format;
  texture.rd_format_srgb = ret_format.format_srgb;

  RD::TextureFormat rd_format;
  RD::TextureView rd_view;
  {  //attempt register
    rd_format.format = texture.rd_format;
    rd_format.width = texture.width;
    rd_format.height = texture.height;
    rd_format.depth = 1;
    rd_format.array_layers = texture.layers;
    rd_format.mipmaps = texture.mipmaps;
    rd_format.texture_type = texture.rd_type;
    rd_format.samples = RD::TEXTURE_SAMPLES_1;
    rd_format.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;
    if (texture.rd_format_srgb != RD::DATA_FORMAT_MAX) {
      rd_format.shareable_formats.push_back(texture.rd_format);
      rd_format.shareable_formats.push_back(texture.rd_format_srgb);
    }
  }
  {
    rd_view.swizzle_r = ret_format.swizzle_r;
    rd_view.swizzle_g = ret_format.swizzle_g;
    rd_view.swizzle_b = ret_format.swizzle_b;
    rd_view.swizzle_a = ret_format.swizzle_a;
  }
  Vector<Vector<uint8_t>> data_slices;
  for (int i = 0; i < images.size(); i++) {
    Vector<uint8_t> data = images[i]->get_data();  //use image data
    data_slices.push_back(data);
  }
  texture.rd_texture = RD::get_singleton()->texture_create(rd_format, rd_view, data_slices);
  ERR_FAIL_COND(texture.rd_texture.is_null());
  if (texture.rd_format_srgb != RD::DATA_FORMAT_MAX) {
    rd_view.format_override = texture.rd_format_srgb;
    texture.rd_texture_srgb = RD::get_singleton()->texture_create_shared(rd_view, texture.rd_texture);
    if (texture.rd_texture_srgb.is_null()) {
      RD::get_singleton()->free(texture.rd_texture);
      ERR_FAIL_COND(texture.rd_texture_srgb.is_null());
    }
  }

  //used for 2D, overridable
  texture.width_2d = texture.width;
  texture.height_2d = texture.height;
  texture.is_render_target = false;
  texture.rd_view = rd_view;

  texture_owner.initialize_rid(p_texture, texture);
}
// 3d texture 仍然是一个大的2d纹理，只是在这里记录偏移
void TextureStorage::texture_3d_initialize(RID p_texture, Image::Format p_format, int p_width, int p_height, int p_depth, bool p_mipmaps, const Vector<Ref<Image>>& p_data) {
  ERR_FAIL_COND(p_data.is_empty());

  Image::Image3DValidateError verr = Image::validate_3d_image(p_format, p_width, p_height, p_depth, p_mipmaps, p_data);
  if (verr != Image::VALIDATE_3D_OK) {
    ERR_FAIL_MSG(Image::get_3d_image_validation_error_text(verr));
  }

  TextureToRDFormat ret_format;
  Image::Format validated_format = Image::FORMAT_MAX;
  Vector<uint8_t> all_data;
  uint32_t mipmap_count = 0;
  Vector<Texture::BufferSlice3D> slices;
  {
    Vector<Ref<Image>> images;
    uint32_t all_data_size = 0;
    images.resize(p_data.size());
    for (int i = 0; i < p_data.size(); i++) {
      TextureToRDFormat f;
      images.write[i] = _validate_texture_format(p_data[i], f);
      if (i == 0) {
        ret_format = f;
        validated_format = images[0]->get_format();
      }

      all_data_size += images[i]->get_data().size();
    }

    all_data.resize(all_data_size);  //consolidate all data here
    uint32_t offset = 0;
    Size2i prev_size;
    for (int i = 0; i < p_data.size(); i++) {
      uint32_t s = images[i]->get_data().size();

      memcpy(&all_data.write[offset], images[i]->get_data().ptr(), s);
      {
        Texture::BufferSlice3D slice;
        slice.size.x = images[i]->get_width();
        slice.size.y = images[i]->get_height();
        slice.offset = offset;
        slice.buffer_size = s;
        slices.push_back(slice);
      }
      offset += s;

      Size2i img_size(images[i]->get_width(), images[i]->get_height());
      if (img_size != prev_size) {
        mipmap_count++;  // 不同说明是mipmap
      }
      prev_size = img_size;
    }
  }

  Texture texture;

  texture.type = TextureStorage::TYPE_3D;
  texture.width = p_width;
  texture.height = p_height;
  texture.depth = p_depth;
  texture.mipmaps = mipmap_count;
  texture.format = p_data[0]->get_format();
  texture.validated_format = validated_format;

  texture.buffer_size_3d = all_data.size();
  texture.buffer_slices_3d = slices;

  texture.rd_type = RD::TEXTURE_TYPE_3D;
  texture.rd_format = ret_format.format;
  texture.rd_format_srgb = ret_format.format_srgb;

  RD::TextureFormat rd_format;
  RD::TextureView rd_view;
  {  //attempt register
    rd_format.format = texture.rd_format;
    rd_format.width = texture.width;
    rd_format.height = texture.height;
    rd_format.depth = texture.depth;
    rd_format.array_layers = 1;
    rd_format.mipmaps = texture.mipmaps;
    rd_format.texture_type = texture.rd_type;  // 3d
    rd_format.samples = RD::TEXTURE_SAMPLES_1;
    rd_format.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;
    if (texture.rd_format_srgb != RD::DATA_FORMAT_MAX) {
      rd_format.shareable_formats.push_back(texture.rd_format);
      rd_format.shareable_formats.push_back(texture.rd_format_srgb);
    }
  }
  {
    rd_view.swizzle_r = ret_format.swizzle_r;
    rd_view.swizzle_g = ret_format.swizzle_g;
    rd_view.swizzle_b = ret_format.swizzle_b;
    rd_view.swizzle_a = ret_format.swizzle_a;
  }
  Vector<Vector<uint8_t>> data_slices;
  data_slices.push_back(all_data);  //one slice

  texture.rd_texture = RD::get_singleton()->texture_create(rd_format, rd_view, data_slices);
  ERR_FAIL_COND(texture.rd_texture.is_null());
  if (texture.rd_format_srgb != RD::DATA_FORMAT_MAX) {
    rd_view.format_override = texture.rd_format_srgb;
    texture.rd_texture_srgb = RD::get_singleton()->texture_create_shared(rd_view, texture.rd_texture);
    if (texture.rd_texture_srgb.is_null()) {
      RD::get_singleton()->free(texture.rd_texture);
      ERR_FAIL_COND(texture.rd_texture_srgb.is_null());
    }
  }

  //used for 2D, overridable
  texture.width_2d = texture.width;
  texture.height_2d = texture.height;
  texture.is_render_target = false;
  texture.rd_view = rd_view;
  texture_owner.initialize_rid(p_texture, texture);
}

void lain::RendererRD::TextureStorage::texture_2d_update(RID p_texture, const Ref<Image>& p_image, int p_layer) {
  return _texture_2d_update(p_texture, p_image, p_layer);
}

void TextureStorage::texture_3d_update(RID p_texture, const Vector<Ref<Image>>& p_data) {
  Texture* tex = texture_owner.get_or_null(p_texture);
  ERR_FAIL_NULL(tex);
  ERR_FAIL_COND(tex->type != TextureStorage::TYPE_3D);

  Image::Image3DValidateError verr = Image::validate_3d_image(tex->format, tex->width, tex->height, tex->depth, tex->mipmaps > 1, p_data);
  if (verr != Image::VALIDATE_3D_OK) {
    ERR_FAIL_MSG(Image::get_3d_image_validation_error_text(verr));
  }

  Vector<uint8_t> all_data;
  {
    Vector<Ref<Image>> images;
    uint32_t all_data_size = 0;
    images.resize(p_data.size());
    for (int i = 0; i < p_data.size(); i++) {
      Ref<Image> image = p_data[i];
      if (image->get_format() != tex->validated_format) {
        image = image->duplicate();
        image->convert(tex->validated_format);
      }
      all_data_size += image->get_data().size();
      images.write[i] = image;
    }

    all_data.resize(all_data_size);  //consolidate all data here
    uint32_t offset = 0;

    for (int i = 0; i < p_data.size(); i++) {
      uint32_t s = images[i]->get_data().size();
      memcpy(&all_data.write[offset], images[i]->get_data().ptr(), s);
      offset += s;
    }
  }

  RD::get_singleton()->texture_update(tex->rd_texture, 0, all_data);
}

Ref<Image> TextureStorage::texture_2d_get(RID p_texture) const {
  Texture* tex = texture_owner.get_or_null(p_texture);
  ERR_FAIL_NULL_V(tex, Ref<Image>());

#ifdef TOOLS_ENABLED
  if (tex->image_cache_2d.is_valid() && !tex->is_render_target) {
    return tex->image_cache_2d;
  }
#endif
  Vector<uint8_t> data = RD::get_singleton()->texture_get_data(tex->rd_texture, 0);
  ERR_FAIL_COND_V(data.is_empty(), Ref<Image>());
  Ref<Image> image;

  // Expand RGB10_A2 into RGBAH. This is needed for capturing viewport data
  // when using the mobile renderer with HDR mode on.
  // 特殊处理RGB10_A2格式
  if (tex->rd_format == RD::DATA_FORMAT_A2B10G10R10_UNORM_PACK32) {
    Vector<uint8_t> new_data;
    new_data.resize(data.size() * 2);
    uint16_t* ndp = (uint16_t*)new_data.ptr();

    uint32_t* ptr = (uint32_t*)data.ptr();
    uint32_t num_pixels = data.size() / 4;

    for (uint32_t ofs = 0; ofs < num_pixels; ofs++) {
      uint32_t px = ptr[ofs];
      uint32_t r = (px & 0x3FF);
      uint32_t g = ((px >> 10) & 0x3FF);
      uint32_t b = ((px >> 20) & 0x3FF);
      uint32_t a = ((px >> 30) & 0x3);

      ndp[ofs * 4 + 0] = Math::make_half_float(float(r) / 1023.0);
      ndp[ofs * 4 + 1] = Math::make_half_float(float(g) / 1023.0);
      ndp[ofs * 4 + 2] = Math::make_half_float(float(b) / 1023.0);
      ndp[ofs * 4 + 3] = Math::make_half_float(float(a) / 3.0);
    }
    image = Image::create_from_data(tex->width, tex->height, tex->mipmaps > 1, tex->validated_format, new_data);
  } else {
    image = Image::create_from_data(tex->width, tex->height, tex->mipmaps > 1, tex->validated_format, data);
  }

  ERR_FAIL_COND_V(image->is_empty(), Ref<Image>());
  if (tex->format != tex->validated_format) {
    image->convert(tex->format);
  }

#ifdef TOOLS_ENABLED
  if (Engine::get_singleton()->is_editor_hint() && !tex->is_render_target) {
    tex->image_cache_2d = image;
  }
#endif

  return image;
}

Ref<Image> lain::RendererRD::TextureStorage::texture_2d_layer_get(RID p_texture, int p_layer) const {
  return Ref<Image>();
}

Vector<Ref<Image>> lain::RendererRD::TextureStorage::texture_3d_get(RID p_texture) const {
  return Vector<Ref<Image>>();
}

void lain::RendererRD::TextureStorage::texture_2d_placeholder_initialize(RID p_texture) {
  //this could be better optimized to reuse an existing image , done this way
  //for now to get it working
  Ref<Image> image = Image::create_empty(4, 4, false, Image::FORMAT_RGBA8);
  image->fill(Color(1, 0, 1, 1));

  texture_2d_initialize(p_texture, image);
}

void TextureStorage::texture_2d_layered_placeholder_initialize(RID p_texture, RS::TextureLayeredType p_layered_type) {
  //this could be better optimized to reuse an existing image , done this way
  //for now to get it working
  Ref<Image> image = Image::create_empty(4, 4, false, Image::FORMAT_RGBA8);
  image->fill(Color(1, 0, 1, 1));

  Vector<Ref<Image>> images;
  if (p_layered_type == RS::TEXTURE_LAYERED_2D_ARRAY) {
    images.push_back(image);
  } else {
    //cube
    for (int i = 0; i < 6; i++) {
      images.push_back(image);
    }
  }

  texture_2d_layered_initialize(p_texture, images, p_layered_type);
}

void TextureStorage::texture_3d_placeholder_initialize(RID p_texture) {
  //this could be better optimized to reuse an existing image , done this way
  //for now to get it working
  Ref<Image> image = Image::create_empty(4, 4, false, Image::FORMAT_RGBA8);
  image->fill(Color(1, 0, 1, 1));

  Vector<Ref<Image>> images;
  //cube
  for (int i = 0; i < 4; i++) {
    images.push_back(image);
  }

  texture_3d_initialize(p_texture, Image::FORMAT_RGBA8, 4, 4, 4, false, images);
}

void lain::RendererRD::TextureStorage::texture_replace(RID p_texture, RID p_by_texture) {
  // @todo
}

void lain::RendererRD::TextureStorage::texture_set_size_override(RID p_texture, int p_width, int p_height) {
  //@todo
}

void lain::RendererRD::TextureStorage::texture_set_path(RID p_texture, const String& p_path) {
  Texture* tex = texture_owner.get_or_null(p_texture);
  ERR_FAIL_NULL(tex);
  tex->path = p_path;
  // 没有callback
}

String lain::RendererRD::TextureStorage::texture_get_path(RID p_texture) const {
  Texture* tex = texture_owner.get_or_null(p_texture);
  ERR_FAIL_NULL_V(tex, String());
  return tex->path;
}

Image::Format lain::RendererRD::TextureStorage::texture_get_format(RID p_texture) const {
  Texture* tex = texture_owner.get_or_null(p_texture);
  ERR_FAIL_NULL_V(tex, Image::FORMAT_MAX);
  return tex->format;
}

RID lain::RendererRD::TextureStorage::RenderTarget::get_framebuffer() {
	// Note that if we're using an overridden color buffer, we're likely cycling through a texture chain.
	// this is where our framebuffer cache comes in clutch..

	if (msaa != RS::VIEWPORT_MSAA_DISABLED) {
		return FramebufferCacheRD::get_singleton()->get_cache_multiview(view_count, color_multisample, overridden.color.is_valid() ? overridden.color : color);
	} else {
		return FramebufferCacheRD::get_singleton()->get_cache_multiview(view_count, overridden.color.is_valid() ? overridden.color : color);
	}
}

void TextureStorage::texture_rd_initialize(RID p_texture, const RID &p_rd_texture, const RS::TextureLayeredType p_layer_type) {
	ERR_FAIL_COND(!RD::get_singleton()->texture_is_valid(p_rd_texture));

	// TODO : investigate if we can support this, will need to be able to obtain the order and obtain the slice info
	ERR_FAIL_COND_MSG(RD::get_singleton()->texture_is_shared(p_rd_texture), "Please create the texture object using the original texture");

	RD::TextureFormat tf = RD::get_singleton()->texture_get_format(p_rd_texture);
	ERR_FAIL_COND(!(tf.usage_bits & RD::TEXTURE_USAGE_SAMPLING_BIT));

	TextureFromRDFormat imfmt;
	_texture_format_from_rd(tf.format, imfmt);
	ERR_FAIL_COND(imfmt.image_format == Image::FORMAT_MAX);

	Texture texture;

	switch (tf.texture_type) {
		case RD::TEXTURE_TYPE_2D: {
			ERR_FAIL_COND(tf.array_layers != 1);
			texture.type = TextureStorage::TYPE_2D;
		} break;
		case RD::TEXTURE_TYPE_2D_ARRAY: {
			// RenderingDevice doesn't distinguish between Array textures and Cube textures
			// this condition covers TextureArrays, TextureCube, and TextureCubeArray.
			ERR_FAIL_COND(tf.array_layers == 1);
			texture.type = TextureStorage::TYPE_LAYERED;
			texture.layered_type = p_layer_type;
		} break;
		case RD::TEXTURE_TYPE_3D: {
			ERR_FAIL_COND(tf.array_layers != 1);
			texture.type = TextureStorage::TYPE_3D;
		} break;
		default: {
			ERR_FAIL_MSG("This RD texture can't be used as a render texture");
		} break;
	}

	texture.width = tf.width;
	texture.height = tf.height;
	texture.depth = tf.depth;
	texture.layers = tf.array_layers;
	texture.mipmaps = tf.mipmaps;
	texture.format = imfmt.image_format;
	texture.validated_format = texture.format; // ??

	RD::TextureView rd_view;
	rd_view.format_override = imfmt.rd_format == tf.format ? RD::DATA_FORMAT_MAX : imfmt.rd_format;
	rd_view.swizzle_r = imfmt.swizzle_r;
	rd_view.swizzle_g = imfmt.swizzle_g;
	rd_view.swizzle_b = imfmt.swizzle_b;
	rd_view.swizzle_a = imfmt.swizzle_a;

	texture.rd_type = tf.texture_type;
	texture.rd_view = rd_view;
	texture.rd_format = imfmt.rd_format;
	// We create a shared texture here even if our view matches, so we don't obtain ownership.
	texture.rd_texture = RD::get_singleton()->texture_create_shared(rd_view, p_rd_texture);
	if (imfmt.rd_format_srgb != RD::DATA_FORMAT_MAX) {
		rd_view.format_override = imfmt.rd_format_srgb == tf.format ? RD::DATA_FORMAT_MAX : imfmt.rd_format;
		texture.rd_format_srgb = imfmt.rd_format_srgb;
		// We create a shared texture here even if our view matches, so we don't obtain ownership.
		texture.rd_texture_srgb = RD::get_singleton()->texture_create_shared(rd_view, p_rd_texture);
	}

	// TODO figure out what to do with slices

	texture.width_2d = texture.width;
	texture.height_2d = texture.height;
	texture.is_render_target = false;

	texture_owner.initialize_rid(p_texture, texture);
}

RID TextureStorage::texture_get_rd_texture(RID p_texture, bool p_srgb) const {
	if (p_texture.is_null()) {
		return RID();
	}

	Texture *tex = texture_owner.get_or_null(p_texture);
	if (!tex) {
		return RID();
	}

	return (p_srgb && tex->rd_texture_srgb.is_valid()) ? tex->rd_texture_srgb : tex->rd_texture;
}

uint64_t TextureStorage::texture_get_native_handle(RID p_texture, bool p_srgb) const {
	Texture *tex = texture_owner.get_or_null(p_texture);
	ERR_FAIL_NULL_V(tex, 0);

	if (p_srgb && tex->rd_texture_srgb.is_valid()) {
		return RD::get_singleton()->get_driver_resource(RD::DRIVER_RESOURCE_TEXTURE, tex->rd_texture_srgb);
	} else {
		return RD::get_singleton()->get_driver_resource(RD::DRIVER_RESOURCE_TEXTURE, tex->rd_texture);
	}
}

RID TextureStorage::render_target_get_override_velocity(RID p_render_target) const {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt, RID());

	return rt->overridden.velocity;
}

RID lain::RendererRD::TextureStorage::render_target_get_rd_texture(RID p_render_target) {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt, RID());

	if (rt->overridden.color.is_valid()) {
		return rt->overridden.color;
	} else {
		return rt->color;
	}
}
RID lain::RendererRD::TextureStorage::render_target_get_rd_texture_msaa(RID p_render_target) {
  RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt, RID());

	return rt->color_multisample;

}


RID TextureStorage::render_target_get_rd_framebuffer(RID p_render_target) {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt, RID());

	return rt->get_framebuffer();
}

RID TextureStorage::render_target_get_override_velocity_slice(RID p_render_target, const uint32_t p_layer) const {
  RenderTarget* rt = render_target_owner.get_or_null(p_render_target);
  ERR_FAIL_NULL_V(rt, RID());

  if (rt->overridden.velocity.is_null()) {
    return RID();
  } else if (rt->view_count == 1) {
    return rt->overridden.velocity;
  } else {
    RenderTarget::RTOverridden::SliceKey key(rt->overridden.velocity, p_layer);

    if (!rt->overridden.cached_slices.has(key)) {
      rt->overridden.cached_slices[key] = RD::get_singleton()->texture_create_shared_from_slice(RD::TextureView(), rt->overridden.velocity, p_layer, 0);
    }
    return rt->overridden.cached_slices[key];
  }
}
void lain::RendererRD::TextureStorage::render_target_request_clear(RID p_render_target, const Color& p_clear_color) {
  RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL(rt);
	rt->clear_requested = true;
	rt->clear_color = p_clear_color;
}

bool lain::RendererRD::TextureStorage::render_target_is_clear_requested(RID p_render_target) {
  RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt, false);
  return rt->clear_requested;
}

Color lain::RendererRD::TextureStorage::render_target_get_clear_request_color(RID p_render_target)
{
  RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL_V(rt,Color());
  return rt->clear_color;
}
void TextureStorage::render_target_disable_clear_request(RID p_render_target) {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL(rt);
	rt->clear_requested = false;
}
void TextureStorage::render_target_do_clear_request(RID p_render_target) {
	RenderTarget *rt = render_target_owner.get_or_null(p_render_target);
	ERR_FAIL_NULL(rt);
	if (!rt->clear_requested) {
		return;
	}
	Vector<Color> clear_colors;
	clear_colors.push_back(rt->use_hdr ? rt->clear_color.srgb_to_linear() : rt->clear_color); // 注意：如果使用hdr，需要转换到线性空间，为什么？ @?
	RD::get_singleton()->draw_list_begin(rt->get_framebuffer(), RD::ColorInitialAction(), RD::ColorFinalAction(), RD::INITIAL_ACTION_LOAD, RD::FINAL_ACTION_DISCARD, clear_colors);
	RD::get_singleton()->draw_list_end();
	rt->clear_requested = false;
	rt->msaa_needs_resolve = false;
}

