#include "resource_format_text.h"
#include "_generated/serializer/all_serializer.h"
#include "core/config/project_settings.h"
#include "core/io/dir_access.h"
#include "core/meta/reflection/reflection.h"
#include "core/meta/serializer/serializer.h"
#include "core/templates/tuple.h"
#include "scene/resources/common/scene_res.h"
//#define _printerr() ERR_PRINT(String(res_path + ":" + itos(lines) + " - Parse Error: " + error_text).utf8().get_data());

namespace lain {
ResourceFormatLoaderText* ResourceFormatLoaderText::singleton = nullptr;
ResourceFormatSaverText* ResourceFormatSaverText::singleton = nullptr;
Error ResourceFormatSaverText::save(const Ref<Resource>& p_resource, const String& p_path, uint32_t p_flags) {
  if (p_path.ends_with(".tscn") && !Ref<PackedScene>(p_resource).is_valid()) {
    return ERR_FILE_UNRECOGNIZED;
  }

  ResourceSaverText saver;
  return saver.save(p_path, p_resource, p_flags);
}

void ResourceFormatLoaderText::get_recognized_extensions(List<String>* p_extensions) const {
  p_extensions->push_back("tscn");
  p_extensions->push_back("tres");
}

void ResourceFormatLoaderText::get_recognized_resources(List<String>* p_extensions) const {
  p_extensions->push_back("PackedScene");
}
Ref<Resource> ResourceFormatLoaderText::load(const String& p_path, const String& p_original_path, Error* r_error, bool p_use_sub_threads, float* r_progress,
                                             CacheMode p_cache_mode) {
  if (r_error) {
    *r_error = ERR_CANT_OPEN;
  }
  Error err;

  Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ, &err);

  ERR_FAIL_COND_V_MSG(err != OK, Ref<Resource>(), "Cannot open file '" + p_path + "'.");

  ResourceLoaderText loader;
  String path = !p_original_path.is_empty() ? p_original_path : p_path;
  switch (p_cache_mode) {
    case CACHE_MODE_IGNORE:
    case CACHE_MODE_REUSE:
    case CACHE_MODE_REPLACE:
      loader.cache_mode = p_cache_mode;
      loader.cache_mode_for_external = CACHE_MODE_REUSE;
      break;
    case CACHE_MODE_IGNORE_DEEP:
      loader.cache_mode = ResourceFormatLoader::CACHE_MODE_IGNORE;
      loader.cache_mode_for_external = p_cache_mode;
      break;
    case CACHE_MODE_REPLACE_DEEP:
      loader.cache_mode = ResourceFormatLoader::CACHE_MODE_REPLACE;
      loader.cache_mode_for_external = p_cache_mode;
      break;
  }
  loader.use_sub_threads = p_use_sub_threads;
  loader.local_path = ProjectSettings::GetSingleton()->LocalizePath(path);
  loader.progress = r_progress;
  loader.res_path = loader.local_path;
  loader.open(f);  // 读封上的头，文件类型等
  err = loader.load();
  if (r_error) {
    *r_error = err;
  }
  if (err == OK) {
    return loader.get_resource();
  } else {
    return Ref<Resource>();
  }
}

