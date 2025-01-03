#include "image_loader_stb.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
namespace lain {
	StbLoader* StbLoader::singleton = nullptr;

	Error StbLoader::load_image(Ref<Image> p_image, Ref<FileAccess> f, BitField<ImageLoader::LoaderFlags> p_flags, float p_scale) {
		int width, height, channels;
		ui8* img = stbi_load(CSTR(f->get_path_absolute()), & width, & height, & channels, 0);
		ERR_FAIL_NULL_V_MSG(img, ERR_FILE_CORRUPT, "stbi_load failed");
		Image::Format fmt;
		Vector<ui8> data;
		data.resize(height * width * channels);
		uint8_t* dw = data.ptrw();

		fmt = _get_format(f->get_path_absolute().get_extension(), channels);

		p_image->set_data(width, height, false, fmt, data);
		stbi_image_free(img);
		return OK;
	}
	Image::Format StbLoader::_get_format(String p_ext, int p_channels) {
		Image::Format format = Image::FORMAT_MAX;
		if (p_ext == "jpeg" || p_ext == "jpg") {
			if (p_channels == 1) {
				format = Image::FORMAT_L8;
			}
			else if(p_channels == 3) {
				format = Image::FORMAT_RGB8;
			}
			ERR_FAIL_COND_V_MSG(false, Image::FORMAT_MAX, "wrong jpeg?");
		}
		else if (p_ext == "bmp" || p_ext == "png" || p_ext == "hdr") {
			switch (p_channels) {
			case 1:
				format = Image::FORMAT_L8;
				break;
			case 2:
				format = Image::FORMAT_LA8;
				break;
			
			case 3:
				format = Image::FORMAT_RGB8;
				break;
			
			case 4:
				format = Image::FORMAT_RGBA8;
				break;
			
			default:
				ERR_FAIL_V_MSG( Image::FORMAT_MAX, "wrong image?");
			}
		}
		else {
			ERR_FAIL_V_MSG( Image::FORMAT_MAX, "unknown format");
		}
		return format;
	}

}