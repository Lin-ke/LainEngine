#include "project_settings.h"
#include "core/error/error.h"
#include "core/os/dir_access.h"
namespace lain {
	ProjectSettings* ProjectSettings::p_singleton = nullptr;

	Error ProjectSettings::Initialize(const String p_path) {
		Error err = _initialize(p_path);
		if (OK == err) {
		}
		

	}


	/// private
    // TODO: use own filesystem
	Error ProjectSettings::_initialize(const String config_file_path) {
		Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
    }
	}
}