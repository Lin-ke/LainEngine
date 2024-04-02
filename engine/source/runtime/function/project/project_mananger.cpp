//#include "project_manager.h"
//#include "core/os/memory.h"
//#include "core/meta/serializer/serializer.h"
//#include "_generated/serializer/all_serializer.h"
//
//namespace lain {
//	ProjectList::ProjectList(){
//	}
//	ProjectManager* ProjectManager::p_singleton = nullptr;
//	ProjectManager::ProjectManager() {
//		p_singleton = this;
//		m_project_list = memnew(ProjectList);
//		// load json
//		Serializer::read(Json(), *m_project_list);
//	}
//	ProjectManager::~ProjectManager() {
//		p_singleton = nullptr;
//	}
//
//}