Error ResourceLoaderText::load() {

  if (error != OK) {
    return error;
  }

  PackedSceneRes packed_res;

  String json_str = f->get_residual_text();
  String error_line;

  Json json_text = Json::parse(json_str, error_line);

  ERR_FAIL_COND_V_MSG(!error_line.is_empty(), FAILED, error_line);
  Serializer::read(json_text, packed_res);

  Dictionary p = packed_res.head;
  String name = p["tag"];


  if (name == "scene") {
    is_scene = true;
  } else if (name == "resource") {

  } else {
    error_text = "Unrecognized file type: " + name;
    error = ERR_PARSE_ERROR;
    ERR_FAIL_COND_V_MSG(false, ERR_FILE_CORRUPT, error_text);
  }
  if (p.has("uid")) {
    res_uid = ResourceUID::get_singleton()->text_to_id(p["uid"]);
  }
  if (p.has("load_steps")) {
    resources_total = p["load_steps"];
  }
  // 处理外部文件
  for (auto&& ext_file : packed_res.ext_res) {
    String path = ext_file.m_def_path;
    String type = ext_file.m_type;
    String id = ext_file.m_id;
    if (ext_file.m_uid != "") {
      String uidt = ext_file.m_uid;
      ResourceUID::ID uid = ResourceUID::get_singleton()->text_to_id(uidt);
      if (uid != ResourceUID::INVALID_ID && ResourceUID::get_singleton()->has_id(uid)) {
        // If a UID is found and the path is valid, it will be used, otherwise, it falls back to the path.
        path = ResourceUID::get_singleton()->get_id_path(uid);
      }
    }
    if (!path.contains("://") && path.is_relative_path()) {
      // path is relative to file being loaded, so convert to a resource path
      path = ProjectSettings::GetSingleton()->LocalizePath(local_path.get_base_dir().path_join(path));
    }

    ext_resources[id].path = path;
    ext_resources[id].type = type;
    ext_resources[id].load_token =
        ResourceLoader::_load_start(path, type, use_sub_threads ? ResourceLoader::LOAD_THREAD_DISTRIBUTE : ResourceLoader::LOAD_THREAD_FROM_CURRENT, cache_mode_for_external);
    if (!ext_resources[id].load_token.is_valid()) {
      // 不存在的外部文件
      if (ResourceLoader::get_abort_on_missing_resources()) {
        error = ERR_FILE_CORRUPT;
        error_text = "[ext_resource] referenced non-existent resource at: " + path;
        L_PERROR(error_text);
        return error;
      } else {
        //ResourceLoader::notify_dependency_error(local_path, path, type); // trying to load again
        error_text = "[ext_resource] dependency error resource at: " + path;
        L_PERROR(error_text);
        return error;
      }
    }
    resource_current++;
  }
  //these are the ones that count
  resources_total -= resource_current;
  resource_current = 0;
  //
  // @TODO:sub_resource
  // @TODO:resource

  if (!is_scene && packed_res.gobjects.size() > 0) {
    error_text += "found the 'gobject' tag on a resource file!";
    error = ERR_FILE_CORRUPT;
    ERR_FAIL_COND_V_MSG(true, ERR_FILE_CORRUPT, error_text);
  }

  HashMap<String, Variant> dict;
  Ref<PackedScene> packed_scene;
  packed_scene.instantiate();
  for (auto&& gobject : packed_res.gobjects) {
    dict = gobject.m_variants;
    int parent = -1;
    int owner = -1;
    int type = -1;
    int name = -1;
    int instance = -1;
    int index = -1;
		
    for (auto kv : dict) {
			// L_PRINT("resource dict key: " + kv.key);
			// L_PRINT("resource dict value: " + kv.value.operator String());
			String key = kv.key;
      if (key == "name") {
        name = packed_scene->get_state()->add_name(dict["name"]);
        continue;
      }
      if (key == "type") {
        type = packed_scene->get_state()->add_name(dict["type"]);
        continue;
      }
      if (key == "parent") {
        GObjectPath np = dict["parent"];
        np.prepend_period();  //compatible to how it manages paths internally
        parent = packed_scene->get_state()->add_gobject_path(np);
        continue;
      }
      if (key == "instance") {
        String type_id = dict["instance"];
        int start_pos = 0;
        int end_pos = 0;
        for (int i = 0; i < type_id.length(); i++) {
          if (type_id[i] == '\"') {
            if (!start_pos)
              start_pos = i;
            else
              end_pos = i;
          }
        }
        String id = type_id.substr(start_pos + 1, end_pos - start_pos - 1);
        String type = type_id.substr(0, start_pos - 1);  // without (
        Ref<ResourceLoader::LoadToken>& load_token = ext_resources[id].load_token;
        Error err = OK;
        Ref<Resource> r_res;
        if (!ext_resources.has(id)) {
          L_PERROR("Can't load cached ext-resource id: " + id);
        } else {

          if (load_token.is_valid()) {  // If not valid, it's OK since then we know this load accepts broken dependencies.
            Ref<Resource> res = ResourceLoader::_load_complete(*load_token.ptr(), &err);
            if (res.is_null()) {
              /*if (!ResourceLoader::is_cleaning_tasks()) {
								if (ResourceLoader::get_abort_on_missing_resources()) {
									error = ERR_FILE_MISSING_DEPENDENCIES;
									error_text = "[ext_resource] referenced non-existent resource at: " + path;
									_printerr();
									err = error;
								}
								else {
									ResourceLoader::notify_dependency_error(local_path, path, type);
								}
							}*/
            } else {
              res->set_id_for_path(local_path, id);
              r_res = res;
            }
          } else {
            r_res = Ref<Resource>();
          }
#ifdef TOOLS_ENABLED
          if (r_res.is_null()) {
            // Hack to allow checking original path.
            r_res.instantiate();
            //r_res->set_meta("__load_path__", ext_resources[id].path);
          }
#endif
        }

        instance = packed_scene->get_state()->add_value(r_res);  // variants[instance] = ref<resource> from ext_resource, value: "Ext("..")"

        if (packed_scene->get_state()->get_gobject_count() == 0 && parent == -1) {
          packed_scene->get_state()->set_base_scene(instance);  //int
          instance = -1;
        }
				continue;
      }
			if(key == "owner"){
  	    owner = packed_scene->get_state()->add_gobject_path(dict["owner"]);
				continue;
			}
			if(key == "index"){
				index = dict["index"];
				continue;
			}
    }

		if (type == -1){
     type = SceneState::TYPE_INSTANTIATED; //no type? assume this was instantiated
		}
		if(owner == -1){
			if (parent != -1 && !(type == SceneState::TYPE_INSTANTIATED && instance == -1)) {
        owner = 0;  //if no owner, owner is root
      }
		}
    int gobject_id = packed_scene->get_state()->add_gobject(parent, owner, type, name, instance, index);
    if (dict.has("groups")) {
      Array groups = dict["groups"];
      for (int i = 0; i < groups.size(); i++) {
        packed_scene->get_state()->add_gobject_group(gobject_id, packed_scene->get_state()->add_name(groups[i]));
      }
    }
		HashSet<StringName> path_properties;

		for(auto kv : dict){
			String key = kv.key;
			if(key == "name" || key == "type" || key == "parent" || key == "instance" || key == "owner" || key == "index" || key == "groups"){
				continue;
			}
			// 一般情况
				int nameidx = packed_scene->get_state()->add_name(key);
				int valueidx = packed_scene->get_state()->add_value(dict[key]);
				packed_scene->get_state()->add_gobject_property(gobject_id, nameidx, valueidx, path_properties.has(key));
		}

    if (gobject.m_instanced_components.size() > 0) {
      Vector<Component*> cmpts;
      cmpts.resize(gobject.m_instanced_components.size());
      auto&& writer = cmpts.ptrw();
      for (int i = 0; i < gobject.m_instanced_components.size(); i++) {
        writer[i] = gobject.m_instanced_components[i].operator->();
      }
      packed_scene->get_state()->add_components(gobject_id, cmpts);  //ref
    }

    //
    // properties
    packed_scene->get_state()->add_instance_res(gobject_id, gobject.m_instance_res);
    // 按他这个写法和python有啥区别，dict和variant装全部
    // 组件里面只有一些数据，所以也不是不行吧

  }  // exit parsing gobject
  /*if (!packed_scene.is_valid()) {
			return error;
		}*/

  error = OK;
  //get it here
  resource = packed_scene;
  if (cache_mode != ResourceFormatLoader::CACHE_MODE_IGNORE) {
    if (!ResourceCache::has(res_path)) {
      packed_scene->set_path(res_path);
    }
  } else {
    packed_scene->get_state()->set_path(res_path);
    packed_scene->set_pathCache(res_path);
  }

  resource_current++;

  if (progress && resources_total > 0) {
    *progress = resource_current / float(resources_total);
  }

  return error;
}

