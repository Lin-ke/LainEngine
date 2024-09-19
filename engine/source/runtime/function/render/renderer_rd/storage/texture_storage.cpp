#include "texture_storage.h"
using namespace lain::RendererRD;
using namespace lain;
TextureStorage::TextureStorage() {
  singleton = this;
}

TextureStorage::~TextureStorage() {
  singleton = nullptr;
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
		} break; //luminance
		case Image::FORMAT_LA8: {
			r_format.format = RD::DATA_FORMAT_R8G8_UNORM;
			r_format.swizzle_r = RD::TEXTURE_SWIZZLE_R;
			r_format.swizzle_g = RD::TEXTURE_SWIZZLE_R;
			r_format.swizzle_b = RD::TEXTURE_SWIZZLE_R;
			r_format.swizzle_a = RD::TEXTURE_SWIZZLE_G;
		} break; //luminance-alpha
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
			if (false && RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_R8G8B8_UNORM, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT) && RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_R8G8B8_SRGB, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
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
			r_format.swizzle_r = RD::TEXTURE_SWIZZLE_B; //needs swizzle
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
		} break; //float
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
				r_format.format = RD::DATA_FORMAT_R32G32B32_SFLOAT;
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

		} break; //half float
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

		} break; //s3tc bc1
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

		} break; //bc2
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
		} break; //bc3
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

		} break; //btpc bc7
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
		} break; //float bc6h
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
		} break; //unsigned float bc6hu
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

		} break; //etc2
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
		} break; //signed: {} break; NOT srgb.
		case Image::FORMAT_ETC2_RG11: {
			if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_EAC_R11G11_UNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
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
			if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_EAC_R11G11_SNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
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
			if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
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
			if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
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
			if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
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
			if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
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
			if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_ASTC_4x4_UNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
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

		} break; // astc 4x4
		case Image::FORMAT_ASTC_8x8:
		case Image::FORMAT_ASTC_8x8_HDR: {
			if (RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_ASTC_8x8_UNORM_BLOCK, RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT)) {
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

		} break; // astc 8x8

		default: {
		}
	}

	return image;
}

void lain::RendererRD::TextureStorage::_texture_2d_update(RID p_texture, const Ref<Image>& p_image, int p_layer, bool p_immediate) {
  	ERR_FAIL_COND(p_image.is_null() || p_image->is_empty());

	Texture *tex = texture_owner.get_or_null(p_texture);
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
  Texture *t = texture_owner.get_or_null(p_rid);
  ERR_FAIL_NULL(t);
	ERR_FAIL_COND(t->is_render_target);

  t->cleanup();
	// decal_atlas_remove_texture(p_rid); //@todo
	texture_owner.free(p_rid);
}

void TextureStorage::texture_2d_initialize(RID p_texture, const Ref<Image> &p_image) {
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
	{ //attempt register
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
	Vector<uint8_t> data = image->get_data(); //use image data
	Vector<Vector<uint8_t>> data_slices;
	data_slices.push_back(data);
	texture.rd_texture = RD::get_singleton()->texture_create(rd_format, rd_view, data_slices);
	ERR_FAIL_COND(texture.rd_texture.is_null());
	if (texture.rd_format_srgb != RD::DATA_FORMAT_MAX) {
		rd_view.format_override = texture.rd_format_srgb;
		texture.rd_texture_srgb = RD::get_singleton()->texture_create_shared(rd_view, texture.rd_texture);
		if (texture.rd_texture_srgb.is_null()) { //fall
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

void TextureStorage::texture_2d_layered_initialize(RID p_texture, const Vector<Ref<Image>> &p_layers, RS::TextureLayeredType p_layered_type) {
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
			ERR_FAIL(); // Shouldn't happen, silence warnings.
	}

	texture.rd_format = ret_format.format;
	texture.rd_format_srgb = ret_format.format_srgb;

	RD::TextureFormat rd_format;
	RD::TextureView rd_view;
	{ //attempt register
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
		Vector<uint8_t> data = images[i]->get_data(); //use image data
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
void TextureStorage::texture_3d_initialize(RID p_texture, Image::Format p_format, int p_width, int p_height, int p_depth, bool p_mipmaps, const Vector<Ref<Image>> &p_data) {
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

		all_data.resize(all_data_size); //consolidate all data here
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
				mipmap_count++; // 不同说明是mipmap
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
	{ //attempt register
		rd_format.format = texture.rd_format;
		rd_format.width = texture.width;
		rd_format.height = texture.height;
		rd_format.depth = texture.depth;
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
	Vector<Vector<uint8_t>> data_slices;
	data_slices.push_back(all_data); //one slice

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
