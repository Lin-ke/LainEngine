#pragma once
#ifndef __PROJECT_SETTINGS_H__
#define __PROJECT_SETTINGS_H__
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "core/string/string_name.h"
namespace lain {
	class ProjectSettings {
public:
	static ProjectSettings* p_singleton;
	Error Initialize(const String p_path);
	String project_data_dir_name;
	String resource_path = "";
	bool _set(const StringName& p_name, const Variant& p_value);
	bool _get(const StringName& p_name, Variant& r_ret) const;
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
	 String GetResourcePath() const {
		return resource_path;
	}
private:
	Error _initialize(const String p_path, bool p_ignore_override = true);
	// load settings
	Error _load_settings_text_or_binary(const String& p_text_path, const String& p_binary_path);
	Error _load_settings_text(const String& text_path);
	Error _load_settings_binary(const String& binary_path);
	
	};


}

#endif // !_PROJECT_SETTINGS_H__
