#pragma once
#ifndef CLASS_DB_H
#define CLASS_DB_H
// 1. resource type and solver 函数
// 2. Base and dertived relation
#include "core/meta/json.h"
#include "core/meta/method_bind.h"
#include "core/object/object.h"
#include "core/os/rwlock.h"
#include "core/os/thread_safe.h"
#include "core/string/print_string.h"
#include "core/templates/hash_set.h"
#include "core/templates/rb_map.h"
namespace lain {
// 以数据库列，表的方式组织
#ifdef DEBUG_METHODS_ENABLED

struct MethodDefinition {
  StringName name;
  Vector<StringName> args;
  MethodDefinition() {}
  MethodDefinition(const char* p_name) : name(p_name) {}
  MethodDefinition(const StringName& p_name) : name(p_name) {}
};

MethodDefinition D_METHODP(const char* p_name, const char* const** p_args, uint32_t p_argcount);

template <typename... VarArgs>
MethodDefinition D_METHOD(const char* p_name, const VarArgs... p_args) {
  const char* args[sizeof...(p_args) + 1] = {p_args..., nullptr};  // +1 makes sure zero sized arrays are also supported.
  const char* const* argptrs[sizeof...(p_args) + 1];
  for (uint32_t i = 0; i < sizeof...(p_args); i++) {
    argptrs[i] = &args[i];
  }

  return D_METHODP(p_name, sizeof...(p_args) == 0 ? nullptr : (const char* const**)argptrs, sizeof...(p_args));
}
#else
#define D_METHOD(m_c, ...) m_c
#endif

class ClassDB {
 public:
  // 萃取返回值类型
  // 包括成员函数，静态，普通
  template <typename>
  struct member_function_traits;

  template <typename R, typename T, typename... Args>
  struct member_function_traits<R (T::*)(Args...)> {
    using return_type = R;
  };

  template <typename R, typename T, typename... Args>
  struct member_function_traits<R (T::*)(Args...) const> {
    using return_type = R;
  };

  template <typename R, typename... Args>
  struct member_function_traits<R (*)(Args...)> {
    using return_type = R;
  };

  // M 是 method类型
  //create method bind 会匹配不同的类型：
  // 1. (void (T::*p_method)
  // 2. R (T::*p_method)(P...)
  // 3. (void (T::*p_method) const
  // 4. R (T::*p_method)(P...) const
  // 该方法会创建不同的MethodInfo实例：
  //  4-> MethodBindTRC
  template <typename N, typename M, typename... VarArgs>
  static MethodBind* bind_method(N p_method_name, M p_method, VarArgs... p_args) {
    Variant args[sizeof...(p_args) + 1] = {p_args..., Variant()};  // +1 makes sure zero sized arrays are also supported.
    const Variant* argptrs[sizeof...(p_args) + 1];
    for (uint32_t i = 0; i < sizeof...(p_args); i++) {
      argptrs[i] = &args[i];
    }
    MethodBind* bind = create_method_bind(p_method);
    if constexpr (std::is_same_v<typename member_function_traits<M>::return_type, Object*>) {
      bind->set_return_type_is_raw_object_ptr(true);
    }
    return bind_methodfi(METHOD_FLAGS_DEFAULT, bind, false, p_method_name, sizeof...(p_args) == 0 ? nullptr : (const Variant**)argptrs, sizeof...(p_args));
  }

  struct PropertySetGet {
    int index;
    StringName setter;
    StringName getter;
    MethodBind* _setptr = nullptr;
    MethodBind* _getptr = nullptr;
    Variant::Type type;
  };

  struct ClassInfo {  // class 包含这些内容：
                      // 类名，继承关系；类指针；方法map；常量map；属性列表；枚举map；信号map；方法map；属性setget
    ClassInfo* inherits_ptr = nullptr;
    void* class_ptr = nullptr;
    StringName name;
    StringName inherits;
    HashMap<StringName, MethodBind*> method_map;
    HashMap<StringName, int64_t> constant_map;
    List<PropertyInfo> property_list;

    Object* (*creation_func)() = nullptr;  // 一个memnew
    struct EnumInfo {
      List<StringName> constants;
      bool is_bitfield = false;
    };
    HashMap<StringName, EnumInfo> enum_map;
    HashMap<StringName, MethodInfo> signal_map;
    HashMap<StringName, PropertySetGet> property_setget;
		HashMap<StringName, PropertyInfo> property_map;

    bool disabled = false;
    bool exposed = false;
    bool reloadable = false;
    bool is_virtual = false;
    bool is_runtime = false;
    ClassInfo() {}
    ~ClassInfo() {}
  };

 private:
  static HashMap<StringName, StringName> resource_base_extensions;
  static HashMap<StringName, ClassInfo> classes;
  static HashMap<StringName, StringName> compat_classes;
  static RWLock lock;

