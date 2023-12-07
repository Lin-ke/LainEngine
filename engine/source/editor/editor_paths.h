#pragma once
#ifndef EDITOR_PATHS_H
#define EDITOR_PATHS_H
#include "core/string/ustring.h"
namespace lain {
	class EditorPaths {
	private:

		static EditorPaths* p_singleton;
		bool paths_valid = false; // If any of the paths can't be created, this is false.
		String data_dir; // Editor data (templates, shader cache, etc.).
		String config_dir; // Editor config (settings, profiles, themes, etc.).
		String cache_dir; // Editor cache (thumbnails, tmp generated files).
		String project_data_dir; // Project-specific data (metadata, shader cache, etc.).
		bool self_contained = false; // Self-contained means everything goes to `editor_data` dir.
		String self_contained_file; // Self-contained file with configuration.
		String export_templates_folder = "export_templates";
		String text_editor_themes_folder = "text_editor_themes";
		String script_templates_folder = "script_templates";
		String feature_profiles_folder = "feature_profiles";
	public:
		static EditorPaths* GetSingleton() { return p_singleton; }
		bool are_paths_valid() const;
		String get_data_dir() const;
		String get_config_dir() const;
		String get_cache_dir() const;
		String get_project_data_dir() const;
		String get_export_templates_dir() const;
		String get_project_settings_dir() const;
		String get_text_editor_themes_dir() const;
		String get_script_templates_dir() const;
		String get_project_script_templates_dir() const;
		String get_feature_profiles_dir() const;
		bool is_self_contained() const;
		String get_self_contained_file() const;

		static void create();
		static void free();
		EditorPaths();
		~EditorPaths() { p_singleton = nullptr; }
	};
}

#endif