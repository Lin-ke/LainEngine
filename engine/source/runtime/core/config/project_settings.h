#pragma once
#ifndef __PROJECT_SETTINGS_H__
#define __PROJECT_SETTINGS_H__
#include "base.h"
#include "core/string/ustring.h"
#include "core/string/string_name.h"
namespace lain {
	class ProjectSettings {
public:
	static ProjectSettings* p_singleton;
	Error Initialize(const String p_path);
	String project_data_dir_name;
	String resource_path;
	bool _set(const StringName& p_name, const Variant& p_value);
	bool _get(const StringName& p_name, Variant& r_ret) const;
	ProjectSettings* GetSingleTon() {
		return p_singleton;
	}
	ProjectSettings() {
		p_singleton = this;
	}
	~ProjectSettings() {
		p_singleton = nullptr;
	}
private:
	Error _initialize(const String p_path);
	};
}

#endif // !_PROJECT_SETTINGS_H__
