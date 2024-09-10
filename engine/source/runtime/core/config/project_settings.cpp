#include "project_settings.h"
#include "base.h"
#include "config_parser.h"
#include "core/error/error_macros.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/os/mutex.h"
#include "core/os/os.h"

#define OSRESDIR OS::GetSingleton()->GetResourceDir()

using namespace lain;
ProjectSettings* ProjectSettings::p_singleton = nullptr;
const String ProjectSettings::PROJECT_DATA_DIR_NAME_SUFFIX = "prjdata";
const String ProjectSettings::PROJECT_FILE_NAME = "prj.txt";
const String ProjectSettings::PROJECT_BINARY_NAME = "prj.data";
const String ProjectSettings::ALL_PROJECTS_FILE_NAME = "prj.config";

Error ProjectSettings::Initialize(const String p_path) {
  Error err = _initialize(p_path);
  if (OK == err) {
    L_CORE_INFO("Successful init projectSettings.");
    project_data_dir_name = PROJECT_DATA_DIR_NAME_SUFFIX;
  }

  return OK;
}
const PackedStringArray ProjectSettings::get_required_features() {
  PackedStringArray features;
  features.append(VERSION_BRANCH);
  return features;
}

String ProjectSettings::GlobalizePath(const String& p_path) const {
  if (p_path.begins_with("res://")) {
    if (!resource_path.is_empty()) {
      return p_path.replace("res:/", resource_path);
    }
    return p_path.replace("res://", "");
  } else if (p_path.begins_with("user://")) {
    String data_dir = OS::GetSingleton()->GetUserDataDir();
    if (!data_dir.is_empty()) {
      return p_path.replace("user:/", data_dir);
    }
    return p_path.replace("user://", "");
  }

  return p_path;
}

String ProjectSettings::LocalizePath(const String& p_path) const {
  if (p_path.begins_with("res://"))
    return p_path;
  String path = p_path.simplify_path();

  if (resource_path.is_empty() || (path.is_absolute_path() && !path.begins_with(resource_path))) {
    return path;
  }

  // Check if we have a special path (like res://) or a protocol identifier.
  int p = path.find("://");
  bool found = false;
  if (p > 0) {
    found = true;
    for (int i = 0; i < p; i++) {
      if (!is_ascii_alphanumeric_char(path[i])) {
        found = false;
        break;
      }
    }
  }
  if (found) {
    return path;
  }

  Ref<DirAccess> dir = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);

  if (dir->change_dir(path) == OK) {
    String cwd = dir->get_current_dir();
    cwd = cwd.replace("\\", "/");

    // Ensure that we end with a '/'.
    // This is important to ensure that we do not wrongly localize the resource path
    // in an absolute path that just happens to contain this string but points to a
    // different folder (e.g. "/my/project" as resource_path would be contained in
    // "/my/project_data", even though the latter is not part of res://.
    // `path_join("")` is an easy way to ensure we have a trailing '/'.
    const String res_path = resource_path.path_join("");

    // DirAccess::get_current_dir() is not guaranteed to return a path that with a trailing '/',
    // so we must make sure we have it as well in order to compare with 'res_path'.
    cwd = cwd.path_join("");

    if (!cwd.begins_with(res_path)) {
      return path;
    }

    return cwd.replace_first(res_path, "res://");
  } else {
    int sep = path.rfind("/");
    if (sep == -1) {
      return "res://" + path;
    }

    String parent = path.substr(0, sep);

    String plocal = LocalizePath(parent);
    if (plocal.is_empty()) {
      return "";
    }
    // Only strip the starting '/' from 'path' if its parent ('plocal') ends with '/'
    if (plocal[plocal.length() - 1] == '/') {
      sep += 1;
    }
    return plocal + path.substr(sep, path.size() - sep);
  }
}

/// private
// Ã»³õÊ¼»¯  OSRESDIR = ""

