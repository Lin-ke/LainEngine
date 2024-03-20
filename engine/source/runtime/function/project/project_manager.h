#pragma once
#include "core/meta/reflection/reflection.h"
#include "core/string/ustring.h"
namespace lain {
	REFLECTION_TYPE(ProjectListItem)
	CLASS(ProjectListItem, Fields) {
		REFLECTION_BODY(ProjectListItem);
	public :
		ProjectListItem(const String p_name,
			const String p_description,
			const String p_path,
			const String p_main_scene,
			const ui64    p_last_edited) {
			m_project_name = p_name;
			m_description = p_description;
			m_path = p_path;
			m_main_scene = p_main_scene;
			m_last_edited = p_last_edited;
		}
		ProjectListItem() {}
	private:
		String m_project_name;
		String m_description;
		String m_path;
		String m_main_scene;
		ui64 m_last_edited = 0; 
		

	};
	REFLECTION_TYPE(ProjectList)
	CLASS(ProjectList,Fields){
		REFLECTION_BODY(ProjectList);
public: ProjectList();
	private:
		Vector<ProjectListItem> m_projects;

	};
	class ProjectManager {
	private:
		ProjectList* m_project_list = nullptr;
		static ProjectManager* p_singleton;
	public:
		void load_projects();
		int get_project_count() const;
		void add_project(const String& dir_path, bool favorite);
		void select_project(int p_index);

		static ProjectManager* GetSingleton() {
			return p_singleton;
		}
		ProjectManager();
		~ProjectManager();
	};


}

