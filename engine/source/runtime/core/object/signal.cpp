#include "signal.h"
#include "core/object/objectdb.h"
#include "core/string/string_name.h"
using namespace lain;
bool Signal::is_null() const {
  return object.is_null() && name == StringName();
}
Object* Signal::get_object() const {
  return ObjectDB::get_instance(object);
}

bool Signal::operator==(const Signal& p_signal) const {
  return object == p_signal.object && name == p_signal.name;
}

bool Signal::operator!=(const Signal& p_signal) const {
  return object != p_signal.object || name != p_signal.name;
}

bool Signal::operator<(const Signal& p_signal) const {
  if (object == p_signal.object) {
    return name < p_signal.name;
  } else {
    return object < p_signal.object;
  }
}

Signal::Signal(const Object* p_object, const StringName& p_name) {
  ERR_FAIL_NULL_MSG(p_object, "Object argument to Signal constructor must be non-null.");

  object = p_object->get_instance_id();
  name = p_name;
}

Signal::Signal(ObjectID p_object, const StringName& p_name) {
  object = p_object;
  name = p_name;
}

Signal::operator String() const {
	Object *base = get_object();
	if (base) {
		String class_name = base->get_class();
		// Ref<Script> script = base->get_script();
		// if (script.is_valid() && script->get_path().is_resource_file()) {
		// 	class_name += "(" + script->get_path().get_file() + ")";
		// }
		return class_name + "::[signal]" + String(name);
	} else {
		return "null::[signal]" + String(name);
	}
}

