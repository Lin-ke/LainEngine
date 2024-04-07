#include "class_db.h"
namespace lain {
	HashMap<StringName, StringName> ClassDB::resource_base_extensions;

	void ClassDB::add_resource_base_extension(const StringName& p_extension, const StringName& p_class) {
		if (resource_base_extensions.has(p_extension)) {
			return;
		}

		resource_base_extensions[p_extension] = p_class;
	}

	void ClassDB::get_resource_base_extensions(List<String>* p_extensions) {
		for (const KeyValue<StringName, StringName>& E : resource_base_extensions) {
			p_extensions->push_back(E.key);
		}
	}

}