Error ProjectSettings::_initialize(const String p_path, bool p_ignore_override) {
  if (!OSRESDIR.is_empty()) {
    // OS (default) will call ProjectSettings->get_resource_path which will be empty if not overridden!
    // If the OS would rather use a specific location, then it will not be empty.
    resource_path = OSRESDIR.replace("\\", "/");
    if (!resource_path.is_empty() && resource_path[resource_path.length() - 1] == '/') {
      resource_path = resource_path.substr(0, resource_path.length() - 1);  // Chop end.
    }
  }
  // Try to use the filesystem for files, according to OS.
  // (Only Android -when reading from pck- and iOS use this.)
  if (!OSRESDIR.is_empty()) {  // if resdir is set, open it .
    Error err = _load_settings_text_or_binary("res://" + PROJECT_FILE_NAME, "");
    if (err == OK && !p_ignore_override) {
      // Optional, we don't mind if it fails.
      _load_settings_text("res://override.cfg");
    }
    return err;
  }
  // Nothing was found, try to find a project file in provided path (`p_path`)
  // or, if requested (`p_upwards`) in parent directories.
  Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
  ERR_FAIL_COND_V_MSG(d.is_null(), ERR_CANT_CREATE, "Cannot create DirAccess for path '" + p_path + "'.");
  d->change_dir(p_path);

  String current_dir = d->get_current_dir();
  bool found = false;
  Error err;
  while (true) {
    // Set the resource path early so things can be resolved when loading.
    resource_path = current_dir;
    resource_path = resource_path.replace("\\", "/");  // Windows path to Unix path just in case.
    err = _load_settings_text_or_binary(current_dir.path_join(PROJECT_FILE_NAME), current_dir.path_join(PROJECT_BINARY_NAME));
    if (err == OK && !p_ignore_override) {
      // Optional, we don't mind if it fails.
      _load_settings_text(current_dir.path_join("override.cfg"));
      found = true;
      break;
    }

    //if (p_upwards) {
    //	// Try to load settings ascending through parent directories
    //	d->change_dir("..");
    //	if (d->get_current_dir() == current_dir) {
    //		break; // not doing anything useful
    //	}
    //	current_dir = d->get_current_dir();
    //}
    else {
      break;
    }
  }

  if (!found) {
    return err;
  }

  if (resource_path.length() && resource_path[resource_path.length() - 1] == '/') {
    resource_path = resource_path.substr(0, resource_path.length() - 1);  // Chop end.
  }

  return OK;
}
Error ProjectSettings::_load_settings_text_or_binary(const String& p_text_path, const String& p_bin_path) {

  Error err = _load_settings_text(p_text_path);
  if (err == OK) {
    return OK;
  } else if (err != ERR_FILE_NOT_FOUND) {
    ERR_PRINT("Couldn't load file '" + p_text_path + "', error code " + itos(err) + ".");
  }

  // Fallback to text-based binary file if binary was not found.
  err = _load_settings_binary(p_bin_path);
  if (err == OK) {
    return OK;
  } else if (err != ERR_FILE_NOT_FOUND) {
    // If the file exists but can't be loaded, we want to know it.
    ERR_PRINT("Couldn't load file '" + p_bin_path + "', error code " + itos(err) + ".");
  }

  return err;
}

Error ProjectSettings::_load_settings_text(const String& p_path) {
  Error err;
  Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ, &err);
  auto cp = ConfigFile();

  if (f.is_null()) {
    // FIXME: Above 'err' error code is ERR_FILE_CANT_OPEN if the file is missing
    // This needs to be streamlined if we want decent error reporting
    return ERR_FILE_NOT_FOUND;
  }

  cp.ParseFile(f);
  for (const KeyValue<String, HashMap<String, Variant>>& E : cp.values) {
    String section = E.key;
    for (const KeyValue<String, Variant>& F : E.value) {
      if (section.is_empty() && F.key == "config_version") {
        // config_version
      } else {
        if (section.is_empty()) {
          Set(F.key, F.value);
        } else {
          Set(section + "/" + F.key, F.value);
        }
      }
    }
  }

  return OK;
}
Error ProjectSettings::_load_settings_binary(const String& text_path) {
  return OK;
}

/*
	* TODO
	*/
bool ProjectSettings::Set(const StringName& p_name, const Variant& p_value) {
  _THREAD_SAFE_METHOD_

  if (p_value.get_type() == Variant::NIL) {
    props.erase(p_name);
  } else {
    if (props.has(p_name)) {
      props[p_name].variant = p_value;
    } else {
      props[p_name] = VariantContainer(p_value, last_order++);
    }
  }

  return true;
}
bool ProjectSettings::Get(const StringName& p_name, Variant& r_ret) const {
  _THREAD_SAFE_METHOD_

  if (!props.has(p_name)) {
    WARN_PRINT("Property not found: " + String(p_name));
    return false;
  }
  r_ret = props[p_name].variant;
  return true;
}

