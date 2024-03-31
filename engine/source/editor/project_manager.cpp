#include "project_manager.h"
#include "editor_paths.h"
#include "core/config/config_parser.h"
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
}