 public:
  static void add_resource_base_extension(const StringName& p_extension, const StringName& p_class);
  static void get_resource_base_extensions(List<String>* p_extensions);
  static StringName get_parent_class(const StringName& p_class);
  static bool class_exists(const StringName& p_class);
  static bool class_can_serializable(const StringName& p_class);
  static Object* instantiate(const StringName& cp_class, bool p_require_real_class = false);
  static bool can_instantiate(const StringName& cp_class);
  static void* instantiate_with_name_json(const StringName& cp_class, const Json& json);
  static void* instantiate_with_json(const Json& json);
  // @todo
  static bool get_property(Object* p_object, const StringName& p_property, Variant& r_value, bool* r_valid = nullptr);
  static bool set_property(Object* p_object, const StringName& p_property, const Variant& p_value, bool* r_valid = nullptr);
  static void get_property_list(const StringName& p_class, List<PropertyInfo>* p_list, bool p_no_inheritance, const Object* p_validator);
  static void add_property(const StringName& p_class, const PropertyInfo& p_pinfo, const StringName& p_setter, const StringName& p_getter, int p_index = -1);
  static bool has_method(const StringName& p_class, const StringName& p_method, bool p_no_inheritance = false);
	static bool get_property_info(const StringName &p_class, const StringName &p_property, PropertyInfo *r_info, bool p_no_inheritance = false, const Object *p_validator = nullptr);
  static MethodBind* get_method(const StringName&, const StringName&);
#ifdef DEBUG_METHODS_ENABLED
  static MethodBind* bind_methodfi(uint32_t p_flags, MethodBind* p_bind, bool p_compatibility, const MethodDefinition& method_name, const Variant** p_defs, int p_defcount);
#else
  static MethodBind* bind_methodfi(uint32_t p_flags, MethodBind* p_bind, bool p_compatibility, const char* method_name, const Variant** p_defs, int p_defcount);
#endif

  template <typename T>
  static void register_class(bool p_virtual = false) {
    GLOBAL_LOCK_FUNCTION;
    static_assert(types_are_same_v<typename T::self_type, T>, "Class not declared properly, please use GDCLASS.");
    T::initialize_class();
    ClassInfo* t = classes.getptr(T::get_class_static());
    ERR_FAIL_NULL(t);
    t->creation_func = &creator<T>;
    t->exposed = true;
    t->is_virtual = p_virtual;
    t->class_ptr = T::get_class_ptr_static();
    // t->api = current_api;
    // T::register_custom_data_to_otdb();
  }
  template <typename T>
  static void _add_class() {
    _add_class2(T::get_class_static(), T::get_parent_class_static());
  }
  static void _add_class2(const StringName& p_class, const StringName& p_inherits);

  template <typename T>
  static Object* creator() {
    return memnew(T);
  }
  //template <typename N, typename M, typename... VarArgs>
  //static MethodBind* bind_method(N p_method_name, M p_method, VarArgs... p_args) {
  //	Variant args[sizeof...(p_args) + 1] = { p_args..., Variant() }; // +1 makes sure zero sized arrays are also supported.
  //	const Variant* argptrs[sizeof...(p_args) + 1];
  //	for (uint32_t i = 0; i < sizeof...(p_args); i++) {
  //		argptrs[i] = &args[i];
  //	}
  //	MethodBind* bind = create_method_bind(p_method);
  //	if constexpr (std::is_same_v<typename member_function_traits<M>::return_type, Object*>) {
  //		bind->set_return_type_is_raw_object_ptr(true);
  //	}
  //	return bind_methodfi(METHOD_FLAGS_DEFAULT, bind, false, p_method_name, sizeof...(p_args) == 0 ? nullptr : (const Variant**)argptrs, sizeof...(p_args));
  //}
};

}  // namespace lain

#define GDREGISTER_CLASS(m_class)             \
  if (m_class::_class_is_enabled) {           \
    lain::ClassDB::register_class<m_class>(); \
  }
#define GDREGISTER_VIRTUAL_CLASS(m_class)         \
  if (m_class::_class_is_enabled) {               \
    lain::ClassDB::register_class<m_class>(true); \
  }
#define GDREGISTER_ABSTRACT_CLASS(m_class)             \
  if (m_class::_class_is_enabled) {                    \
    lain::ClassDB::register_abstract_class<m_class>(); \
  }
#define GDREGISTER_INTERNAL_CLASS(m_class)             \
  if (m_class::_class_is_enabled) {                    \
    lain::ClassDB::register_internal_class<m_class>(); \
  }

#define GDREGISTER_RUNTIME_CLASS(m_class)             \
  if (m_class::_class_is_enabled) {                   \
    lain::ClassDB::register_runtime_class<m_class>(); \
  }
#endif