void ResourceLoaderText::open(Ref<FileAccess> p_f, bool p_skip_first_tag) {
  // basic informations
  error = OK;
  f = p_f;
}

// 封上一个头
Error ResourceLoaderText::set_uid(Ref<FileAccess> p_f, ResourceUID::ID p_uid) {
  open(p_f, true);
  ERR_FAIL_COND_V(error != OK, error);
  //ignore_resource_parsing = true;

  Ref<FileAccess> fw;

  fw = FileAccess::open(local_path + ".uidren", FileAccess::WRITE);
  Dictionary dict;
  if (is_scene) {
    dict["tag"] = "scene";
    dict["uid"] = ResourceUID::get_singleton()->id_to_text(p_uid);
  } else {
    dict["tag"] = "resource";
  }
  dict["load_steps"] = resources_total;
  fw->store_string(Serializer::write(dict).dump());

  uint8_t c = f->get_8();
  while (!f->eof_reached()) {
    fw->store_8(c);
    c = f->get_8();
  }

  bool all_ok = fw->get_error() == OK;

  if (!all_ok) {
    return ERR_CANT_CREATE;
  }

  return OK;
}

/// saver

Error ResourceFormatSaverText::set_uid(const String& p_path, ResourceUID::ID p_uid) {
  String lc = p_path.to_lower();
  if (!lc.ends_with(".tscn") && !lc.ends_with(".tres")) {
    return ERR_FILE_UNRECOGNIZED;
  }

  String local_path = ProjectSettings::GetSingleton()->LocalizePath(p_path);
  Error err = OK;
  {
    Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
    if (file.is_null()) {
      ERR_FAIL_V(ERR_CANT_OPEN);
    }

    ResourceLoaderText loader;
    loader.local_path = local_path;
    loader.res_path = loader.local_path;
    err = loader.set_uid(file, p_uid);
  }

  if (err == OK) {
    Ref<DirAccess> da = DirAccess::create(DirAccess::ACCESS_RESOURCES);
    da->remove(local_path);
    da->rename(local_path + ".uidren", local_path);
  }

  return err;
}

