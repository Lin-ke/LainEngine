#include "signal.h"
namespace lain {
	Signal::operator String() const {
		/*Object* base = get_object();
		if (base) {
			String class_name = base->get_class();
			Ref<Script> script = base->get_script();
			if (script.is_valid() && script->get_path().is_resource_file()) {
				class_name += "(" + script->get_path().get_file() + ")";
			}
			return class_name + "::[signal]" + String(name);
		}
		else {
			return "null::[signal]" + String(name);
		}*/
		return "";
	}
}