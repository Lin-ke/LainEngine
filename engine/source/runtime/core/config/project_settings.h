#pragma once
#ifndef __PROJECT_SETTINGS_H__
#define __PROJECT_SETTINGS_H__
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "core/templates/hash_map.h"
#include "core/string/string_name.h"
#include "core/templates/hash_set.h"
#include "core/io/file_access.h"

// TODO:
#define GLOBAL_GET(m_var) ProjectSettings::GetSingleton()->GetSetting(m_var)

namespace lain {
	// config manager
	class ProjectSettings {
public:
	HashSet<String> custom_features;
	HashMap<StringName, std::vector<Pair<StringName, StringName>>> feature_overrides;
	static ProjectSettings* p_singleton;
	Error Initialize(const String p_path);
    static const String PROJECT_DATA_DIR_NAME_SUFFIX;
    static const String PROJECT_FILE_NAME;
	static const String PROJECT_BINARY_NAME;
	static const String ALL_PROJECTS_FILE_NAME;



	String project_data_dir_name = "";
	String resource_path = "";
	bool _set(const StringName& p_name, const Variant& p_value);
	bool _get(const StringName& p_name, Variant& r_ret) const;
	Variant GetWithOverride(const StringName& p_name) const;
	String GlobalizePath(const String& path) const;
    String LocalizePath(const String& path) const;
	/// <summary>
	/// features
	/// </summary>
	/// <returns></returns>
	const PackedStringArray get_required_features();
	static ProjectSettings* GetSingleton() {
		return p_singleton;
	}
	ProjectSettings() {
		p_singleton = this;
	}
	~ProjectSettings() {
		p_singleton = nullptr;
	}
    String GetSetting(const StringName& p_name) const;
	String GetResourcePath() const { return resource_path; }
    String GetProjectDataName() const { return project_data_dir_name; }
	String GetProjectDataPath() const { return "res://" + project_data_dir_name; }
private:
	Error _initialize(const String p_path, bool p_ignore_override = true);
	// load settings
	Error _load_settings_text_or_binary(const String& p_text_path, const String& p_binary_path);
	Error _load_settings_text(const String& text_path);
	Error _load_settings_binary(const String& binary_path);
	
	};

   

}

#endif // !_PROJECT_SETTINGS_H__
