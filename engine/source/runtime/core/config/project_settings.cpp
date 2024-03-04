#include "project_settings.h"
#include "base.h"
#include "core/error/error_macros.h"
#include "core/os/os.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"

#define OSRESDIR OS::GetSingleton()->GetResourceDir()

namespace lain {
	ProjectSettings* ProjectSettings::p_singleton = nullptr;

	Error ProjectSettings::Initialize(const String p_path) {
		Error err = _initialize(p_path);
		if (OK == err) {
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
		}
		else if (p_path.begins_with("user://")) {
			String data_dir = OS::GetSingleton()->GetUserDataDir();
			if (!data_dir.is_empty()) {
				return p_path.replace("user:/", data_dir);
			}
			return p_path.replace("user://", "");
		}

		return p_path;
	}

	/// private
	Error ProjectSettings::_initialize(const String p_path, bool p_ignore_override) {
		//if (!OSRESDIR.is_empty()) {
		//	resource_path = OSRESDIR.replace("\\", "/");
		//	if (!resource_path.is_empty() && resource_path[resource_path.length() - 1] == '/') {
		//		resource_path = resource_path.substr(0, resource_path.length() - 1); // Chop end.
		//	}
		//}
		//// Try to use the filesystem for files, according to OS.
		//// (Only Android -when reading from pck- and iOS use this.)
		//if (!OSRESDIR.is_empty()) { // if resdir is set, open it .
		//	Error err = _load_settings_text_or_binary("res://project.lain", "");
		//	if (err == OK && !p_ignore_override) {
		//		// Optional, we don't mind if it fails.
		//		_load_settings_text("res://override.cfg");
		//	}
		//	return err;
		//}
		//// Nothing was found, try to find a project file in provided path (`p_path`)
		//// or, if requested (`p_upwards`) in parent directories.
		//Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
		//ERR_FAIL_COND_V_MSG(d.is_null(), ERR_CANT_CREATE, "Cannot create DirAccess for path '" + p_path + "'.");
		//d->change_dir(p_path);

		//String current_dir = d->get_current_dir();
		//L_PRINT(current_dir);
		//bool found = false;
		//Error err;
		//while (true) {
		//	// Set the resource path early so things can be resolved when loading.
		//	resource_path = current_dir;
		//	resource_path = resource_path.replace("\\", "/"); // Windows path to Unix path just in case.
		//	err = _load_settings_text_or_binary(current_dir.path_join("project.godot"), current_dir.path_join("project.binary"));
		//	if (err == OK && !p_ignore_override) {
		//		// Optional, we don't mind if it fails.
		//		_load_settings_text(current_dir.path_join("override.cfg"));
		//		found = true;
		//		break;
		//	}

		//	//if (p_upwards) {
		//	//	// Try to load settings ascending through parent directories
		//	//	d->change_dir("..");
		//	//	if (d->get_current_dir() == current_dir) {
		//	//		break; // not doing anything useful
		//	//	}
		//	//	current_dir = d->get_current_dir();
		//	//}
		//	else {
		//		break;
		//	}
		//}

		//if (!found) {
		//	return err;
		//}

		//if (resource_path.length() && resource_path[resource_path.length() - 1] == '/') {
		//	resource_path = resource_path.substr(0, resource_path.length() - 1); // Chop end.
		//}

		return OK;
	}
	Error ProjectSettings::_load_settings_text_or_binary(const String& p_text_path, const String& p_bin_path) {
		Error err_text, err_binary = OK;
		if (p_text_path != "") {
			err_text = _load_settings_text(p_text_path);
			if (err_text == OK) { return OK; }
			// If the file exists but can't be loaded, we want to know it.
			else if (err_text != ERR_FILE_NOT_FOUND) {
				ERR_PRINT(("Couldn't load file '" + p_bin_path + "', error code " + itos(err_text) + "."));
			}
		}
		if (p_bin_path != "") {
			err_binary = _load_settings_binary(p_bin_path);
			if (err_binary == OK) { return OK; }
			else if (err_binary != ERR_FILE_NOT_FOUND) {
				ERR_PRINT(("Couldn't load file '" + p_bin_path + "', error code " + itos(err_binary) + "."));
			}
		}
		return (err_text == OK && err_binary == OK) ? OK : ERR_FILE_NOT_FOUND;
	}
	
	Error ProjectSettings::_load_settings_text(const String& p_path) { 
		Error err;
		Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ, &err);
		auto cp = ConfigParser();

		if (f.is_null()) {
			cp.ParseFile("proj.setting");
			auto hashset = cp.m_hashmap;

			// FIXME: Above 'err' error code is ERR_FILE_CANT_OPEN if the file is missing
			// This needs to be streamlined if we want decent error reporting
			return ERR_FILE_NOT_FOUND;
		}
	}
	Error ProjectSettings::_load_settings_binary(const String& text_path) { return OK; }
	
	/*
	* TODO
	*/
	String ProjectSettings::GetSetting(const StringName& p_name) const {
		return "";
	}


}