bool lain::ProjectSettings::Has(const StringName& p_name) const {
	_THREAD_SAFE_METHOD_
	return props.has(p_name);
}

Variant ProjectSettings::GetSetting(const StringName& p_name) const {
  _THREAD_SAFE_METHOD_
  StringName name = p_name;
  if (!props.has(name)) {
    WARN_PRINT("Property not found: " + String(name));
    return Variant();
  }
  return props[name].variant;
}


void ProjectSettings::set_initial_value(const String &p_name, const Variant &p_value) {
	ERR_FAIL_COND_MSG(!props.has(p_name), "Request for nonexistent project setting: " + p_name + ".");

	// Duplicate so that if value is array or dictionary, changing the setting will not change the stored initial value.
	props[p_name].initial = p_value.duplicate();
}

void ProjectSettings::set_restart_if_changed(const String &p_name, bool p_restart) {
	ERR_FAIL_COND_MSG(!props.has(p_name), "Request for nonexistent project setting: " + p_name + ".");
	props[p_name].restart_if_changed = p_restart;
}

void ProjectSettings::set_as_basic(const String &p_name, bool p_basic) {
	ERR_FAIL_COND_MSG(!props.has(p_name), "Request for nonexistent project setting: " + p_name + ".");
	props[p_name].basic = p_basic;
}

void ProjectSettings::set_as_internal(const String &p_name, bool p_internal) {
	ERR_FAIL_COND_MSG(!props.has(p_name), "Request for nonexistent project setting: " + p_name + ".");
	props[p_name].internal = p_internal;
}

void ProjectSettings::set_ignore_value_in_docs(const String &p_name, bool p_ignore) {
	ERR_FAIL_COND_MSG(!props.has(p_name), "Request for nonexistent project setting: " + p_name + ".");
#ifdef DEBUG_METHODS_ENABLED
	props[p_name].ignore_value_in_docs = p_ignore;
#endif
}
bool ProjectSettings::get_ignore_value_in_docs(const String &p_name) const {
	ERR_FAIL_COND_V_MSG(!props.has(p_name), false, "Request for nonexistent project setting: " + p_name + ".");
#ifdef DEBUG_METHODS_ENABLED
	return props[p_name].ignore_value_in_docs;
#else
	return false;
#endif
}

void ProjectSettings::set_builtin_order(const String &p_name) {
	ERR_FAIL_COND_MSG(!props.has(p_name), "Request for nonexistent project setting: " + p_name + ".");
	if (props[p_name].order >= NO_BUILTIN_ORDER_BASE) {
		props[p_name].order = last_builtin_order++;
	}
}
Variant lain::_GLOBAL_DEF(const String& p_var, const Variant& p_default, bool p_restart_if_changed, bool p_ignore_value_in_docs, bool p_basic, bool p_internal) {
  Variant ret;
	if (!ProjectSettings::GetSingleton()->Has(p_var)) {
		ProjectSettings::GetSingleton()->Set(p_var, p_default);
	}
	ret = GLOBAL_GET(p_var);

	ProjectSettings::GetSingleton()->set_initial_value(p_var, p_default);
	ProjectSettings::GetSingleton()->set_builtin_order(p_var);
	ProjectSettings::GetSingleton()->set_as_basic(p_var, p_basic);
	ProjectSettings::GetSingleton()->set_restart_if_changed(p_var, p_restart_if_changed);
	ProjectSettings::GetSingleton()->set_ignore_value_in_docs(p_var, p_ignore_value_in_docs);
	ProjectSettings::GetSingleton()->set_as_internal(p_var, p_internal);
	return ret;
}

void ProjectSettings::set_custom_property_info(const PropertyInfo &p_info) {
	const String &prop_name = p_info.name;
	ERR_FAIL_COND(!props.has(prop_name));
	custom_prop_info[prop_name] = p_info;
}

Variant lain::_GLOBAL_DEF(const PropertyInfo & p_info, const Variant & p_default, bool p_restart_if_changed, bool p_ignore_value_in_docs, bool p_basic, bool p_internal)
{
	Variant ret = _GLOBAL_DEF(p_info.name, p_default, p_restart_if_changed, p_ignore_value_in_docs, p_basic, p_internal);
	ProjectSettings::GetSingleton()->set_custom_property_info(p_info);
	return ret;
}
# undef OSRESDIR