#pragma once
#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H
#include "core/io/resource_loader.h"
#include "core/io/image.h"
#include "core/io/file_access.h"

namespace lain {

	class ResourceFormatLoaderImage;

	// 干活儿的类
	class ResourceLoaderImage;

	// 抽象出这个，以实现loader_image带Ref<Image> p_image的接口
	// 以及在确定是image的情况下避免调用resource
	class ImageLoader {
		friend class ResourceFormatLoaderImage;
		friend class ResourceLoaderImage;
		enum {
			MAX_LOADERS = 10
		};

		static Ref<ResourceLoaderImage> loader[ImageLoader::MAX_LOADERS];
		static int loader_count;
		static HashMap<String, Vector<int>> ext_to_id;
		static void get_recognized_extensions(List<String>* p_extensions);
	public:
		enum LoaderFlags {
			FLAG_NONE = 0,
			FLAG_FORCE_LINEAR = 1,
			FLAG_CONVERT_COLORS = 2,
		};
		static void add_image_format_loader(Ref<ResourceLoaderImage>& p_loader);
		static void remove_image_format_loader(Ref<ResourceLoaderImage>& p_loader);
		static void remove_all_loaders();

		static Error load_image(const String& p_file, Ref<Image> p_image, Ref<FileAccess> p_custom = Ref<FileAccess>(), LoaderFlags p_flags = ImageLoader::FLAG_NONE, float p_scale = 1.0);
	};


	class ResourceLoaderImage : public RefCounted {
		friend class ResourceFormatLoaderImage;
		friend class ImageLoader;

		virtual Error load_image(Ref<Image> p_image, Ref<FileAccess> f, BitField<ImageLoader::LoaderFlags> p_flags = ImageLoader::FLAG_NONE, float p_scale = 1.0) { return FAILED; }

		virtual void get_recognized_extensions(List<String>* p_extensions) const {}
	};

	class ResourceFormatLoaderImage : public ResourceFormatLoader {
	private:

		static ResourceFormatLoaderImage* singleton;
	public:
		static ResourceFormatLoaderImage* get_singleton() { return singleton; }
		virtual Ref<Resource> load(const String& p_path, const String& p_original_path = "", Error* r_error = nullptr, bool p_use_sub_threads = false, float* r_progress = nullptr, CacheMode p_cache_mode = CACHE_MODE_REUSE) override;
		virtual void get_recognized_extensions(List<String>* p_extensions) const {
			ImageLoader::get_recognized_extensions(p_extensions);
		}
		virtual void get_recognized_resources(List<String>* p_extensions) const override {
			p_extensions->push_back("Image");
		}
		ResourceFormatLoaderImage();
		L_INLINE void add_image_format_loader(Ref < ResourceLoaderImage> p_loader) {
			ImageLoader::add_image_format_loader(p_loader);
		}
		L_INLINE void remove_image_format_loader(Ref < ResourceLoaderImage> p_loader) {
			ImageLoader::remove_image_format_loader(p_loader);
		}
		~ResourceFormatLoaderImage() {
			singleton = nullptr;
			ImageLoader::remove_all_loaders();
		}

	};
	
}
#endif 