#include "object.h"
#include "core/meta/class_db.h"
#include "core/object/objectdb.h"
namespace lain {
void Object::_construct_object(bool p_is_ref) {
  m_type_is_reference = p_is_ref;
  m_instance_id = ObjectDB::add_instance(this);
}
// 考虑script里的顺序
void Object::from_data(void* p_data, bool p_reversed) {
  if (p_reversed) {

  } else {
    _from_datav(p_data, p_reversed);
  }
  if (p_reversed) {
    _from_datav(p_data, p_reversed);
  }
}
// reverse：先调父类再调子类
void Object::notification(int p_notification, bool p_reversed) {

  _notificationv(p_notification, p_reversed);
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

void Object::initialize_class() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  ClassDB::_add_class<Object>();
  _bind_methods();
  _bind_compatibility_methods();
  initialized = true;
}

Variant Object::callp(const StringName& p_method, const Variant** p_args, int p_argcount, Callable::CallError& r_error) {
  r_error.error = Callable::CallError::CALL_OK;
  // 处理free

  // 如果是脚本调用

  // 通过反射获得类的方法并调用
  Variant ret;

  MethodBind* method = ClassDB::get_method(get_class_name(), p_method);

  if (method) {
    ret = method->call(this, p_args, p_argcount, r_error);
  } else {
    r_error.error = Callable::CallError::CALL_ERROR_INVALID_METHOD;
  }

  return ret;
}
Variant Object::get(const StringName& p_name, bool* r_valid) const {
  Variant ret;
  // Try built-in getter.
  {
    if (ClassDB::get_property(const_cast<Object*>(this), p_name, ret)) {
      if (r_valid) {
        *r_valid = true;
      }
      return ret;
    }
  }

  // Something inside the object... :|
  bool success = _getv(p_name, ret);
  if (success) {
    if (r_valid) {
      *r_valid = true;
    }
    return ret;
  }

  if (r_valid) {
    *r_valid = false;
  }
  return Variant();
}

void Object::set(const StringName& p_name, const Variant& p_value, bool* r_valid) {
  // Try built-in setter.
  if (ClassDB::set_property(this, p_name, p_value)) {
    return;
  }
  bool success = _setv(p_name, p_value);
  if (success) {
    if (r_valid) {
      *r_valid = true;
    }
    return;
  }

  if (r_valid) {
    *r_valid = false;
  }
}

void Object::get_property_list(List<PropertyInfo>* p_list, bool p_reversed) const {
  _get_property_listv(p_list, p_reversed);
}

}  // namespace lain