void ResourceFormatSaverText::get_recognized_extensions(const Ref<Resource>& p_resource, List<String>* p_extensions) const {
  if (Ref<PackedScene>(p_resource).is_valid()) {
    p_extensions->push_back("tscn");  // Text scene.
  } else {
    p_extensions->push_back("tres");  // Text resource.
  }
}

void ResourceFormatSaverText::get_recognized_extensions(List<String>* p_extensions) const {
  p_extensions->push_back("tscn");
  p_extensions->push_back("tres");  // Text resource.
}
void ResourceFormatSaverText::get_recognized_resources(List<String>* p_extensions) const {
  p_extensions->push_back("PackedScene");
}
ResourceFormatSaverText::ResourceFormatSaverText() {
  singleton = this;
}

// save to .tscn
Error ResourceSaverText::save(const String& p_path, const Ref<Resource>& p_resource, uint32_t p_flags) {
  packed_scene = p_resource;

  Error err;
  Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::WRITE, &err);
  ERR_FAIL_COND_V_MSG(err, ERR_CANT_OPEN, "Cannot save file '" + p_path + "'.");
  Ref<FileAccess> _fref(f);

  local_path = ProjectSettings::GetSingleton()->LocalizePath(p_path);

  relative_paths = p_flags & ResourceSaver::FLAG_RELATIVE_PATHS;
  skip_editor = p_flags & ResourceSaver::FLAG_OMIT_EDITOR_PROPERTIES;
  bundle_resources = p_flags & ResourceSaver::FLAG_BUNDLE_RESOURCES;
  takeover_paths = p_flags & ResourceSaver::FLAG_REPLACE_SUBRESOURCE_PATHS;
  if (!p_path.begins_with("res://")) {
    takeover_paths = false;
  }

  // Save resources.
  _find_resources(p_resource, true);

  Dictionary title;
  title["tag"] = packed_scene.is_valid() ? "scene" : "resource";
  if (packed_scene.is_null()) {
    title["type"] = p_resource->get_class();
    title["script_class"] = "";
  }
  ResourceUID::ID uid = ResourceSaver::get_resource_id_for_path(local_path, true);
  if (uid != ResourceUID::INVALID_ID)
    title["uid"] = uid;
  if (packed_scene.is_valid()) {
    PackedSceneRes packed_res;
    for (int i = 0; i < packed_scene->get_state()->get_gobject_count(); ++i) {

      Ref<PackedScene> instance = packed_scene->get_state()->get_gobject_instance(i);
      if (instance.is_valid() && !external_resources.has(instance)) {
        int index = external_resources.size() + 1;
        external_resources[instance] = Pair(index, Resource::generate_scene_unique_id());  // Keep the order for improved thread loading performance.
      }
    }
    HashSet<String> cached_ids_found;
    for (KeyValue<Ref<Resource>, Pair<int, String>>& E : external_resources) {

      String cached_id = E.key->get_id_for_path(local_path);
      if (cached_id.is_empty() || cached_ids_found.has(cached_id)) {
        // continue to use
      } else {
        E.value.second = cached_id;
        cached_ids_found.insert(cached_id);
      }
    }

    Vector<Pair<int, Pair<String, Ref<Resource>>>> sorted_res;

    //sorted_res.resize(10);
    // Create IDs for non cached resources.
    for (KeyValue<Ref<Resource>, Pair<int, String>>& E : external_resources) {
      if (cached_ids_found.has(E.value.second)) {  // Already cached, go on.
        continue;
      }

      String attempt;
      while (true) {
        attempt = E.value.second + Resource::generate_scene_unique_id();
        if (!cached_ids_found.has(attempt)) {
          break;
        }
      }

      cached_ids_found.insert(attempt);
      E.value.second = attempt;
      // Update also in resource.
      Ref<Resource> res = E.key;
      res->set_id_for_path(local_path, attempt);
      sorted_res.push_back(Pair(E.value.first, Pair(E.value.second, E.key)));
    }
    if (sorted_res.size() > 0)
      sorted_res.sort_custom<PairSort<int, Pair<String, Ref<Resource>>>>();
    // sort
    // 0 inx, 1 id, 2 res
    packed_res.ext_res.resize(sorted_res.size());
    {
      auto&& prtw = packed_res.ext_res.ptrw();
      int idx = 0;
      for (auto&& E : sorted_res) {
        ExtRes ext_res;
        String p = E.second.second->GetPath();

        ext_res.m_id = itos(E.first) + "_" + E.second.first;
        ext_res.m_def_path = p;
        ext_res.m_type = E.second.second->get_class();
        auto uid = ResourceSaver::get_resource_id_for_path(p, false);
        if (uid != ResourceUID::INVALID_ID) {
          ext_res.m_uid = ResourceUID::get_singleton()->id_to_text(uid);
        }
        prtw[idx++] = ext_res;
      }
    }

    if (packed_scene.is_valid()) {
      // If this is a scene, save gobjects and connections!
      Ref<SceneState> state = packed_scene->get_state();
      int gobjects_size = state->get_gobject_count();
      packed_res.gobjects.resize(gobjects_size);
      for (int i = 0; i < gobjects_size; i++) {
        StringName type = state->get_gobject_type(i);
        StringName name = state->get_gobject_name(i);
        int index = state->get_gobject_index(i);
        GObjectPath path = state->get_gobject_path(i, true);
        GObjectPath owner = state->get_gobject_owner_path(i);
        Ref<PackedScene> instance = state->get_gobject_instance(i);
        GObjectInstanceRes gires;
        if (type != StringName()) {
          gires.m_variants["type"] = type;
        }
        if (path != GObjectPath()) {
          gires.m_variants["parent"] = path;
        }
        if (owner != GObjectPath() && owner != GObjectPath(".")) {
          gires.m_variants["owner"] = owner.simplified();
        }
        if (index >= 0) {
          gires.m_variants["index"] = index;
        }
        if (instance.is_valid()) {
          Ref<Resource> res = instance;
          String p;
          if (external_resources.has(res)) {
            p = "ExtResource(\"" + itos(external_resources[res].first) + String("_") + external_resources[res].second + "\")";
          } else {
            if (internal_resources.has(res)) {
              p = "SubResource(\"" + internal_resources[res] + "\")";
            } else if (!res->IsBuiltIn()) {
              if (res->GetPath() == local_path) {  //circular reference attempt
                p = "null";
              }
              //external resource
              String path = relative_paths ? local_path.path_to_file(res->GetPath()) : res->GetPath();
              p = "Resource(\"" + path + "\")";
            } else {
              ERR_PRINT("Resource was not pre cached for the resource section, bug?");
              p = "";
              //internal resource
            }
          }
          gires.m_variants["instance"] = p;
        }

        gires.m_variants["name"] = name;
        // variants
        for (int j = 0; j < state->get_gobject_property_count(i); j++) {
          gires.m_variants[state->get_gobject_property_name(i, j)] = state->get_gobject_property_value(i, j);
        }

        Vector<Component*> cmpts = state->get_gobject_components(i);
        gires.m_instance_res = state->get_gobject_insres(i);

        using Reflection::ReflectionPtr;
        Vector<ReflectionPtr<Component>> cmpts_ref;
        cmpts_ref.resize(cmpts.size());
        {
          auto&& ptrw = cmpts_ref.ptrw();

          for (int idx = 0; idx < cmpts.size(); ++idx) {
            cmpts_ref.ptrw()[idx] = ReflectionPtr<Component>(cmpts[idx]->get_c_class(), cmpts[idx]);
          }
        }
        gires.m_instanced_components = cmpts_ref;
        packed_res.gobjects.ptrw()[i] = gires;
      }
    }

    title["load_steps"] = saved_resources.size() + external_resources.size();
    packed_res.head = title;

    auto&& json = Serializer::write(packed_res);
    f->store_string(json.dump());
  }
  return err;
}

