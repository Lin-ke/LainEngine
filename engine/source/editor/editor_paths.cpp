#include "editor_paths.h"
#include "core/config/project_settings.h"
#include "core/os/os.h"
#include "core/io/dir_access.h"
namespace lain {
	EditorPaths* EditorPaths::p_singleton = nullptr;
	EditorPaths::EditorPaths() {
		p_singleton = this;
		project_data_dir = ProjectSettings::GetSingleton()->GetProjectDataPath();
		String data_path;
		String config_path;
		String cache_path;
		data_path = OS::GetSingleton()->GetDataPath();
		data_dir = data_path.path_join(LAINDIRNAME);
		// Can be different from data_path e.g. on Linux or macOS.
		config_path = OS::GetSingleton()->GetConfigPath();
		config_dir = config_path.path_join(LAINDIRNAME);
		// Can be different from above paths, otherwise a subfolder of data_dir.
		cache_path = OS::GetSingleton()->GetCachePath();
		if (cache_path == data_path) {
			cache_dir = data_dir.path_join("cache");
		}
		else {
			cache_dir = cache_path.path_join(LAINDIRNAME);
		}
		paths_valid = (!data_path.is_empty() && !config_path.is_empty() && !cache_path.is_empty());
		ERR_FAIL_COND_MSG(!paths_valid, "Editor data, config, or cache paths are invalid.");

		Ref<DirAccess> dir = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);

		// Data dir.
		{
			if (dir->change_dir(data_dir) != OK) {
				dir->make_dir_recursive(data_dir);
				if (dir->change_dir(data_dir) != OK) {
					ERR_PRINT("Could not create editor data directory: " + data_dir);
					paths_valid = false;
				}
			}

			if (!dir->dir_exists(export_templates_folder)) {
				dir->make_dir(export_templates_folder);
			}
		}

		// Config dir.
		{
			if (dir->change_dir(config_dir) != OK) {
				dir->make_dir_recursive(config_dir);
				if (dir->change_dir(config_dir) != OK) {
					ERR_PRINT("Could not create editor config directory: " + config_dir);
					paths_valid = false;
				}
			}

			if (!dir->dir_exists(text_editor_themes_folder)) {
				dir->make_dir(text_editor_themes_folder);
			}
			if (!dir->dir_exists(script_templates_folder)) {
				dir->make_dir(script_templates_folder);
			}
			if (!dir->dir_exists(feature_profiles_folder)) {
				dir->make_dir(feature_profiles_folder);
			}
		}

		// Cache dir.
		{
			if (dir->change_dir(cache_dir) != OK) {
				dir->make_dir_recursive(cache_dir);
				if (dir->change_dir(cache_dir) != OK) {
					ERR_PRINT("Could not create editor cache directory: " + cache_dir);
					paths_valid = false;
				}
			}
		}


	}
	bool EditorPaths::ArePathsVaild() const {
		return paths_valid;
	}

	String EditorPaths::GetDataDir() const {
		return data_dir;
	}

	String EditorPaths::GetConfigDir() const {
		return config_dir;
	}

	String EditorPaths::GetCacheDir() const {
		return cache_dir;
	}

	String EditorPaths::GetProjectDataDir() const {
		return project_data_dir;
	}

	bool EditorPaths::is_self_contained() const {
		return self_contained;
	}

	String EditorPaths::get_self_contained_file() const {
		return self_contained_file;
	}

}