#include "image_loader_stb.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
namespace lain {
	StbLoader* StbLoader::singleton = nullptr;

	Error StbLoader::load_image(Ref<Image> p_image, Ref<FileAccess> f, ImageLoader::LoaderFlags p_flags, float p_scale) {
		int width, height, channels;
		const char* p_file = CSTR(f->get_path_absolute());
		ui8* img = stbi_load(p_file, & width, & height, & channels, 0);
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
	Image::Format _get_format(String p_ext, int p_channels) {
		if (p_ext == "jpeg" || p_ext == "jpg") {
			if (p_channels == 1) {
				return Image::FORMAT_L8;
			}
			else if(p_channels == 3) {
				return Image::FORMAT_RGB8;
			}
			ERR_FAIL_COND_V_MSG(false, Image::FORMAT_MAX, "wrong jpeg?");
		}
		else if (p_ext == "bmp") {
			// bmp和png是一样的吗
			switch (p_channels) {
			case 1:
				return Image::FORMAT_L8;
			case 2:
				return Image::FORMAT_LA8;
			case 3:
				return Image::FORMAT_RGB8;
			case 4:
				return Image::FORMAT_RGBA8;
			default:
				ERR_FAIL_COND_V_MSG(false, Image::FORMAT_MAX, "wrong bmp?");
			}
		}
		else if (p_ext == "png") {
			switch (p_channels) {
			case 1:
				return Image::FORMAT_L8;
			case 2:
				return Image::FORMAT_LA8;
			case 3:
				return Image::FORMAT_RGB8;
			case 4:
				return Image::FORMAT_RGBA8;
			default:
				ERR_FAIL_COND_V_MSG(false, Image::FORMAT_MAX, "wrong png?");
			}
		}
	}

}