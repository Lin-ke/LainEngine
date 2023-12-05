#include "dir_access.h"
namespace lain {

DirAccess::CreateFunc DirAccess::create_func[ACCESS_MAX] = { nullptr, nullptr, nullptr };
Ref<DirAccess> DirAccess::create(AccessType p_access){
	Ref<DirAccess> da = create_func[p_access] ? create_func[p_access]() : nullptr;
	if (da.is_valid()) {
		da->m_access_type = p_access;

		// for ACCESS_RESOURCES and ACCESS_FILESYSTEM, current_dir already defaults to where game was started
		// in case current directory is force changed elsewhere for ACCESS_RESOURCES
		if (p_access == ACCESS_RESOURCES) {
			da->change_dir("res://");
		}
		else if (p_access == ACCESS_USERDATA) {
			da->change_dir("user://");
		}
	}

	return da;
}
}
