#pragma once
#ifndef RESOURCER_LOADER_H
#define RESOURCER_LOADER_H
#include "base.h"
#include "core/object/refcounted.h"
#include "core/io/resource.h"
#include "core/thread/worker_thread_pool.h"
#include "core/templates/hash_map.h"
namespace lain {
	class ResourceFormatLoader : public RefCounted {
		LCLASS(ResourceFormatLoader, RefCounted);

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
		virtual bool handles_type(const String& p_type) const;


	};
	typedef void (*ResourceLoadedCallback)(Ref<Resource> p_resource, const String& p_path);
	typedef void (*ResourceLoadErrorNotify)(const String& p_text);

	class ResourceLoader {
		friend class ResourceFormatImporter;
		friend void register_core_types();
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

		static const int BINARY_MUTEX_TAG = 1;
		static Ref<Resource> load(const String& p_path, const String& p_type_hint = "", ResourceFormatLoader::CacheMode p_cache_mode = ResourceFormatLoader::CACHE_MODE_REUSE, Error* r_error = nullptr);
		static String path_remap(const String& p_path);

		static void initialize();
		static void finalize();
	private:
		static Ref<Resource> _load_complete_inner(LoadToken& p_load_token, Error* r_error, MutexLock<SafeBinaryMutex<BINARY_MUTEX_TAG>>& p_thread_load_lock);

		static Ref<ResourceFormatLoader> loader[MAX_LOADERS];
		static int loader_count;
		static bool timestamp_on_load;
		struct ThreadLoadTask{
		WorkerThreadPool::TaskID task_id = 0; // Used if run on a worker thread from the pool.
		Thread::ID thread_id = 0; // Used if running on an user thread (e.g., simple non-threaded load).
		bool awaited = false; // If it's in the pool, this helps not awaiting from more than one dependent thread.
		ConditionVariable* cond_var = nullptr; // In not in the worker pool or already awaiting, this is used as a secondary awaiting mechanism.
		LoadToken* load_token = nullptr;
		String local_path;
		String remapped_path;
		String dependent_path;
		String type_hint;
		float progress = 0.0f;
		float max_reported_progress = 0.0f;
		ThreadLoadStatus status = THREAD_LOAD_IN_PROGRESS;
		ResourceFormatLoader::CacheMode cache_mode = ResourceFormatLoader::CACHE_MODE_REUSE;
		Error error = OK;
		Ref<Resource> resource;
		bool xl_remapped = false;
		bool use_sub_threads = false;
		HashSet<String> sub_tasks;
		};

		static void _thread_load_function(void* p_userdata);
		static ResourceLoadedCallback _loaded_callback; // 回调
		static String _path_remap(const String& p_path, bool* r_translation_remapped = nullptr);


		static Ref<Resource> _load(const String& p_path, const String& p_original_path, const String& p_type_hint, ResourceFormatLoader::CacheMode p_cache_mode, Error* r_error, bool p_use_sub_threads, float* r_progress);
		// static 类型 is implied 
		static thread_local int load_nesting;
		static thread_local WorkerThreadPool::TaskID caller_task_id;
		static thread_local Vector<String>* load_paths_stack; // A pointer to avoid broken TLS implementations from double-running the destructor.
		static SafeBinaryMutex<BINARY_MUTEX_TAG> thread_load_mutex;
		static HashMap<String, ThreadLoadTask> thread_load_tasks;
		static bool cleaning_tasks; // ?

		static HashMap<String, ResourceLoader::LoadToken*> user_load_tokens;

	};
}

#endif // !RESOURCER_LOADER_H
