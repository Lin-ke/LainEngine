#include "project_manager.h"
#include "editor_paths.h"
#include "core/config/config_parser.h"
#include "core/config/project_settings.h"
namespace lain {
	void ProjectList::update_project_list() {

	}
	ProjectList::ProjectList() {
		_config_path = EditorPaths::GetSingleton()->GetDataDir().path_join("projects.cfg");
		_migrate_config();


	}
	void ProjectList::_migrate_config() {
		if (FileAccess::exists(_config_path)) {
			return;
		}
		save_config();

	}
	void ProjectList::save_config() {
		
		_config.Save(_config_path);
	}

	void ProjectList::add_project(const String& dir_path, bool favorite) {
		if (!_config.has_section(dir_path)) {
			_config.set_value(dir_path, "favorite", favorite);
		}
	}
	void ProjectList::set_project_version(const String& p_project_path, int p_version) {
		for (ProjectList::Item& E : _projects) {
			if (E.path == p_project_path) {
				E.version = p_version;
				break;
			}
		}
	}

	ProjectList::Item ProjectList::load_project_data(const String& p_path, bool p_favorite) {
		String conf = p_path.path_join(ProjectSettings::PROJECT_FILE_NAME);
		bool grayed = false;
		bool missing = false;

		Ref<ConfigFile> cf = memnew(ConfigFile);
		Error cf_err = cf->Load(conf);

		int config_version = 0;
		//String project_name = TTR("Unnamed Project");
		String project_name = "Unnamed Project";

		if (cf_err == OK) {
			String cf_project_name = cf->get_value("application", "config/name", "");
			if (!cf_project_name.is_empty()) {
				project_name = cf_project_name.xml_unescape();
			}
			config_version = (int)cf->get_value("", "config_version", 0);
		}

		//if (config_version > ProjectSettings::CONFIG_VERSION) {
		//	// Comes from an incompatible (more recent) Godot version, gray it out.
		//	grayed = true;
		//}

		const String description = cf->get_value("application", "config/description", "");
		const PackedStringArray tags = cf->get_value("application", "config/tags", PackedStringArray());
		const String icon = cf->get_value("application", "config/icon", "");
		const String main_scene = cf->get_value("application", "run/main_scene", "");

		PackedStringArray project_features = cf->get_value("application", "config/features", PackedStringArray());
		//PackedStringArray unsupported_features = ProjectSettings::get_unsupported_features(project_features);
		PackedStringArray unsupported_features = PackedStringArray();
		uint64_t last_edited = 0;
		if (cf_err == OK) {
			// The modification date marks the date the project was last edited.
			// This is because the `project.godot` file will always be modified
			// when editing a project (but not when running it).
			last_edited = FileAccess::get_modified_time(conf);

			String fscache = p_path.path_join(".fscache");
			if (FileAccess::exists(fscache)) {
				uint64_t cache_modified = FileAccess::get_modified_time(fscache);
				if (cache_modified > last_edited) {
					last_edited = cache_modified;
				}
			}
		}
		else {
			grayed = true;
			missing = true;
			L_CORE_WARN("Project is missing: " + conf);
		}

		/*for (const String& tag : tags) {
			ProjectManager::get_singleton()->add_new_tag(tag);
		}*/

		return Item(project_name, description, tags, p_path, icon, main_scene, unsupported_features, last_edited, p_favorite, grayed, missing, config_version);
	}
}