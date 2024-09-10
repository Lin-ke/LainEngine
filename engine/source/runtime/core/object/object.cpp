#include "object.h"
#include "core/object/objectdb.h"
namespace lain {
void Object::_construct_object(bool p_is_ref) {
  m_type_is_reference = p_is_ref;
  m_instance_id = ObjectDB::add_instance(this);
}
// reverse：先调父类再调子类
void Object::notification(int p_notification, bool p_reversed) {
  if (p_reversed) {
    /*if (script_instance) {
				script_instance->notification(p_notification, p_reversed);
			}*/
  } else {
    _notificationv(p_notification, p_reversed);
  }
  if (p_reversed) {
    _notificationv(p_notification, p_reversed);
  }
  /*else {
			if (script_instance) {
				script_instance->notification(p_notification, p_reversed);
			}
		}*/
}

PropertyInfo PropertyInfo::from_dict(const Dictionary& p_dict) {
  PropertyInfo pi;

  if (p_dict.has("type")) {
    pi.type = Variant::Type(int(p_dict["type"]));
  }

  if (p_dict.has("name")) {
    pi.name = p_dict["name"];
  }

  if (p_dict.has("class_name")) {
    pi.class_name = p_dict["class_name"];
  }

  if (p_dict.has("hint")) {
    pi.hint = PropertyHint(int(p_dict["hint"]));
  }

  if (p_dict.has("hint_string")) {
    pi.hint_string = p_dict["hint_string"];
  }

  if (p_dict.has("usage")) {
    pi.usage = p_dict["usage"];
  }

  return pi;
}
PropertyInfo::operator Dictionary() const {
	Dictionary d;
	d["name"] = name;
	d["class_name"] = class_name;
	d["type"] = type;
	d["hint"] = hint;
	d["hint_string"] = hint_string;
	d["usage"] = usage;
	return d;
}


}  // namespace lain