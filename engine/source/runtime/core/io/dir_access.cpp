#include "dir_access.h"
#include "core/config/project_settings.h"
#include "core/os/os.h"

namespace lain {

DirAccess::CreateFunc DirAccess::create_func[ACCESS_MAX] = { nullptr, nullptr, nullptr };
Ref<DirAccess> DirAccess::create(AccessType p_access){
	Ref<DirAccess> da = create_func[p_access] ? create_func[p_access]() : nullptr;
	if (da.is_valid()) {
		da->m_access_type = p_access;

		// for ACCESS_RESOURCES and ACCESS_FILESYSTEM, current_dir already defaults to where game was started
		// in case current directory is force changed elsewhere for ACCESS_RESOURCES
		if (p_access == ACCESS_RESOURCES) {
			da->change_dir("res://");
		}
		else if (p_access == ACCESS_USERDATA) {
			da->change_dir("user://");
		}
	}

	return da;
}

Ref<DirAccess> DirAccess::create_for_path(const String& p_path) {
	Ref<DirAccess> da;
	if (p_path.begins_with("res://")) {
		da = create(ACCESS_RESOURCES);
	}
	else if (p_path.begins_with("user://")) {
		da = create(ACCESS_USERDATA);
	}
	else {
		da = create(ACCESS_FILESYSTEM);
	}

	return da;
}

String DirAccess::_get_root_path() const {
	switch (m_access_type) {
	case ACCESS_RESOURCES:
		return ProjectSettings::GetSingleton()->GetResourcePath();
	case ACCESS_USERDATA:
		return OS::GetSingleton()->GetUserDataDir();
	default:
		return "";
	}
}


String DirAccess::fix_path(const String&p_path) const {
	switch (m_access_type) {
	case ACCESS_RESOURCES: {
		if (ProjectSettings::GetSingleton()) {
			if (p_path.begins_with("res://")) {
				String resource_path = ProjectSettings::GetSingleton()->GetResourcePath();
				if (!resource_path.is_empty()) {
					return p_path.replace_first("res:/", resource_path);
				}
				return p_path.replace_first("res://", "");
			}
		}

	} break;
	case ACCESS_USERDATA: {
		if (p_path.begins_with("user://")) {
			String data_dir = OS::GetSingleton()->GetUserDataDir();
			if (!data_dir.is_empty()) {
				return p_path.replace_first("user:/", data_dir);
			}
			return p_path.replace_first("user://", "");
		}

	} break;
	case ACCESS_FILESYSTEM: {
		return p_path;
	} break;
	case ACCESS_MAX:
		break; // Can't happen, but silences warning
	}

	return p_path;
}
Ref<DirAccess> DirAccess::_open(const String& p_path) {
	Error err = OK;
	Ref<DirAccess> da = open(p_path, &err);
	last_dir_open_error = err;
	if (err) {
		return Ref<DirAccess>();
	}
	return da;
}
Ref<DirAccess> DirAccess::open(const String& p_path, Error* r_error) {
	Ref<DirAccess> da = create_for_path(p_path);
	ERR_FAIL_COND_V_MSG(da.is_null(), nullptr, "Cannot create DirAccess for path '" + p_path + "'.");
	Error err = da->change_dir(p_path);
	if (r_error) {
		*r_error = err;
	}
	if (err != OK) {
		return nullptr;
	}

	return da;
}

}
