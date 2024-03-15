//#include "editor_paths.h"
//#include "core/config/project_settings.h"
//#include "core/os/os.h"
//#include "core/os/dir_access.h"
//namespace lain {
//
//
//	EditorPaths* EditorPaths::p_singleton = nullptr;
//	EditorPaths::EditorPaths() {
//		p_singleton = this;
//
//		project_data_dir = ProjectSettings::GetSingleton()->GetProjectDataPath();
//		auto exe_path = OS::GetSingleton()->GetExecutablePath().get_base_dir();
//		Ref<DirAccess> d = DirAccess::create_for_path(exe_path);
//		if (d->file_exists(exe_path + "/._sc_")) {
//			self_contained = true;
//			self_contained_file = exe_path + "/._sc_";
//		}
//		else if (d->file_exists(exe_path + "/_sc_")) {
//			self_contained = true;
//			self_contained_file = exe_path + "/_sc_";
//		}
//
//		String data_path;
//		String config_path;
//		String cache_path;
//
//		if (self_contained) {
//			// editor is self contained, all in same folder
//			data_path = exe_path;
//			data_dir = data_path.path_join("editor_data");
//			config_path = exe_path;
//			config_dir = data_dir;
//			cache_path = exe_path;
//			cache_dir = data_dir.path_join("cache");
//		}
//		else {
//			// Typically XDG_DATA_HOME or %APPDATA%.
//			data_path = OS::GetSingleton()->GetDataPath();
//			data_dir = data_path.path_join("datapath");
//			// Can be different from data_path e.g. on Linux or macOS.
//			config_path = OS::GetSingleton()->get_config_path();
//			config_dir = config_path.path_join(OS::GetSingleton()->get_godot_dir_name());
//			// Can be different from above paths, otherwise a subfolder of data_dir.
//			cache_path = OS::GetSingleton()->get_cache_path();
//			if (cache_path == data_path) {
//				cache_dir = data_dir.path_join("cache");
//			}
//			else {
//				cache_dir = cache_path.path_join(OS::GetSingleton()->get_godot_dir_name());
//			}
//		}
//
//		paths_valid = (!data_path.is_empty() && !config_path.is_empty() && !cache_path.is_empty());
//		ERR_FAIL_COND_MSG(!paths_valid, "Editor data, config, or cache paths are invalid.");
//
//		// Validate or create each dir and its relevant subdirectories.
//
//		Ref<DirAccess> dir = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
//
//		// Data dir.
//		{
//			if (dir->change_dir(data_dir) != OK) {
//				dir->make_dir_recursive(data_dir);
//				if (dir->change_dir(data_dir) != OK) {
//					ERR_PRINT("Could not create editor data directory: " + data_dir);
//					paths_valid = false;
//				}
//			}
//
//			if (!dir->dir_exists(export_templates_folder)) {
//				dir->make_dir(export_templates_folder);
//			}
//		}
//
//		// Config dir.
//		{
//			if (dir->change_dir(config_dir) != OK) {
//				dir->make_dir_recursive(config_dir);
//				if (dir->change_dir(config_dir) != OK) {
//					ERR_PRINT("Could not create editor config directory: " + config_dir);
//					paths_valid = false;
//				}
//			}
//
//			if (!dir->dir_exists(text_editor_themes_folder)) {
//				dir->make_dir(text_editor_themes_folder);
//			}
//			if (!dir->dir_exists(script_templates_folder)) {
//				dir->make_dir(script_templates_folder);
//			}
//			if (!dir->dir_exists(feature_profiles_folder)) {
//				dir->make_dir(feature_profiles_folder);
//			}
//		}
//
//		// Cache dir.
//		{
//			if (dir->change_dir(cache_dir) != OK) {
//				dir->make_dir_recursive(cache_dir);
//				if (dir->change_dir(cache_dir) != OK) {
//					ERR_PRINT("Could not create editor cache directory: " + cache_dir);
//					paths_valid = false;
//				}
//			}
//		}
//
//		// Validate or create project-specific editor data dir,
//		// including shader cache subdir.
//		if (Engine::GetSingleton()->is_project_manager_hint() || (Main::is_cmdline_tool() && !ProjectSettings::GetSingleton()->is_project_loaded())) {
//			// Nothing to create, use shared editor data dir for shader cache.
//			Engine::GetSingleton()->set_shader_cache_path(data_dir);
//		}
//		else {
//			Ref<DirAccess> dir_res = DirAccess::create(DirAccess::ACCESS_RESOURCES);
//			if (dir_res->change_dir(project_data_dir) != OK) {
//				dir_res->make_dir_recursive(project_data_dir);
//				if (dir_res->change_dir(project_data_dir) != OK) {
//					ERR_PRINT("Could not create project data directory (" + project_data_dir + ") in: " + dir_res->get_current_dir());
//					paths_valid = false;
//				}
//			}
//
//			// Check that the project data directory '.gdignore' file exists
//			String project_data_gdignore_file_path = project_data_dir.path_join(".gdignore");
//			if (!FileAccess::exists(project_data_gdignore_file_path)) {
//				// Add an empty .gdignore file to avoid scan.
//				Ref<FileAccess> f = FileAccess::open(project_data_gdignore_file_path, FileAccess::WRITE);
//				if (f.is_valid()) {
//					f->store_line("");
//				}
//				else {
//					ERR_PRINT("Failed to create file " + project_data_gdignore_file_path);
//				}
//			}
//
//			Engine::GetSingleton()->set_shader_cache_path(project_data_dir);
//
//			// Editor metadata dir.
//			if (!dir_res->dir_exists("editor")) {
//				dir_res->make_dir("editor");
//			}
//			// Imported assets dir.
//			String imported_files_path = ProjectSettings::GetSingleton()->get_imported_files_path();
//			if (!dir_res->dir_exists(imported_files_path)) {
//				dir_res->make_dir(imported_files_path);
//			}
//		}
//	}
//
//	bool EditorPaths::are_paths_valid() const {
//		return paths_valid;
//	}
//
//	String EditorPaths::get_data_dir() const {
//		return data_dir;
//	}
//
//	String EditorPaths::get_config_dir() const {
//		return config_dir;
//	}
//
//	String EditorPaths::get_cache_dir() const {
//		return cache_dir;
//	}
//
//	String EditorPaths::get_project_data_dir() const {
//		return project_data_dir;
//	}
//
//	bool EditorPaths::is_self_contained() const {
//		return self_contained;
//	}
//
//	String EditorPaths::get_self_contained_file() const {
//		return self_contained_file;
//	}
//
//}