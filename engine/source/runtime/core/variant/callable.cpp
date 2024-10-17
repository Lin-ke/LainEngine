#include "callable.h"
#include "core/math/hashfuncs.h"
#include "core/object/object.h"
#include "core/object/objectdb.h"
#include "core/variant/variant.h"
namespace lain {
uint32_t Callable::hash() const {
  if (is_custom()) {
    return custom->hash();
  } else {
    uint32_t hash = method.hash();
    hash = hash_murmur3_one_64(object, hash);
    return hash_fmix32(hash);
  }
}
Callable::~Callable() {
  if (is_custom()) {
    if (custom->ref_count.unref()) {
      memdelete(custom);
    }
  }
}
Callable::Callable(const Callable& p_callable) {
  if (p_callable.is_custom()) {
    if (!p_callable.custom->ref_count.ref()) {
      object = 0;
    } else {
      object = 0;
      custom = p_callable.custom;
    }
  } else {
    method = p_callable.method;
    object = p_callable.object;
  }
}
StringName CallableCustom::get_method() const {
  ERR_FAIL_V_MSG(StringName(), vformat("Can't get method on CallableCustom \"%s\".", get_as_text()));
}
void CallableCustom::get_bound_arguments(Vector<Variant>& r_arguments, int& r_argcount) const {
  	r_arguments = Vector<Variant>();
	r_argcount = 0;
}

bool Callable::operator!=(const Callable &p_callable) const {
	return !(*this == p_callable);
}

Callable::Callable(ObjectID p_object, const StringName& p_method) {
  if (unlikely(p_method == StringName())) {
    object = 0;
    ERR_FAIL_MSG("Method argument to Callable constructor must be a non-empty string.");
  }

  object = p_object;
  method = p_method;
}

Callable::Callable(CallableCustom* p_custom) {
  if (unlikely(p_custom->referenced)) {
    object = 0;
    ERR_FAIL_MSG("Callable custom is already referenced.");
  }
  p_custom->referenced = true;
  object = 0;  //ensure object is all zero, since pointer may be 32 bits
  custom = p_custom;
}

Callable::Callable(const Object* p_object, const StringName& p_method) {
  if (unlikely(p_method == StringName())) {
    object = 0;
    ERR_FAIL_MSG("Method argument to Callable constructor must be a non-empty string.");
  }
  if (unlikely(p_object == nullptr)) {
    object = 0;
    ERR_FAIL_MSG("Object argument to Callable constructor must be non-null.");
  }

  object = p_object->get_instance_id();
  method = p_method;
}

void Callable::operator=(const Callable& p_callable) {
  if (is_custom()) {
    if (p_callable.is_custom()) {
      if (custom == p_callable.custom) {
        return;
      }
    }

    if (custom->ref_count.unref()) {
      memdelete(custom);
    }
  }

  if (p_callable.is_custom()) {
    method = StringName();
    if (!p_callable.custom->ref_count.ref()) {
      object = 0;
    } else {
      object = 0;
      custom = p_callable.custom;
    }
  } else {
    method = p_callable.method;
    object = p_callable.object;
  }
}
bool Callable::operator==(const Callable& p_callable) const {
  bool custom_a = is_custom();
  bool custom_b = p_callable.is_custom();

  if (custom_a == custom_b) {
    if (custom_a) {
      if (custom == p_callable.custom) {
        return true;  //same pointer, don't even compare
      }

      CallableCustom::CompareEqualFunc eq_a = custom->get_compare_equal_func();
      CallableCustom::CompareEqualFunc eq_b = p_callable.custom->get_compare_equal_func();
      if (eq_a == eq_b) {
        return eq_a(custom, p_callable.custom);
      } else {
        return false;
      }
    } else {
      return object == p_callable.object && method == p_callable.method;
    }
  } else {
    return false;
  }
}
bool Callable::is_valid() const {
  if (is_custom()) {
		return get_custom()->is_valid();
	} else {
    // @todo 需要class db记录 method信息 
		// return get_object() && get_object()->has_method(get_method());
    return true;
	}
}

CallableCustom *Callable::get_custom() const {
	ERR_FAIL_COND_V_MSG(!is_custom(), nullptr,
			vformat("Can't get custom on non-CallableCustom \"%s\".", operator String()));
	return custom;
}

Callable::operator String() const {
	if (is_custom()) {
		return custom->get_as_text();
	} else {
		if (is_null()) {
			return "null::null";
		}

		Object *base = get_object();
		// if (base) {
			String class_name = base->get_class();
		// 	Ref<Script> script = base->get_script();
		// 	if (script.is_valid() && script->get_path().is_resource_file()) {
		// 		class_name += "(" + script->get_path().get_file() + ")";
		// 	}
			return class_name + "::" + String(method);
		// } else {
		// 	return "null::" + String(method);
		// }
	}
}

Object *Callable::get_object() const {
	if (is_null()) {
		return nullptr;
	} else if (is_custom()) {
		return ObjectDB::get_instance(custom->get_object());
	} else {
		return ObjectDB::get_instance(ObjectID(object));
	}
}

StringName Callable::get_method() const {
	if (is_custom()) {
		return get_custom()->get_method();
	}
	return method;
}


void Callable::callp(const Variant** p_arguments, int p_argcount, Variant& r_return_value, CallError& r_call_error) const {
  if (is_null()) {
    r_call_error.error = CallError::CALL_ERROR_INSTANCE_IS_NULL;
    r_call_error.argument = 0;
    r_call_error.expected = 0;
    r_return_value = Variant();
  } else if (is_custom()) {
    if (!is_valid()) {
      r_call_error.error = CallError::CALL_ERROR_INSTANCE_IS_NULL;
      r_call_error.argument = 0;
      r_call_error.expected = 0;
      r_return_value = Variant();
      return;
    }
    custom->call(p_arguments, p_argcount, r_return_value, r_call_error);
  } else {
    Object* obj = ObjectDB::get_instance(ObjectID(object));
#ifdef DEBUG_ENABLED
    if (!obj) {
      r_call_error.error = CallError::CALL_ERROR_INSTANCE_IS_NULL;
      r_call_error.argument = 0;
      r_call_error.expected = 0;
      r_return_value = Variant();
      return;
    }
#endif
    r_return_value = obj->callp(method, p_arguments, p_argcount, r_call_error);
  }
}

void Callable::get_bound_arguments_ref(Vector<Variant> &r_arguments, int &r_argcount) const {
	if (!is_null() && is_custom()) {
		custom->get_bound_arguments(r_arguments, r_argcount);
	} else {
		r_arguments.clear();
		r_argcount = 0;
	}
}

}  // namespace lain
