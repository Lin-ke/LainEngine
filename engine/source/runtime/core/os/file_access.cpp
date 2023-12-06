#include "core/config/project_settings.h"
#include "core/os/os.h"
#include "file_access.h"

namespace lain {
	FileAccess::CreateFunc FileAccess::create_func[ACCESS_MAX] = { nullptr, nullptr, nullptr };

	Ref<FileAccess> FileAccess::open(const String& p_path, int p_mode_flags, Error* r_error) {
		//try packed data first

		Ref<FileAccess> ret;
		/*if (!(p_mode_flags & WRITE) && PackedData::get_singleton() && !PackedData::get_singleton()->is_disabled()) {
			ret = PackedData::get_singleton()->try_open_path(p_path);
			if (ret.is_valid()) {
				if (r_error) {
					*r_error = OK;
				}
				return ret;
			}
		}*/

		ret = create_for_path(p_path);
		Error err = ret->open_internal(p_path, p_mode_flags);

		if (r_error) {
			*r_error = err;
		}
		if (err != OK) {
			ret.unref();
		}

		return ret;
	}
	Ref<FileAccess> FileAccess::create_for_path(const String& p_path) {
		Ref<FileAccess> ret;
		if (p_path.begins_with("res://")) {
			ret = create(ACCESS_RESOURCES);
		}
		else if (p_path.begins_with("user://")) {
			ret = create(ACCESS_USERDATA);

		}
		else {
			ret = create(ACCESS_FILESYSTEM);
		}

		return ret;
	}

	Ref<FileAccess> FileAccess::create(AccessType p_access) {
		ERR_FAIL_INDEX_V(p_access, ACCESS_MAX, nullptr);

		Ref<FileAccess> ret = create_func[p_access]();
		ret->_set_access_type(p_access);
		return ret;
	}

	void FileAccess::_set_access_type(AccessType p_access) {
		_access_type = p_access;
	}

	FileAccess::AccessType FileAccess::get_access_type() const {
		return _access_type;
	}

	/// <summary>
	/// turn relative path(res:// to real urls.)
	/// </summary>
	/// <param name="p_path"></param>
	/// <returns></returns>
	String FileAccess::fix_path(const String& p_path) const {
		//helper used by file accesses that use a single filesystem

		String r_path = p_path.replace("\\", "/");

		switch (_access_type) {
		case ACCESS_RESOURCES: {
			if (ProjectSettings::GetSingleton()) {
				if (r_path.begins_with("res://")) {
					String resource_path = ProjectSettings::GetSingleton()->GetResourcePath();
					if (!resource_path.is_empty()) {
						return r_path.replace("res:/", resource_path);
					}
					return r_path.replace("res://", "");
				}
			}

		} break;
		case ACCESS_USERDATA: {
			if (r_path.begins_with("user://")) {
				String data_dir = OS::GetSingleton()->GetUserDataDir();
				if (!data_dir.is_empty()) {
					return r_path.replace("user:/", data_dir);
				}
				return r_path.replace("user://", "");
			}

		} break;
		case ACCESS_FILESYSTEM: {
			return r_path;
		} break;
		case ACCESS_MAX:
			break; // Can't happen, but silences warning
		}

		return r_path;
	}

}