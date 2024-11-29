#include "class_db.h"
#include "core/object/object.h"
#include "core/variant/variant_helper.h"
#include "reflection/reflection.h"
namespace lain {

MethodDefinition D_METHODP(const char* p_name, const char* const** p_args, uint32_t p_argcount) {
  MethodDefinition md;
  md.name = StaticCString::create(p_name);
  md.args.resize(p_argcount);
  for (uint32_t i = 0; i < p_argcount; i++) {
    md.args.write[i] = StaticCString::create(*p_args[i]);
  }
  return md;
}

HashMap<StringName, StringName> ClassDB::resource_base_extensions;
HashMap<StringName, ClassInfo> ClassDB::classes;
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
bool ClassDB::get_property(Object* p_object, const StringName& p_property, Variant& r_value) {
  void* property_ptr = nullptr;
  Reflection::TypeMeta meta = Reflection::TypeMeta::newMetaFromName(p_object->get_class_name());
  Reflection::FieldAccessor* fields;
  int fields_count = meta.getFieldsList(fields);
  bool found = false;
  for (int i = 0; i < fields_count; i++) {
    if (fields[i].getFieldName() == p_property) {
      property_ptr = fields[i].get(p_object);
      found = true;
      break;
    }
  }
  if (!found) {
    return false;
  }
  // 变成variant
  Variant::Type type = VariantHelper::get_type_from_name(fields->getFieldTypeName());
  VariantHelper::variant_from_data(type, property_ptr, r_value);
}

bool ClassDB::set_property(Object* p_object, const StringName& p_property, const Variant& r_value) {
  void* property_ptr = nullptr;
  Reflection::TypeMeta meta = Reflection::TypeMeta::newMetaFromName(p_object->get_class_name());
  Reflection::FieldAccessor* fields;  // @todo 把这个做成缓存，就是做成表
  int fields_count = meta.getFieldsList(fields);
  bool found = false;
  for (int i = 0; i < fields_count; i++) {
    if (fields[i].getFieldName() == p_property) {
      property_ptr = fields[i].get(p_object);
      found = true;
      break;
    }
  }
  if (!found) {
    return false;
  }
  Variant::Type type = VariantHelper::get_type_from_name(fields->getFieldTypeName());
  if (type != r_value.get_type()) {
    L_CORE_WARN("type mismatch");
  }
  VariantHelper::object_set_data(property_ptr, r_value);
}
}  // namespace lain
