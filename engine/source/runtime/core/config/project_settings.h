#pragma once
#ifndef __PROJECT_SETTINGS_H__
#define __PROJECT_SETTINGS_H__
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "core/templates/hash_map.h"
#include "core/string/string_name.h"
#include "core/templates/hash_set.h"
#include "core/io/file_access.h"
#include "core/os/thread_safe.h"
#include "core/templates/rb_map.h"

// TODO:
#define GLOBAL_GET(m_var) ProjectSettings::GetSingleton()->GetSetting(m_var)
namespace lain {
	// config manager
	class ProjectSettings:public Object {
		LCLASS(ProjectSettings, Object);
		_THREAD_SAFE_CLASS_
public:
	Error Initialize(const String p_path);
    static const String PROJECT_DATA_DIR_NAME_SUFFIX;
    static const String PROJECT_FILE_NAME;
	static const String PROJECT_BINARY_NAME;
	static const String ALL_PROJECTS_FILE_NAME;
	enum {
		// Properties that are not for built in values begin from this value, so builtin ones are displayed first.
		NO_BUILTIN_ORDER_BASE = 1 << 16
	};
	
	
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
    Variant GetSetting(const StringName& p_name) const;
	String GetResourcePath() const { return resource_path; }
    String GetProjectDataName() const { return project_data_dir_name; }
	String GetProjectDataPath() const { return "res://" + project_data_dir_name; }
	Error Save();
	
	void set_initial_value(const String &p_name, const Variant &p_value);
	void set_as_basic(const String &p_name, bool p_basic);
	void set_as_internal(const String &p_name, bool p_internal);
	void set_restart_if_changed(const String &p_name, bool p_restart);
	void set_ignore_value_in_docs(const String &p_name, bool p_ignore);
	bool get_ignore_value_in_docs(const String &p_name) const;
	void set_builtin_order(const String &p_name);

protected:
	HashSet<String> custom_features;
	HashMap<StringName, std::vector<Pair<StringName, StringName>>> feature_overrides;

	struct VariantContainer {
		int order = 0;
		bool persist = false;
		bool basic = false;
		bool internal = false;
		Variant variant;
		Variant initial;
		bool hide_from_editor = false;
		bool restart_if_changed = false;
#ifdef DEBUG_METHODS_ENABLED
		bool ignore_value_in_docs = false;
#endif

		VariantContainer() {}

		VariantContainer(const Variant& p_variant, int p_order, bool p_persist = false) :
			order(p_order),
			persist(p_persist),
			variant(p_variant) {
		}
	};
	int last_order = NO_BUILTIN_ORDER_BASE;
	int last_builtin_order = 0;

	RBMap<StringName, VariantContainer> props; // NOTE: Key order is used e.g. in the save_custom method.
	String project_data_dir_name = "";
	String resource_path = "";
public:
	bool Set(const StringName& p_name, const Variant& p_value);
	bool Get(const StringName& p_name, Variant& r_ret) const;
	bool Has(const StringName& p_name) const;
private:
	Error _initialize(const String p_path, bool p_ignore_override = true);
	// load settings
	Error _load_settings_text_or_binary(const String& p_text_path, const String& p_binary_path);
	Error _load_settings_text(const String& text_path);
	Error _load_settings_binary(const String& binary_path);
	
	static ProjectSettings* p_singleton;

	};

Variant _GLOBAL_DEF(const String &p_var, const Variant &p_default, bool p_restart_if_changed = false, bool p_ignore_value_in_docs = false, bool p_basic = false, bool p_internal = false);
// Variant _GLOBAL_DEF(const PropertyInfo &p_info, const Variant &p_default, bool p_restart_if_changed = false, bool p_ignore_value_in_docs = false, bool p_basic = false, bool p_internal = false);

#define GLOBAL_DEF(m_var, m_value) _GLOBAL_DEF(m_var, m_value)
#define GLOBAL_DEF_RST(m_var, m_value) _GLOBAL_DEF(m_var, m_value, true)
#define GLOBAL_DEF_NOVAL(m_var, m_value) _GLOBAL_DEF(m_var, m_value, false, true)
#define GLOBAL_DEF_RST_NOVAL(m_var, m_value) _GLOBAL_DEF(m_var, m_value, true, true)
}

#endif // !_PROJECT_SETTINGS_H__
