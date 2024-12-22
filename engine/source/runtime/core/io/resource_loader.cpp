#include "resource_loader.h"
#include "core/os/mutex.h"
#include "resource_uid.h"
namespace lain {
Ref<ResourceFormatLoader> ResourceLoader::loader[ResourceLoader::MAX_LOADERS];
int ResourceLoader::loader_count = 0;
thread_local int ResourceLoader::load_nesting = 0;
thread_local WorkerThreadPool::TaskID ResourceLoader::caller_task_id = 0;
thread_local Vector<String>* ResourceLoader::load_paths_stack;
HashMap<String, ResourceLoader::ThreadLoadTask> ResourceLoader::thread_load_tasks;
HashMap<String, ResourceLoader::LoadToken*> ResourceLoader::user_load_tokens;
bool ResourceLoader::cleaning_tasks = false;
ResourceLoadedCallback ResourceLoader::_loaded_callback = nullptr;  // 回调

//
bool ResourceLoader::create_missing_resources_if_class_unavailable = false;
bool ResourceLoader::abort_on_missing_resource = true;
bool ResourceLoader::timestamp_on_load = false;

template <>
thread_local uint32_t SafeBinaryMutex<ResourceLoader::BINARY_MUTEX_TAG>::count = 0;
SafeBinaryMutex<ResourceLoader::BINARY_MUTEX_TAG> ResourceLoader::thread_load_mutex;
// HashMap
HashMap<String, Vector<int>> ResourceLoader::ext_to_loader_idx; // 根据extension找到loader ，再进一步
HashMap<String, Vector<int>> ResourceLoader::type_to_loader_idx;

void ResourceLoader::initialize() {}

void ResourceLoader::finalize() {}

void ResourceLoader::add_resource_format_loader(Ref<ResourceFormatLoader> p_format_loader, bool p_at_front) {
  ERR_FAIL_COND(p_format_loader.is_null());
  ERR_FAIL_COND(loader_count >= MAX_LOADERS);
  int saved_idx = 0;
  if (p_at_front) {
    for (int i = loader_count; i > 0; i--) {
      loader[i] = loader[i - 1];
    }
    loader[0] = p_format_loader;
    loader_count++;
  } else {
    saved_idx = loader_count;
    loader[loader_count++] = p_format_loader;
  }
  List<String> exts;
  p_format_loader->get_recognized_extensions(&exts);
  List<String> res_types;

  p_format_loader->get_recognized_resources(&res_types);

  for (const String& ext : exts) {
    Vector<int>& idxs = ext_to_loader_idx[ext];
    idxs.append(saved_idx);
  }
  for (const String& res_type : res_types) {
    Vector<int>& idxs = type_to_loader_idx[res_type];
    idxs.append(saved_idx);
  }
}
// 用数组存，但是和链表是一样的用，xs

void ResourceLoader::remove_resource_format_loader(Ref<ResourceFormatLoader> p_format_loader) {
  ERR_FAIL_COND(p_format_loader.is_null());

  // Find loader
  int i = 0;
  for (; i < loader_count; ++i) {
    if (loader[i] == p_format_loader) {
      break;
    }
  }

  ERR_FAIL_COND(i >= loader_count);  // Not found

  List<String> exts;
  p_format_loader->get_recognized_extensions(&exts);
  for (const String& ext : exts) {
    Vector<int>& idxs = type_to_loader_idx[ext];
    idxs.erase(i);
    if (idxs.size() == 0) {
      ERR_FAIL_COND(!type_to_loader_idx.erase(ext));  // delete failed
    }
  }

  exts.clear();
  p_format_loader->get_recognized_resources(&exts);
  for (const String& ext : exts) {
    Vector<int>& idxs = ext_to_loader_idx[ext];
    idxs.erase(i);
    if (idxs.size() == 0) {
      ERR_FAIL_COND(!ext_to_loader_idx.erase(ext));  // delete failed
    }
  }
  // Shift next loaders up
  for (; i < loader_count - 1; ++i) {
    loader[i] = loader[i + 1];
  }
  loader[loader_count - 1].unref();
  --loader_count;
}

Ref<Resource> ResourceLoader::load(const String& p_path, const String& p_type_hint, ResourceFormatLoader::CacheMode p_cache_mode, Error* r_error) {
  if (r_error) {
    *r_error = OK;
  }

  Ref<LoadToken> load_token = _load_start(p_path, p_type_hint, LOAD_THREAD_FROM_CURRENT, p_cache_mode);
  if (!load_token.is_valid()) {
    if (r_error) {
      *r_error = FAILED;
    }
    return Ref<Resource>();
  }

  Ref<Resource> res = _load_complete(*load_token.ptr(), r_error);
  return res;
}
// 如果缓存过，直接转换
static String _validate_local_path(const String& p_path) {
  // 转换为项目路径路径
  ResourceUID::ID uid = ResourceUID::get_singleton()->text_to_id(p_path);
  if (uid != ResourceUID::INVALID_ID) {
    return ResourceUID::get_singleton()->get_id_path(uid);
  } else if (p_path.is_relative_path()) {
    return "res://" + p_path;
  } else {
    return ProjectSettings::GetSingleton()->LocalizePath(p_path);
  }
}
String ResourceLoader::path_remap(const String& p_path) {
  return _path_remap(p_path);
}
String ResourceLoader::_path_remap(const String& p_path, bool* r_translation_remapped) {
  return p_path;
}

Ref<ResourceLoader::LoadToken> ResourceLoader::_load_start(const String& p_path, const String& p_type_hint, LoadThreadMode p_thread_mode,
                                                           ResourceFormatLoader::CacheMode p_cache_mode) {
  String local_path = _validate_local_path(p_path);

  Ref<LoadToken> load_token;
  bool must_not_register = false;
  ThreadLoadTask unregistered_load_task;  // Once set, must be valid up to the call to do the load.
  ThreadLoadTask* load_task_ptr = nullptr;
  bool run_on_current_thread = false;
  {
    MutexLock thread_load_lock(thread_load_mutex);

    if (thread_load_tasks.has(local_path)) {
      load_token = Ref<LoadToken>(thread_load_tasks[local_path].load_token);
      if (!load_token.is_valid()) {
        // The token is dying (reached 0 on another thread).
        // Ensure it's killed now so the path can be safely reused right away.
        thread_load_tasks[local_path].load_token->clear();
      } else {
        if (p_cache_mode != ResourceFormatLoader::CACHE_MODE_IGNORE) {
          return load_token;  // 找到
        }
      }
    }

    load_token.instantiate();  // memnew
    load_token->local_path = local_path;

    //create load task
    {
      ThreadLoadTask load_task;

      //load_task.remapped_path = _path_remap(local_path, &load_task.xl_remapped);
      // @TODO translate remapped_path
      load_task.remapped_path = local_path;
      load_task.load_token = load_token.ptr();
      load_task.local_path = local_path;
      load_task.type_hint = p_type_hint;
      load_task.cache_mode = p_cache_mode;
      load_task.use_sub_threads = p_thread_mode == LOAD_THREAD_DISTRIBUTE;
      if (p_cache_mode == ResourceFormatLoader::CACHE_MODE_REUSE) {
        Ref<Resource> existing = ResourceCache::get_ref(local_path);
        if (existing.is_valid()) {
          //referencing is fine
          load_task.resource = existing;
          load_task.status = THREAD_LOAD_LOADED;
          load_task.progress = 1.0;
          thread_load_tasks[local_path] = load_task;
          return load_token;
        }
      }

      // If we want to ignore cache, but there's another task loading it, we can't add this one to the map and we also have to finish unconditionally synchronously.
      must_not_register = thread_load_tasks.has(local_path) && p_cache_mode == ResourceFormatLoader::CACHE_MODE_IGNORE;
      if (must_not_register) {
        load_token->local_path.clear();
        unregistered_load_task = load_task;
      } else {
        thread_load_tasks[local_path] = load_task;
      }
      // 放到hashmap里（内存存着）再传给thread
      load_task_ptr = must_not_register ? &unregistered_load_task : &thread_load_tasks[local_path];
    }
    // 在这个线程是tag或不注册
    run_on_current_thread = must_not_register || p_thread_mode == LOAD_THREAD_FROM_CURRENT;

    if (run_on_current_thread) {
      load_task_ptr->thread_id = Thread::get_caller_id();
    } else {
      load_task_ptr->task_id = WorkerThreadPool::get_singleton()->add_native_task(&ResourceLoader::_thread_load_function, load_task_ptr);
    }
  }

  if (run_on_current_thread) {
    _thread_load_function(load_task_ptr);
    if (must_not_register) {
      load_token->res_if_unregistered = load_task_ptr->resource;
    }
  }

  return load_token;
}

void ResourceLoader::_thread_load_function(void* p_userdata) {
  ThreadLoadTask& load_task = *(ThreadLoadTask*)p_userdata;

  thread_load_mutex.lock();
  caller_task_id = load_task.task_id;
  if (cleaning_tasks) {
    load_task.status = THREAD_LOAD_FAILED;
    thread_load_mutex.unlock();
    return;
  }
  thread_load_mutex.unlock();

  // Thread-safe either if it's the current thread or a brand new one.
  //CallQueue* mq_override = nullptr;
  if (load_nesting == 0) {
    load_paths_stack = memnew(Vector<String>);

    if (!load_task.dependent_path.is_empty()) {
      load_paths_stack->push_back(load_task.dependent_path);
    }
    if (!Thread::is_main_thread()) {
      //mq_override = memnew(CallQueue);
      //MessageQueue::set_thread_singleton_override(mq_override);
      //set_current_thread_safe_for_nodes(true);
    }
  } else {
    DEV_ASSERT(load_task.dependent_path.is_empty());
  }
  // --

  if (!Thread::is_main_thread()) {
    set_current_thread_safe_for_nodes(true);
  }
  // @TODO : remapped path;
  Ref<Resource> res = _load(load_task.remapped_path, load_task.remapped_path != load_task.local_path ? load_task.local_path : String(), load_task.type_hint,
                            load_task.cache_mode, &load_task.error, load_task.use_sub_threads, &load_task.progress);
  //if (mq_override) {
  //mq_override->flush();
  //}

  thread_load_mutex.lock();

  load_task.resource = res;

  load_task.progress = 1.0;  //it was fully loaded at this point, so force progress to 1.0
  if (load_task.error != OK) {
    load_task.status = THREAD_LOAD_FAILED;
  } else {
    load_task.status = THREAD_LOAD_LOADED;
  }

  if (load_task.cond_var) {
    load_task.cond_var->notify_all();
    memdelete(load_task.cond_var);
    load_task.cond_var = nullptr;
  }
  // 更新
  bool ignoring = load_task.cache_mode == ResourceFormatLoader::CACHE_MODE_IGNORE || load_task.cache_mode == ResourceFormatLoader::CACHE_MODE_IGNORE_DEEP;
  bool replacing = load_task.cache_mode == ResourceFormatLoader::CACHE_MODE_REPLACE || load_task.cache_mode == ResourceFormatLoader::CACHE_MODE_REPLACE_DEEP;
  if (load_task.resource.is_valid()) {
    if (!ignoring) {
      if (replacing) {  // 更新缓存
        Ref<Resource> old_res = ResourceCache::get_ref(load_task.local_path);
        if (old_res.is_valid() && old_res != load_task.resource) {
          // If resource is already loaded, only replace its data, to avoid existing invalidating instances.
          old_res->CopyFrom(load_task.resource);
          load_task.resource = old_res;
        }
      }
      load_task.resource->set_path(load_task.local_path, replacing);
    } else {
      load_task.resource->set_pathCache(load_task.local_path);  //只设置该资源
    }

    /*if (load_task.xl_remapped) {
				load_task.resource->set_as_translation_remapped(true);
			}*/

#ifdef TOOLS_ENABLED
    load_task.resource->set_edited(false);
    if (timestamp_on_load) {
      uint64_t mt = FileAccess::get_modified_time(load_task.remapped_path);
      //printf("mt %s: %lli\n",remapped_path.utf8().get_data(),mt);
      load_task.resource->set_last_modified_time(mt);
    }
#endif

    if (_loaded_callback) {
      _loaded_callback(load_task.resource, load_task.local_path);
    }
  } else if (!ignoring) {
    Ref<Resource> existing = ResourceCache::get_ref(load_task.local_path);
    if (existing.is_valid()) {
      load_task.resource = existing;
      load_task.status = THREAD_LOAD_LOADED;
      load_task.progress = 1.0;

      if (_loaded_callback) {
        _loaded_callback(load_task.resource, load_task.local_path);
      }
    }
  }

  thread_load_mutex.unlock();

  if (load_nesting == 0) {
    /*if (mq_override) {
				memdelete(mq_override);
			}*/
    memdelete(load_paths_stack);
  }
}

Ref<Resource> ResourceLoader::_load(const String& p_path, const String& p_original_path, const String& p_type_hint, ResourceFormatLoader::CacheMode p_cache_mode,
                                    Error* r_error, bool p_use_sub_threads, float* r_progress) {
  load_nesting++;
  if (load_paths_stack->size()) {
    thread_load_mutex.lock();
    HashMap<String, ThreadLoadTask>::Iterator E = thread_load_tasks.find(load_paths_stack->get(load_paths_stack->size() - 1));
    if (E) {
      E->value.sub_tasks.insert(p_original_path);
    }
    thread_load_mutex.unlock();
  }
  load_paths_stack->push_back(p_original_path);

  // Try all loaders and pick the first match for the type hint

  bool found = false;
  Ref<Resource> res;
  String extension = p_path.get_extension();
  Vector<int> type_loaders;

  if (ext_to_loader_idx.has(extension)) {
    for (int idx : ext_to_loader_idx[extension]) {
      if(!p_type_hint.is_empty()){
        type_loaders = type_to_loader_idx[p_type_hint];
        if (type_loaders.find(idx) == -1) {
          continue;
        }
      }
      found = true;
      res = loader[idx]->load(p_path, !p_original_path.is_empty() ? p_original_path : p_path, r_error, p_use_sub_threads, r_progress, p_cache_mode);
      if (!res.is_null()) {
        break;
      }
      // res is null, try next loader
    }
  }

  load_paths_stack->resize(load_paths_stack->size() - 1);
  load_nesting--;

  if (!res.is_null()) {
    return res;
  }

  ERR_FAIL_COND_V_MSG(found, Ref<Resource>(),
                      vformat("Failed loading resource: %s. Make sure resources have been imported by opening the project in the editor at least once.", p_path));

#ifdef TOOLS_ENABLED
  Ref<FileAccess> file_check = FileAccess::create(FileAccess::ACCESS_RESOURCES);
  ERR_FAIL_COND_V_MSG(!file_check->file_exists(p_path), Ref<Resource>(), vformat("Resource file not found: %s (expected type: %s)", p_path, p_type_hint));
#endif

  ERR_FAIL_V_MSG(Ref<Resource>(), vformat("No loader found for resource: %s (expected type: %s)", p_path, p_type_hint));
}

Ref<Resource> ResourceLoader::_load_complete(LoadToken& p_load_token, Error* r_error) {
  MutexLock thread_load_lock(thread_load_mutex);
  return _load_complete_inner(p_load_token, r_error, thread_load_lock);
}

// 加载函数
Ref<Resource> ResourceLoader::_load_complete_inner(LoadToken& p_load_token, Error* r_error, MutexLock<SafeBinaryMutex<BINARY_MUTEX_TAG>>& p_thread_load_lock) {
  if (r_error) {
    *r_error = OK;
  }

  if (!p_load_token.local_path.is_empty()) {
    if (!thread_load_tasks.has(p_load_token.local_path)) {
#ifdef DEV_ENABLED
      CRASH_NOW();
#endif
      // On non-dev, be defensive and at least avoid crashing (at this point at least).
      if (r_error) {
        *r_error = ERR_BUG;
      }
      return Ref<Resource>();
    }

    ThreadLoadTask& load_task = thread_load_tasks[p_load_token.local_path];

    if (load_task.status == THREAD_LOAD_IN_PROGRESS) {
      DEV_ASSERT((load_task.task_id == 0) != (load_task.thread_id == 0));

      if ((load_task.task_id != 0 && load_task.task_id == caller_task_id) || (load_task.thread_id != 0 && load_task.thread_id == Thread::get_caller_id())) {
        // Load is in progress, but it's precisely this thread the one in charge.
        // That means this is a cyclic load.
        if (r_error) {
          *r_error = ERR_BUSY;
        }
        return Ref<Resource>();
      }

      if (load_task.task_id != 0) {
        // Loading thread is in the worker pool.
        thread_load_mutex.unlock();
        // 工作
        Error err = WorkerThreadPool::get_singleton()->wait_for_task_completion(load_task.task_id);
        if (err == ERR_BUSY) {
          // The WorkerThreadPool has reported that the current task wants to await on an older one.
          // That't not allowed for safety, to avoid deadlocks. Fortunately, though, in the context of
          // resource loading that means that the task to wait for can be restarted here to break the
          // cycle, with as much recursion into this process as needed.
          // When the stack is eventually unrolled, the original load will have been notified to go on.
          // CACHE_MODE_IGNORE is needed because, otherwise, the new request would just see there's
          // an ongoing load for that resource and wait for it again. This value forces a new load.
          Ref<ResourceLoader::LoadToken> token = _load_start(load_task.local_path, load_task.type_hint, LOAD_THREAD_DISTRIBUTE, ResourceFormatLoader::CACHE_MODE_IGNORE);
          Ref<Resource> resource = _load_complete(*token.ptr(), &err);
          if (r_error) {
            *r_error = err;
          }
          thread_load_mutex.lock();
          return resource;
        } else {
          DEV_ASSERT(err == OK);
          thread_load_mutex.lock();
          load_task.awaited = true;
        }
      } else {
        // Loading thread is main or user thread.
        if (!load_task.cond_var) {
          load_task.cond_var = memnew(ConditionVariable);
        }
        do {
          load_task.cond_var->wait(p_thread_load_lock);
          DEV_ASSERT(thread_load_tasks.has(p_load_token.local_path) && p_load_token.get_reference_count());
        } while (load_task.cond_var);
      }
    }

    if (cleaning_tasks) {
      load_task.resource = Ref<Resource>();
      load_task.error = FAILED;
    }

    Ref<Resource> resource = load_task.resource;
    if (r_error) {
      *r_error = load_task.error;
    }
    return resource;
  } else {
    // Special case of an unregistered task.
    // The resource should have been loaded by now.
    Ref<Resource> resource = p_load_token.res_if_unregistered;
    if (!resource.is_valid()) {
      if (r_error) {
        *r_error = FAILED;
      }
    }
    return resource;
  }
}

/////////// load token
void ResourceLoader::LoadToken::clear() {
  thread_load_mutex.lock();

  WorkerThreadPool::TaskID task_to_await = 0;

  if (!local_path.is_empty()) {  // Empty is used for the special case where the load task is not registered.
    DEV_ASSERT(thread_load_tasks.has(local_path));
    ThreadLoadTask& load_task = thread_load_tasks[local_path];
    if (!load_task.awaited) {
      task_to_await = load_task.task_id;
      load_task.awaited = true;
    }
    thread_load_tasks.erase(local_path);
    local_path.clear();
  }

  if (!user_path.is_empty()) {
    DEV_ASSERT(user_load_tokens.has(user_path));
    user_load_tokens.erase(user_path);
    user_path.clear();
  }

  thread_load_mutex.unlock();

  // If task is unused, await it here, locally, now the token data is consistent.
  if (task_to_await) {
    WorkerThreadPool::get_singleton()->wait_for_task_completion(task_to_await);
  }
}
// RAII
ResourceLoader::LoadToken::~LoadToken() {
  clear();
}

//// ResourceFormatLoader

/*void ResourceFormatLoader::get_recognized_extensions(List<String>* p_extensions) const {
	}*/

Ref<Resource> ResourceFormatLoader::load(const String& p_path, const String& p_original_path, Error* r_error, bool p_use_sub_threads, float* r_progress,
                                         CacheMode p_cache_mode) {
  ERR_FAIL_V_MSG(Ref<Resource>(), "Failed to load resource '" + p_path + "'. ResourceFormatLoader::load was not implemented for this resource type.");
}

bool ResourceFormatLoader::recognize_path(const String & p_path, const String & p_for_type) const
{
	String extension = p_path.get_extension();
	List<String> extensions;
	if (p_for_type.is_empty()) {
		get_recognized_extensions(&extensions);
	} else {
		get_recognized_extensions_for_type(p_for_type, &extensions);
	}
	for (const String &E : extensions) {
		if (E.nocasecmp_to(extension) == 0) {
			return true;
		}
	}
	return false;
}

bool ResourceFormatLoader::handles_type(const String& p_type) const {
  List<String> p_types;
  get_recognized_resources(&p_types);
  for (const String& F : p_types) {
    if (F.nocasecmp_to(p_type) == 0) {
      return true;
    }
  }
  return false;
}

bool ResourceLoader::exists(const String& p_path, const String& p_type_hint) {
  String local_path = _validate_local_path(p_path);

  if (ResourceCache::has(local_path)) {
    return true;  // If cached, it probably exists
  }

  /*bool xl_remapped = false;
		String path = _path_remap(local_path, &xl_remapped);*/
  String ext = p_path.get_extension();
  for (auto idx : ext_to_loader_idx[ext]) {
    if (p_type_hint != "" && !type_to_loader_idx[p_type_hint].has(idx)) {
      continue;  // if have typehint but idx can't use
    }
    if (loader[idx]->exists(p_path))
      return true;
  }

  return FileAccess::exists(p_path);
}

}  // namespace lain