#pragma once
#ifndef RESOURCER_LOADER_H
#define RESOURCER_LOADER_H
#include "base.h"
#include "core/object/refcounted.h"
#include "core/io/resource.h"
namespace lain {
	class ResourceFormatLoader : public RefCounted {
	public:
		enum CacheMode { // 如何使用cache
			CACHE_MODE_IGNORE,
			CACHE_MODE_REUSE,
			CACHE_MODE_REPLACE,
			CACHE_MODE_IGNORE_DEEP,
			CACHE_MODE_REPLACE_DEEP,
		};
		virtual Ref<Resource> load(const String& p_path, const String& p_original_path = "", Error* r_error = nullptr, bool p_use_sub_threads = false, float* r_progress = nullptr, CacheMode p_cache_mode = CACHE_MODE_REUSE); // load
		virtual bool exists(const String& p_path) const;
		virtual bool recognize_path(const String& p_path, const String& p_for_type = String()) const;
		virtual bool handles_type(const String& p_type) const;


	};
	class ResourceLoader {
		friend class ResourceFormatImporter;

		enum {
			MAX_LOADERS = 64
		};

	public:
		enum ThreadLoadStatus {
			THREAD_LOAD_INVALID_RESOURCE,
			THREAD_LOAD_IN_PROGRESS,
			THREAD_LOAD_FAILED,
			THREAD_LOAD_LOADED
		};

		enum LoadThreadMode {
			LOAD_THREAD_FROM_CURRENT,
			LOAD_THREAD_SPAWN_SINGLE,
			LOAD_THREAD_DISTRIBUTE,
		};

		struct LoadToken : public RefCounted {
			String local_path;
			String user_path;
			Ref<Resource> res_if_unregistered;

			void clear();

			virtual ~LoadToken();
		};
		static Ref<LoadToken> _load_start(const String& p_path, const String& p_type_hint, LoadThreadMode p_thread_mode, ResourceFormatLoader::CacheMode p_cache_mode);
		static Ref<Resource> _load_complete(LoadToken& p_load_token, Error* r_error);
	private:
		static Ref<ResourceFormatLoader> loader[MAX_LOADERS];
		static int loader_count;
		static bool timestamp_on_load;
		struct ThreadLoadTask {
		};


	};
}

#endif // !RESOURCER_LOADER_H
