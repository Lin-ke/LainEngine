#pragma once
#ifndef PROJECT_MANAGER_H
#define PROJECT_MANAGER_H
#include "core/io/file_access.h"
#include "core/io/dir_access.h"

namespace lain {
	class ProjectList {
	public:
		ProjectList();
		~ProjectList();
		void update_project_list();

		struct Item {
			String project_name;
			String description;
			PackedStringArray tags;
			String tag_sort_string;
			String path;
			String icon;
			String main_scene;
			PackedStringArray unsupported_features;
			uint64_t last_edited = 0;
			bool favorite = false;
			bool grayed = false;
			bool missing = false;
			int version = 0;

			//ProjectListItemControl* control = nullptr;

			Item() {}

			Item(const String& p_name,
				const String& p_description,
				const PackedStringArray& p_tags,
				const String& p_path,
				const String& p_icon,
				const String& p_main_scene,
				const PackedStringArray& p_unsupported_features,
				uint64_t p_last_edited,
				bool p_favorite,
				bool p_grayed,
				bool p_missing,
				int p_version) {
				project_name = p_name;
				description = p_description;
				tags = p_tags;
				path = p_path;
				icon = p_icon;
				main_scene = p_main_scene;
				unsupported_features = p_unsupported_features;
				last_edited = p_last_edited;
				favorite = p_favorite;
				grayed = p_grayed;
				missing = p_missing;
				version = p_version;

				//control = nullptr;

				PackedStringArray sorted_tags = tags;
				sorted_tags.sort();
				tag_sort_string = String().join(sorted_tags);
			}

			_FORCE_INLINE_ bool operator==(const Item& l) const {
				return path == l.path;
			}
		};

	private:
		String _config_path;
		Vector<Item> _projects;
		void _migrate_config();
		static Item load_project_data(const String& p_property_key, bool p_favorite);
	};
}
#endif // !PROJECT_MANAGER_H