void ResourceSaverText::_find_resources(const Variant& p_variant, bool p_main) {
  switch (p_variant.get_type()) {
    case Variant::OBJECT: {
      Ref<Resource> res = p_variant;

      if (res.is_null() || external_resources.has(res) /*|| res->get_meta(SNAME("_skip_save_"), false)*/) {
        return;
      }

      if (!p_main && (!bundle_resources) && !res->IsBuiltIn()) {
        if (res->GetPath() == local_path) {
          ERR_PRINT("Circular reference to resource being saved found: '" + local_path + "' will be null next time it's loaded.");
          return;
        }

        // Use a numeric ID as a base, because they are sorted in natural order before saving.
        // This increases the chances of thread loading to fetch them first.
        external_resources[res] = Pair(static_cast<int>(external_resources.size() + 1), Resource::generate_scene_unique_id());
        return;
      }

      if (resource_set.has(res)) {
        return;
      }

      resource_set.insert(res);
      // property中可能有需要保存的Res
      /*List<PropertyInfo> property_list;

			res->get_property_list(&property_list);
			property_list.sort();

			List<PropertyInfo>::Element* I = property_list.front();

			while (I) {
				PropertyInfo pi = I->get();

				if (pi.usage & PROPERTY_USAGE_STORAGE) {
					Variant v = res->get(I->get().name); // 感觉这么写很不优雅啊

					if (pi.usage & PROPERTY_USAGE_RESOURCE_NOT_PERSISTENT) {
						NonPersistentKey npk;
						npk.base = res;
						npk.property = pi.name;
						non_persistent_map[npk] = v;

						Ref<Resource> sres = v;
						if (sres.is_valid()) {
							resource_set.insert(sres);
							saved_resources.push_back(sres);
						}
						else {
							_find_resources(v);
						}
					}
					else {
						_find_resources(v);
					}
				}

				I = I->next();
			}*/

      saved_resources.push_back(res);  // Saved after, so the children it needs are available when loaded

    } break;
    case Variant::ARRAY: {
      Array varray = p_variant;
      int len = varray.size();
      for (int i = 0; i < len; i++) {
        const Variant& v = varray.get(i);
        _find_resources(v);
      }

    } break;
    case Variant::DICTIONARY: {
      Dictionary d = p_variant;
      List<Variant> keys;
      d.get_key_list(&keys);
      for (const Variant& E : keys) {
        // Of course keys should also be cached, after all we can't prevent users from using resources as keys, right?
        // See also ResourceFormatSaverBinaryInstance::_find_resources (when p_variant is of type Variant::DICTIONARY)
        _find_resources(E);
        Variant v = d[E];
        _find_resources(v);
      }
    } break;
    default: {
    }
  }
}

}  // namespace lain