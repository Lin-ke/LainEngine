#include "resource_format_text.h"
#include "core/io/dir_access.h"
#include "core/config/project_settings.h"
#include "core/meta/serializer/serializer.h"
#include "core/meta/reflection/reflection.h"
#include "resource/common/scene_res.h"
#include "_generated/serializer/all_serializer.h"

//#define _printerr() ERR_PRINT(String(res_path + ":" + itos(lines) + " - Parse Error: " + error_text).utf8().get_data());

namespace lain{
	ResourceFormatLoaderText* ResourceFormatLoaderText::singleton = nullptr;


	void ResourceFormatLoaderText::get_recognized_extensions(List<String>* p_extensions) const {
		p_extensions->push_back("tscn");
		p_extensions->push_back("tres");
	}
	Ref<Resource> ResourceFormatLoaderText::load(const String& p_path, const String& p_original_path, Error* r_error, bool p_use_sub_threads, float* r_progress, CacheMode p_cache_mode) {
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
		loader.open(f);
		err = loader.load();
		if (r_error) {
			*r_error = err;
		}
		if (err == OK) {
			return loader.get_resource();
		}
		else {
			return Ref<Resource>();
		}
	}
	Error _dict_form_line(const String& line, Dictionary& dict) {
		std::string err;
		//if (line.begins_with("{") && line.ends_with("}")) {
		Json json = Json::parse(CSTR(line), err);
		if (!err.empty()) {
			return ERR_PARSE_ERROR; // Further error handle
		}
		Serializer::read(json, dict);
		if (!dict.has("tag")) {
			return ERR_FILE_CORRUPT;
		}
		return OK;
	}
	using json11::Json;
	Error ResourceLoaderText::load() {
		if (error != OK) {
			return error;
		}
		Dictionary dict;
		String tag;
		// 处理外部文件
		while(!f->eof_reached()) {
			String line = f->get_line();
			if (line == "") continue;
			error = _dict_form_line(line, dict);
			if (error != OK) {
				ERR_FAIL_V(error);
			}
			// 处理tag
			tag = dict["tag"];
			if (tag != "ext_resource") break;
			// ext_resource
			if (!dict.has("id") || !dict.has("path") || !dict.has("type")) {
				error = ERR_FILE_CORRUPT;
				ERR_FAIL_V(error);
			}
			String path = dict["path"];
			String type = dict["type"];
			String id = dict["id"];
			if (dict.has("uid")) {
				String uidt = dict["uid"];
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
			ext_resources[id].load_token = ResourceLoader::_load_start(path, type, use_sub_threads ? ResourceLoader::LOAD_THREAD_DISTRIBUTE : ResourceLoader::LOAD_THREAD_FROM_CURRENT, cache_mode_for_external);
			if (!ext_resources[id].load_token.is_valid()) {
				// 不存在的外部文件
				if (ResourceLoader::get_abort_on_missing_resources()) {
					error = ERR_FILE_CORRUPT;
					error_text = "[ext_resource] referenced non-existent resource at: " + path;
					L_CORE_ERROR(error_text);
					return error;
				}
				else {
					//ResourceLoader::notify_dependency_error(local_path, path, type);
				}
			}
			resource_current++;
		}
		//these are the ones that count
		resources_total -= resource_current;
		resource_current = 0;
		while (!f->eof_reached()) {
			String line = f->get_line();
			if (line == "") continue;
			error = _dict_form_line(line, dict);
			if (error != OK) {
				ERR_FAIL_V(error);
			}

			if (!dict.has("type")) {
				error = ERR_FILE_CORRUPT;
				error_text = "Missing 'type' in external resource tag";
				L_CORE_ERROR(error_text);
				return error;
			}

			if (!dict.has("id")) {
				error = ERR_FILE_CORRUPT;
				error_text = "Missing 'id' in external resource tag";
				L_CORE_ERROR(error_text);
				return error;
			}
			String type = dict["type"];
			String id = dict["id"];

			String path = local_path + "::" + id;

			Ref<Resource> res;
			bool do_assign = false;

			if (cache_mode == ResourceFormatLoader::CACHE_MODE_REPLACE && ResourceCache::has(path)) {
				//reuse existing
				Ref<Resource> cache = ResourceCache::get_ref(path);
				if (cache.is_valid() && cache->get_class() == type) {
					res = cache;
					res->ResetState();
					do_assign = true;
				}
			}

			if (res.is_null()) { //not reuse
				Ref<Resource> cache = ResourceCache::get_ref(path);
				if (cache_mode != ResourceFormatLoader::CACHE_MODE_IGNORE && cache.is_valid()) { //only if it doesn't exist
					//cached, do not assign
					res = cache;
				}
				else {
					//create
					Object* obj = nullptr;
					// 都是object就好办了
					{
						using Reflection::TypeMeta;
						using json11::Json;
						String json_context = dict["json"];
						auto instance = TypeMeta::newFromNameAndJson(CSTR(type), CSTR(json_context));
						obj = static_cast<Object*>(instance.m_instance);
					}
					if (!obj) {
						/*if (ResourceLoader::is_creating_missing_resources_if_class_unavailable_enabled()) {
							missing_resource = memnew(MissingResource);
							missing_resource->set_original_class(type);
							missing_resource->set_recording_properties(true);
							obj = missing_resource;
						}
						else {*/
							error_text += "Can't create sub resource of type: " + type;
							L_CORE_ERROR(error_text);
							error = ERR_FILE_CORRUPT;
							return error;
						//}
					}

					Resource* r = Object::cast_to<Resource>(obj);
					if (!r) {
						error_text += "Can't create sub resource of type, because not a resource: " + type;
						L_CORE_ERROR(error_text);
						error = ERR_FILE_CORRUPT;
						return error;
					}

					res = Ref<Resource>(r);
					do_assign = true;
				}
			}
			resource_current++;
			if (progress && resources_total > 0) {
				*progress = resource_current / float(resources_total);
			}

			int_resources[id] = res; // Always assign int resources.
			if (do_assign) {
				if (cache_mode != ResourceFormatLoader::CACHE_MODE_IGNORE) {
					res->SetPath(path, cache_mode == ResourceFormatLoader::CACHE_MODE_REPLACE);
				}
				else {
					res->SetPathCache(path);
				}
				res->set_scene_unique_id(id);
			}
		}
		return error;
	}
	void ResourceLoaderText::open(Ref<FileAccess> p_f, bool p_skip_first_tag) { 
		// basic informations
		error = OK;
		f = p_f;
		String line = p_f->get_line();
		Dictionary p;
		if (_dict_form_line(line, p) != OK) {
			error = FAILED;
			return;
		}
		
		String name = p["tag"];
		if (name == "scene") {
			is_scene = true;
		}
		else if (name == "resource") {

		}
		else {
			error_text = "Unrecognized file type: " + name;
			L_CORE_ERROR(error_text);
			error = ERR_PARSE_ERROR;
			return;
		}
		if (p.has("uid")) {
			res_uid = ResourceUID::get_singleton()->text_to_id(p["uid"]);
		}
		if (p.has("load_steps")) {
			resources_total = p["load_steps"];
		} 

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
		}
		else {
			dict["tag"] = "resource";
		}
		dict["load_steps"] = resources_total;
		fw->store_string(Serializer::write(dict).dump().c_str()); 

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
	ResourceFormatSaverText* ResourceFormatSaverText::singleton = nullptr;
	Error ResourceFormatSaverText::save(const Ref<Resource>& p_resource, const String& p_path, uint32_t p_flags) {
		if (p_path.ends_with(".tscn") && !Ref<PackedScene>(p_resource).is_valid()) {
			return ERR_FILE_UNRECOGNIZED;
		}

		ResourceSaverText saver;
		return saver.save(p_path, p_resource, p_flags);
	}

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
			p_extensions->push_back("tscn"); // Text scene.
		}
		else {
			p_extensions->push_back("tres"); // Text resource.
		}
	}

	void ResourceFormatSaverText::get_possible_extensions(List<String>* p_extensions) const{
		p_extensions->push_back("tscn");
		p_extensions->push_back("tres"); // Text resource.

	}
	void ResourceFormatSaverText::get_possible_resources(List<String>* p_extensions) const {
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
		//_find_resources(p_resource, true);

		//if (packed_scene.is_valid()) {
		//	// Add instances to external resources if saving a packed scene.
		//	for (int i = 0; i < packed_scene->get_state()->get_node_count(); i++) {
		//		if (packed_scene->get_state()->is_node_instance_placeholder(i)) {
		//			continue;
		//		}

		//		Ref<PackedScene> instance = packed_scene->get_state()->get_node_instance(i);
		//		if (instance.is_valid() && !external_resources.has(instance)) {
		//			int index = external_resources.size() + 1;
		//			external_resources[instance] = itos(index) + "_" + Resource::generate_scene_unique_id(); // Keep the order for improved thread loading performance.
		//		}
		//	}
		//}
		Dictionary title;
		title["tag"] = packed_scene.is_valid()? "scene" : "resource";
			
		if (packed_scene.is_null()) {
			title["script_class"] = "";
		}
		title["load_steps"] = saved_resources.size() + external_resources.size();
		ResourceUID::ID uid = ResourceSaver::get_resource_id_for_path(local_path, true);
		if(uid != ResourceUID::INVALID_ID)
			title["uid"] = uid;
		if (packed_scene.is_valid()) {
			PackedSceneRes packed_res;
			packed_res.head = title;
			for (int i = 0; i < packed_scene->get_state()->get_gobject_count();++i) {

				Ref<PackedScene> instance = packed_scene->get_state()->get_gobject_instance(i);
				if (instance.is_valid() && !external_resources.has(instance)) {
					int index = external_resources.size() + 1;
					external_resources[instance] = Pair(index, Resource::generate_scene_unique_id()); // Keep the order for improved thread loading performance.
				}
			}
			HashSet<String> cached_ids_found;
			for (KeyValue<Ref<Resource>, Pair<int, String>>& E : external_resources) {
				
				String cached_id = E.key->get_id_for_path(local_path);
				if (cached_id.is_empty() || cached_ids_found.has(cached_id)) {
					// continue to use
				}
				else {
					E.value.second = cached_id;
					cached_ids_found.insert(cached_id);
				}
			}
			Vector<Tuple<int, String, Ref<Resource>>> sorted_res;

			// Create IDs for non cached resources.
			for (KeyValue<Ref<Resource>, Pair<int, String>>& E : external_resources) {
				if (cached_ids_found.has(E.value.second)) { // Already cached, go on.
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
				sorted_res.push_back(Tuple(E.value.first, E.value.second, E.key));
			}
			sorted_res.sort();

			// sort
			// 0 inx, 1 id, 2 res
			
			for (auto&& E : sorted_res) {
				ExtRes ext_res;
				String p = __tuple_get<2>(E)->GetPath();

				ext_res.m_id = itos(__tuple_get<0>(E)) + __tuple_get<1>(E);
				ext_res.m_def_path = p;
				auto uid = ResourceSaver::get_resource_id_for_path(p, false);
				if (uid != ResourceUID::INVALID_ID) {
					ext_res.m_uid = ResourceUID::get_singleton()->id_to_text(uid);
				}
				packed_res.ext_res.push_back(ext_res);
			}
			auto&& json = Serializer::write(packed_res);
			f->store_string(json.dump());
		}
	}

}