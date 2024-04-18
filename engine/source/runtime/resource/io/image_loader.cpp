#include "image_loader.h"
namespace lain {

	Ref<Resource> ResourceFormatLoaderImage::load(const String& p_path, const String& p_original_path, Error* r_error, bool p_use_sub_threads, float* r_progress, CacheMode p_cache_mode) {
		Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ);
		if (f.is_null()) {
			if (r_error) {
				*r_error = ERR_CANT_OPEN;
			}
			return Ref<Resource>();
		}
		String ext = p_path.get_extension();
		ERR_FAIL_COND_V(ImageLoader::ext_to_id[ext].is_empty(), (*r_error = ERR_FILE_UNRECOGNIZED, Ref<Resource>()));
		Ref<Image> image;
		image.instantiate();
		Error err;
		for (int i : ImageLoader::ext_to_id[ext]) {
			err = ImageLoader::loader[i]->load_image(image, f);
			if (err != OK) continue;
		}
		
		ERR_FAIL_COND_V(err != OK, (*r_error = err, Ref<Resource>()));
		return image;
	}

	ResourceFormatLoaderImage* ResourceFormatLoaderImage::singleton = nullptr;

	Ref<ResourceLoaderImage> ImageLoader::loader[ImageLoader::MAX_LOADERS];
	HashMap<String, Vector<int>> ImageLoader::ext_to_id;

	void ImageLoader::add_image_format_loader(Ref<ResourceLoaderImage>& p_loader) {
		ERR_FAIL_COND(p_loader.is_null());

		int i;
		for (i = 0; i < loader_count; i++) {
			if (loader[i] == p_loader) {
				// ÒÆ¶¯
				break;
			}
		}
		ERR_FAIL_COND( i == loader_count);
		for (; i < loader_count - 1; i++) {
			loader[i] = loader[i+1];
		}
		List<String> exts;
		p_loader->get_possible_extensions(&exts);

		for (const String& ext : exts) {
			Vector<int>& idxs = ext_to_id[ext];
			idxs.erase(i);
			if (idxs.size() == 0) {
				ERR_FAIL_COND(!ext_to_id.erase(ext)); // delete failed
			}
		}
		loader_count--;
	}

	Error ImageLoader::load_image(const String& p_file, Ref<Image> p_image, Ref<FileAccess> p_custom, LoaderFlags p_flags, float p_scale) {
		ERR_FAIL_COND_V_MSG(p_image.is_null(), ERR_INVALID_PARAMETER, "Can't load an image: invalid Image object.");

		Ref<FileAccess> f = p_custom;
		if (f.is_null()) {
			Error err;
			f = FileAccess::open(p_file, FileAccess::READ, &err);
			ERR_FAIL_COND_V_MSG(f.is_null(), err, "Error opening file '" + p_file + "'.");
		}

		String ext = p_file.get_extension();
		ERR_FAIL_COND_V(ext_to_id[ext].is_empty(), ERR_FILE_UNRECOGNIZED);
		Error err;
		for (int i : ImageLoader::ext_to_id[ext]) {
			err = ImageLoader::loader[i]->load_image(p_image, f, p_flags, p_scale);
			if (err != OK) continue;
			else return OK;
		}

		return ERR_FILE_UNRECOGNIZED;
	}

	void ImageLoader::get_possible_extensions(List<String>* p_extensions) {
			for (int i = 0; i < loader_count; ++i) {
				loader[i]->get_possible_extensions(p_extensions);
			}
		}
}