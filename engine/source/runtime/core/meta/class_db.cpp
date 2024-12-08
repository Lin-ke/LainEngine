#include "class_db.h"
#include "core/object/object.h"
#include "core/variant/variant_helper.h"
#include "reflection/reflection.h"
#define OBJTYPE_RLOCK RWLockRead _rw_lockr_(lock);
#define OBJTYPE_WLOCK RWLockWrite _rw_lockw_(lock);
namespace lain {
RWLock ClassDB::lock;
#ifdef DEBUG_METHODS_ENABLED

MethodDefinition D_METHODP(const char* p_name, const char* const** p_args, uint32_t p_argcount) {
  MethodDefinition md;
  md.name = StaticCString::create(p_name);
  md.args.resize(p_argcount);
  for (uint32_t i = 0; i < p_argcount; i++) {
    md.args.write[i] = StaticCString::create(*p_args[i]);
  }
  return md;
}
#endif

HashMap<StringName, StringName> ClassDB::resource_base_extensions;
HashMap<StringName, ClassDB::ClassInfo> ClassDB::classes;
HashMap<StringName, StringName> ClassDB::compat_classes;

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
bool ClassDB::class_exists(const StringName& p_class) {
  if (Reflection::TypeMeta::is_valid_type(p_class)) {
    return true;
  }
  return false;
}

template <typename T, typename = void>
struct has_serial_spectialization : std::false_type {};

template <typename T>
struct has_serial_spectialization<T, std::void_t<decltype(Serializer::write<T>())>> : std::true_type {};

bool ClassDB::class_can_serializable(const StringName& p_class) {
  if (ClassDB::class_exists(p_class)) {
    return true;
  }
  //	属于可以序列化的类型
  // 但是需要从 StringName -> typename T ，这一步需要typeinfo 这个code gen
  return true;
}
// class数据库
Object* ClassDB::instantiate(const StringName& p_class, bool p_require_real_class) {
  ClassInfo* ti;
  ti = classes.getptr(p_class);
  if (!ti || ti->disabled || !ti->creation_func) {
    if (compat_classes.has(p_class)) {
      ti = classes.getptr(compat_classes[p_class]);
    }
  }
	if(!ti || ti->disabled || !ti->creation_func) {
		void* ptr = Reflection::TypeMeta::memnewByName(String(p_class).utf8().get_data());
		if(ptr) {
			return static_cast<Object*>(ptr);
		}
	}
	// failed
  ERR_FAIL_NULL_V_MSG(ti, nullptr, "Cannot get class '" + String(p_class) + "'.");
  ERR_FAIL_COND_V_MSG(ti->disabled, nullptr, "Class '" + String(p_class) + "' is disabled.");
  ERR_FAIL_NULL_V_MSG(ti->creation_func, nullptr, "Class '" + String(p_class) + "' or its base class cannot be instantiated.");
  return ti->creation_func();
}

bool ClassDB::can_instantiate(const StringName& p_class) {
  ClassInfo* ti = classes.getptr(p_class);
  if (!ti) {
    // return false;
    Reflection::TypeMeta meta = Reflection::TypeMeta::newMetaFromName(String(p_class));
    if (!meta.isValid()) {
      return false;
    }
  }
  return (!ti->disabled && ti->creation_func != nullptr);
}
// 这个json格式不一定正确
void* ClassDB::instantiate_with_name_json(const StringName& cp_class, const Json& p_json) {
  String err;
  Reflection::ReflectionInstance instance = Reflection::TypeMeta::newFromNameAndJson(SCSTR(cp_class), p_json);
  if (instance.m_instance == nullptr) {
    return nullptr;
  } else {
    return instance.m_instance;
  }
}
void* ClassDB::instantiate_with_json(const Json& p_json) {
  String err;
  Reflection::ReflectionInstance instance = Reflection::ReflectionInstance(p_json);
  if (instance.m_instance == nullptr) {
    return nullptr;
  } else {
    return instance.m_instance;
  }
}

void ClassDB::_add_class2(const StringName& p_class, const StringName& p_inherits) {
  //OBJTYPE_WLOCK;

  const StringName& name = p_class;

  ERR_FAIL_COND_MSG(classes.has(name), "Class '" + String(p_class) + "' already exists.");

  classes[name] = ClassInfo();
  ClassInfo& ti = classes[name];
  ti.name = name;
  ti.inherits = p_inherits;
  //ti.api = current_api;

  if (ti.inherits) {
    ERR_FAIL_COND(!classes.has(ti.inherits));  //it MUST be registered.
    ti.inherits_ptr = &classes[ti.inherits];

  } else {
    ti.inherits_ptr = nullptr;
  }
}
/*StringName ClassDB::get_parent_class(const StringName& p_class) {
		
	}*/
// 这里理应按着继承的顺序往上找，但是piccolo这一套太烂了，不弄他了
bool ClassDB::get_property(Object* p_object, const StringName& p_property, Variant& r_value, bool * r_valid) {
  ERR_FAIL_NULL_V(p_object, false);

	ClassInfo *type = classes.getptr(p_object->get_class_name());
	ClassInfo *check = type;
	while (check) {
		const PropertySetGet *psg = check->property_setget.getptr(p_property);
		if (psg) {
			if (!psg->getter) {
				return true; //return true but do nothing
			}

			if (psg->index >= 0) {
				Variant index = psg->index;
				const Variant *arg[1] = { &index };
				Callable::CallError ce;
				r_value = p_object->callp(psg->getter, arg, 1, ce);

			} else {
				Callable::CallError ce;
				if (psg->_getptr) {
					r_value = psg->_getptr->call(p_object, nullptr, 0, ce);
				} else {
					r_value = p_object->callp(psg->getter, nullptr, 0, ce);
				}
			}
			return true;
		}

		const int64_t *c = check->constant_map.getptr(p_property); //constants count
		if (c) {
			r_value = *c;
			return true;
		}

		if (check->method_map.has(p_property)) { //methods count
			r_value = Callable(p_object, p_property);
			return true;
		}

		if (check->signal_map.has(p_property)) { //signals count
			r_value = Signal(p_object, p_property);
			return true;
		}

		check = check->inherits_ptr;
	}

	// The "free()" method is special, so we assume it exists and return a Callable.
	if (p_property == "free") {
		r_value = Callable(p_object, p_property);
		return true;
	}

	return false;
}

bool ClassDB::get_property_info(const StringName &p_class, const StringName &p_property, PropertyInfo *r_info, bool p_no_inheritance, const Object *p_validator) {
	OBJTYPE_RLOCK;

	ClassInfo *check = classes.getptr(p_class);
	while (check) {
		if (check->property_map.has(p_property)) {
			PropertyInfo pinfo = check->property_map[p_property];
			if (p_validator) {
				// p_validator->validate_property(pinfo);
			}
			if (r_info) {
				*r_info = pinfo;
			}
			return true;
		}
		if (p_no_inheritance) {
			break;
		}
		check = check->inherits_ptr;
	}

	return false;
}

bool ClassDB::set_property(Object* p_object, const StringName& p_property, const Variant& p_value, bool *r_valid) {
 ERR_FAIL_NULL_V(p_object, false);

	ClassInfo *type = classes.getptr(p_object->get_class_name());
	ClassInfo *check = type;
	while (check) {
		const PropertySetGet *psg = check->property_setget.getptr(p_property);
		if (psg) {
			if (!psg->setter) {
				if (r_valid) {
					*r_valid = false;
				}
				return true; //return true but do nothing
			}

			Callable::CallError ce;

			if (psg->index >= 0) {
				Variant index = psg->index;
				const Variant *arg[2] = { &index, &p_value };
				//p_object->call(psg->setter,arg,2,ce);
				if (psg->_setptr) {
					psg->_setptr->call(p_object, arg, 2, ce);
				} else {
					p_object->callp(psg->setter, arg, 2, ce);
				}

			} else {
				const Variant *arg[1] = { &p_value };
				if (psg->_setptr) {
					psg->_setptr->call(p_object, arg, 1, ce);
				} else {
					p_object->callp(psg->setter, arg, 1, ce);
				}
			}

			if (r_valid) {
				*r_valid = ce.error == Callable::CallError::CALL_OK;
			}

			return true;
		}

		check = check->inherits_ptr;
	}
	return false;
}
void ClassDB::get_property_list(const StringName& p_class, List<PropertyInfo>* p_list, bool p_no_inheritance, const Object* p_validator) {
	ClassInfo *type = classes.getptr(p_class);
  ClassInfo *check = type;
	while (check) {
		for (const PropertyInfo &pi : check->property_list) {
			if (p_validator) {
				// Making a copy as we may modify it.
				PropertyInfo pi_mut = pi;
				// p_validator->validate_property(pi_mut);
				p_list->push_back(pi_mut);
			} else {
				p_list->push_back(pi);
			}
		}
		if (p_no_inheritance) {
			return;
		}
		check = check->inherits_ptr;
	}
}

void ClassDB::add_property(const StringName& p_class, const PropertyInfo& p_pinfo, const StringName& p_setter, const StringName& p_getter, int p_index) {
  	lock.read_lock();
	ClassInfo *type = classes.getptr(p_class);
	lock.read_unlock();

}

bool ClassDB::has_method(const StringName& p_class, const StringName& p_method, bool p_no_inheritance) {
	ClassInfo *type = classes.getptr(p_class);
	ClassInfo *check = type;
	while (check) {
		if (check->method_map.has(p_method)) {
			return true;
		}
		if (p_no_inheritance) {
			return false;
		}
		check = check->inherits_ptr;
	}

	return false;
}

#ifdef DEBUG_METHODS_ENABLED
MethodBind *ClassDB::bind_methodfi(uint32_t p_flags, MethodBind *p_bind, bool p_compatibility, const MethodDefinition &method_name, const Variant **p_defs, int p_defcount) {
	StringName mdname = method_name.name;
#else
MethodBind *ClassDB::bind_methodfi(uint32_t p_flags, MethodBind *p_bind, bool p_compatibility, const char *method_name, const Variant **p_defs, int p_defcount) {
	StringName mdname = StaticCString::create(method_name);
#endif
OBJTYPE_WLOCK;
	ERR_FAIL_NULL_V(p_bind, nullptr);
	p_bind->set_name(mdname);

	String instance_type = p_bind->get_instance_class();

#ifdef DEBUG_ENABLED

	ERR_FAIL_COND_V_MSG(!p_compatibility && has_method(instance_type, mdname), nullptr, "Class " + String(instance_type) + " already has a method " + String(mdname) + ".");
#endif

	ClassInfo *type = classes.getptr(instance_type);
	if (!type) {
		memdelete(p_bind);
		ERR_FAIL_V_MSG(nullptr, "Couldn't bind method '" + mdname + "' for instance '" + instance_type + "'.");
	}

	if (!p_compatibility && type->method_map.has(mdname)) {
		memdelete(p_bind);
		// overloading not supported
		ERR_FAIL_V_MSG(nullptr, "Method already bound '" + instance_type + "::" + mdname + "'.");
	}

#ifdef DEBUG_METHODS_ENABLED

	if (method_name.args.size() > p_bind->get_argument_count()) {
		memdelete(p_bind);
		ERR_FAIL_V_MSG(nullptr, "Method definition provides more arguments than the method actually has '" + instance_type + "::" + mdname + "'.");
	}

	p_bind->set_argument_names(method_name.args);

	if (!p_compatibility) {
		type->method_order.push_back(mdname);
	}
#endif

	if (p_compatibility) {
		// _bind_compatibility(type, p_bind);
	} else {
		type->method_map[mdname] = p_bind;
	}

	Vector<Variant> defvals;

	defvals.resize(p_defcount);
	for (int i = 0; i < p_defcount; i++) {
		defvals.write[i] = *p_defs[i];
	}

	p_bind->set_default_arguments(defvals);
	p_bind->set_hint_flags(p_flags);
	return p_bind;
}

}  // namespace lain
