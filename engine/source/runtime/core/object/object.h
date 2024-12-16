#pragma once
#ifndef __CORE_OBJECT_H__
#define __CORE_OBJECT_H__
#include "core/math/hashfuncs.h"
#include "core/meta/reflection/reflection_marcos.h"
#include "core/object/object_id.h"
#include "core/object/safe_refcount.h"
#include "core/os/spin_lock.h"
#include "core/string/string_name.h"
#include "core/templates/hash_map.h"
#include "core/templates/list.h"
#include "core/variant/callable.h"
#include "core/variant/variant.h"
#include "signal.h"

// m_class::_get_notification() != m_inherits::_get_notification()
// 判断是否重载了_notification函数指针
//
// 反射总归是通过字典实现的，但是A.传包装过的variant，B.直接传指针，我觉得还是B更可行，比如一些自定义类，在当前实现中需要通过object* 然后 variant来传
// Macros

// 现在已经允许了一种两边都是字典的写法，但是效率受限，因此也使用一些typemeta

namespace lain {

#define LCLASS(m_class, m_inherits)                                                                                                     \
 private:                                                                                                                               \
  void operator=(const m_class& p_rval) {}                                                                                              \
                                                                                                                                        \
 public:                                                                                                                                \
  typedef m_class self_type;                                                                                                            \
  static constexpr bool _class_is_enabled = m_inherits::_class_is_enabled;                                                              \
  virtual String get_class() const override {                                                                                           \
    return String(#m_class);                                                                                                            \
  }                                                                                                                                     \
                                                                                                                                        \
  virtual char* get_c_class() const override {                                                                                          \
    return #m_class;                                                                                                                    \
  }                                                                                                                                     \
  virtual StringName* _get_class_namev() const override {                                                                               \
    static StringName _class_name_static;                                                                                               \
    if (unlikely(!_class_name_static)) {                                                                                                \
      StringName::assign_static_unique_class_name(&_class_name_static, #m_class);                                                       \
    }                                                                                                                                   \
    return &_class_name_static;                                                                                                         \
  }                                                                                                                                     \
                                                                                                                                        \
  static _FORCE_INLINE_ void* get_class_ptr_static() {                                                                                  \
    static int ptr;                                                                                                                     \
    return &ptr;                                                                                                                        \
  }                                                                                                                                     \
  static _FORCE_INLINE_ String get_class_static() {                                                                                     \
    return String(#m_class);                                                                                                            \
  }                                                                                                                                     \
                                                                                                                                        \
  static _FORCE_INLINE_ String get_parent_class_static() {                                                                              \
    return String(m_inherits::get_class_static());                                                                                      \
  }                                                                                                                                     \
                                                                                                                                        \
  virtual bool is_class_ptr(void* p_ptr) const override {                                                                               \
    return (p_ptr == get_class_ptr_static()) ? true : m_inherits::is_class_ptr(p_ptr);                                                  \
  }                                                                                                                                     \
  _FORCE_INLINE_ void (Object::*_get_notification() const)(int) {                                                                       \
    return (void(Object::*)(int)) & m_class::_notification;                                                                             \
  }                                                                                                                                     \
  _FORCE_INLINE_ void (Object::*_get_from_data() const)(int) {                                                                          \
    return (void(Object::*)(int)) & m_class::_from_data;                                                                                \
  }                                                                                                                                     \
  virtual void _notificationv(int p_notification, bool p_reversed) override {                                                           \
    if (!p_reversed) {                                                                                                                  \
      m_inherits::_notificationv(p_notification, p_reversed);                                                                           \
    }                                                                                                                                   \
    if (m_class::_get_notification() != m_inherits::_get_notification()) {                                                              \
      _notification(p_notification);                                                                                                    \
    }                                                                                                                                   \
    if (p_reversed) {                                                                                                                   \
      m_inherits::_notificationv(p_notification, p_reversed);                                                                           \
    }                                                                                                                                   \
  }                                                                                                                                     \
  virtual void _from_datav(void* p_data, bool p_reversed) override {                                                                    \
    if (!p_reversed) {                                                                                                                  \
      m_inherits::_from_datav(p_data, p_reversed);                                                                                      \
    }                                                                                                                                   \
    if (m_class::_get_from_data() != m_inherits::_get_from_data()) {                                                                    \
      _from_data(p_data);                                                                                                               \
    }                                                                                                                                   \
    if (p_reversed) {                                                                                                                   \
      m_inherits::_from_datav(p_data, p_reversed);                                                                                      \
    }                                                                                                                                   \
  }                                                                                                                                     \
                                                                                                                                        \
  _FORCE_INLINE_ void (Object::*_get_get_property_list() const)(List<PropertyInfo> * p_list) const {                                    \
    return (void(Object::*)(List<PropertyInfo>*) const) & m_class::_get_property_list;                                                  \
  }                                                                                                                                     \
  virtual void _get_property_listv(List<PropertyInfo>* p_list, bool p_reversed) const override {                                        \
    if (!p_reversed) {                                                                                                                  \
      m_inherits::_get_property_listv(p_list, p_reversed);                                                                              \
    }                                                                                                                                   \
    p_list->push_back(PropertyInfo(Variant::NIL, get_class_static(), PROPERTY_HINT_NONE, get_class_static(), PROPERTY_USAGE_CATEGORY)); \
    if (!_is_gpl_reversed()) {                                                                                                          \
      lain::ClassDB::get_property_list(#m_class, p_list, true, this);                                                                       \
    }                                                                                                                                   \
    if (m_class::_get_get_property_list() != m_inherits::_get_get_property_list()) {                                                    \
      _get_property_list(p_list);                                                                                                       \
    }                                                                                                                                   \
    if (_is_gpl_reversed()) {                                                                                                           \
      lain::ClassDB::get_property_list(#m_class, p_list, true, this);                                                                       \
    }                                                                                                                                   \
    if (p_reversed) {                                                                                                                   \
      m_inherits::_get_property_listv(p_list, p_reversed);                                                                              \
    }                                                                                                                                   \
  }                                                                                                                                     \
                                                                                                                                        \
 protected:                                                                                                                             \
  _FORCE_INLINE_ static void (*_get_bind_methods())() {                                                                                 \
    return &m_class::_bind_methods;                                                                                                     \
  }                                                                                                                                     \
  _FORCE_INLINE_ static void (*_get_bind_compatibility_methods())() {                                                                   \
    return &m_class::_bind_compatibility_methods;                                                                                       \
  }                                                                                                                                     \
                                                                                                                                        \
 public:                                                                                                                                \
  static void initialize_class() {                                                                                                      \
    static bool initialized = false;                                                                                                    \
    if (initialized) {                                                                                                                  \
      return;                                                                                                                           \
    }                                                                                                                                   \
    m_inherits::initialize_class();                                                                                                     \
    ClassDB::_add_class<m_class>();                                                                                                     \
    if (m_class::_get_bind_methods() != m_inherits::_get_bind_methods()) {                                                              \
      _bind_methods();                                                                                                                  \
    }                                                                                                                                   \
    if (m_class::_get_bind_compatibility_methods() != m_inherits::_get_bind_compatibility_methods()) {                                  \
      _bind_compatibility_methods();                                                                                                    \
    }                                                                                                                                   \
    initialized = true;                                                                                                                 \
  }

#define OBJ_SAVE_TYPE(m_class)                     \
 public:                                           \
  virtual String get_save_class() const override { \
    return #m_class;                               \
  }                                                \
                                                   \
 private:

enum PropertyHint {
  PROPERTY_HINT_NONE,             ///< no hint provided.
  PROPERTY_HINT_RANGE,            ///< hint_text = "min,max[,step][,or_greater][,or_less][,hide_slider][,radians_as_degrees][,degrees][,exp][,suffix:<keyword>] range.
  PROPERTY_HINT_ENUM,             ///< hint_text= "val1,val2,val3,etc"
  PROPERTY_HINT_ENUM_SUGGESTION,  ///< hint_text= "val1,val2,val3,etc"
  PROPERTY_HINT_EXP_EASING,  /// exponential easing function (Math::ease) use "attenuation" hint string to revert (flip h), "positive_only" to exclude in-out and out-in. (ie: "attenuation,positive_only")
  PROPERTY_HINT_LINK,
  PROPERTY_HINT_FLAGS,  ///< hint_text= "flag1,flag2,etc" (as bit flags)
  PROPERTY_HINT_LAYERS_2D_RENDER,
  PROPERTY_HINT_LAYERS_2D_PHYSICS,
  PROPERTY_HINT_LAYERS_2D_NAVIGATION,
  PROPERTY_HINT_LAYERS_3D_RENDER,
  PROPERTY_HINT_LAYERS_3D_PHYSICS,
  PROPERTY_HINT_LAYERS_3D_NAVIGATION,
  PROPERTY_HINT_FILE,              ///< a file path must be passed, hint_text (optionally) is a filter "*.png,*.wav,*.doc,"
  PROPERTY_HINT_DIR,               ///< a directory path must be passed
  PROPERTY_HINT_GLOBAL_FILE,       ///< a file path must be passed, hint_text (optionally) is a filter "*.png,*.wav,*.doc,"
  PROPERTY_HINT_GLOBAL_DIR,        ///< a directory path must be passed
  PROPERTY_HINT_RESOURCE_TYPE,     ///< a resource object type
  PROPERTY_HINT_MULTILINE_TEXT,    ///< used for string properties that can contain multiple lines
  PROPERTY_HINT_EXPRESSION,        ///< used for string properties that can contain multiple lines
  PROPERTY_HINT_PLACEHOLDER_TEXT,  ///< used to set a placeholder text for string properties
  PROPERTY_HINT_COLOR_NO_ALPHA,    ///< used for ignoring alpha component when editing a color
  PROPERTY_HINT_OBJECT_ID,
  PROPERTY_HINT_TYPE_STRING,               ///< a type string, the hint is the base type to choose
  PROPERTY_HINT_NODE_PATH_TO_EDITED_NODE,  // Deprecated.
  PROPERTY_HINT_OBJECT_TOO_BIG,            ///< object is too big to send
  PROPERTY_HINT_NODE_PATH_VALID_TYPES,
  PROPERTY_HINT_SAVE_FILE,         ///< a file path must be passed, hint_text (optionally) is a filter "*.png,*.wav,*.doc,". This opens a save dialog
  PROPERTY_HINT_GLOBAL_SAVE_FILE,  ///< a file path must be passed, hint_text (optionally) is a filter "*.png,*.wav,*.doc,". This opens a save dialog
  PROPERTY_HINT_INT_IS_OBJECTID,   // Deprecated.
  PROPERTY_HINT_INT_IS_POINTER,
  PROPERTY_HINT_ARRAY_TYPE,
  PROPERTY_HINT_LOCALE_ID,
  PROPERTY_HINT_LOCALIZABLE_STRING,
  PROPERTY_HINT_NODE_TYPE,             ///< a node object type
  PROPERTY_HINT_HIDE_QUATERNION_EDIT,  /// Only Node3D::transform should hide the quaternion editor.
  PROPERTY_HINT_PASSWORD,
  PROPERTY_HINT_LAYERS_AVOIDANCE,
  PROPERTY_HINT_MAX,
};

enum PropertyUsageFlags {
  PROPERTY_USAGE_NONE = 0,
  PROPERTY_USAGE_STORAGE = 1 << 1,
  PROPERTY_USAGE_EDITOR = 1 << 2,
  PROPERTY_USAGE_INTERNAL = 1 << 3,
  PROPERTY_USAGE_CHECKABLE = 1 << 4,  // Used for editing global variables.
  PROPERTY_USAGE_CHECKED = 1 << 5,    // Used for editing global variables.
  PROPERTY_USAGE_GROUP = 1 << 6,      // Used for grouping props in the editor.
  PROPERTY_USAGE_CATEGORY = 1 << 7,
  PROPERTY_USAGE_SUBGROUP = 1 << 8,
  PROPERTY_USAGE_CLASS_IS_BITFIELD = 1 << 9,
  PROPERTY_USAGE_NO_INSTANCE_STATE = 1 << 10,
  PROPERTY_USAGE_RESTART_IF_CHANGED = 1 << 11,
  PROPERTY_USAGE_SCRIPT_VARIABLE = 1 << 12,
  PROPERTY_USAGE_STORE_IF_NULL = 1 << 13,
  PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED = 1 << 14,
  PROPERTY_USAGE_SCRIPT_DEFAULT_VALUE = 1 << 15,  // Deprecated.
  PROPERTY_USAGE_CLASS_IS_ENUM = 1 << 16,
  PROPERTY_USAGE_NIL_IS_VARIANT = 1 << 17,
  PROPERTY_USAGE_ARRAY = 1 << 18,             // Used in the inspector to group properties as elements of an array.
  PROPERTY_USAGE_ALWAYS_DUPLICATE = 1 << 19,  // When duplicating a resource, always duplicate, even with subresource duplication disabled.
  PROPERTY_USAGE_NEVER_DUPLICATE = 1 << 20,   // When duplicating a resource, never duplicate, even with subresource duplication enabled.
  PROPERTY_USAGE_HIGH_END_GFX = 1 << 21,
  PROPERTY_USAGE_NODE_PATH_FROM_SCENE_ROOT = 1 << 22,
  PROPERTY_USAGE_RESOURCE_NOT_PERSISTENT = 1 << 23,
  PROPERTY_USAGE_KEYING_INCREMENTS = 1 << 24,          // Used in inspector to increment property when keyed in animation player.
  PROPERTY_USAGE_DEFERRED_SET_RESOURCE = 1 << 25,      // Deprecated.
  PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT = 1 << 26,  // For Object properties, instantiate them when creating in editor.
  PROPERTY_USAGE_EDITOR_BASIC_SETTING = 1 << 27,       //for project or editor settings, show when basic settings are selected.
  PROPERTY_USAGE_READ_ONLY = 1 << 28,                  // Mark a property as read-only in the inspector.
  PROPERTY_USAGE_SECRET = 1 << 29,                     // Export preset credentials that should be stored separately from the rest of the export config.

  PROPERTY_USAGE_DEFAULT = PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_EDITOR,
  PROPERTY_USAGE_NO_EDITOR = PROPERTY_USAGE_STORAGE,
};

#define MAKE_RESOURCE_TYPE_HINT(m_type) vformat("%s/%s:%s", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, m_type)

// base class of all object
class Viewport;
// signal mechanism
REFLECTION_TYPE(Connection)
class Connection {
  REFLECTION_BODY(Connection);

 public:
  META(WhiteListFields)
  Signal signal;
  Callable callable;  // 什么类型的什么方法

  uint32_t flags = 0;
  bool operator<(const Connection& p_conn) const;

  operator Variant() const;

  Connection() {}
  Connection(const Variant& p_variant);
};

/// object
struct PropertyInfo {
  Variant::Type type = Variant::NIL;
  String name;
  StringName class_name;  // For classes
  PropertyHint hint = PROPERTY_HINT_NONE;
  String hint_string;
  uint32_t usage = PROPERTY_USAGE_DEFAULT;

  // If you are thinking about adding another member to this class, ask the maintainer (Juan) first.

  _FORCE_INLINE_ PropertyInfo added_usage(uint32_t p_fl) const {
    PropertyInfo pi = *this;
    pi.usage |= p_fl;
    return pi;
  }

  operator Dictionary() const;

  static PropertyInfo from_dict(const Dictionary& p_dict);

  PropertyInfo() {}

  PropertyInfo(const Variant::Type p_type, const String& p_name, const PropertyHint p_hint = PROPERTY_HINT_NONE, const String& p_hint_string = "",
               const uint32_t p_usage = PROPERTY_USAGE_DEFAULT, const StringName& p_class_name = StringName())
      : type(p_type), name(p_name), hint(p_hint), hint_string(p_hint_string), usage(p_usage) {
    if (hint == PROPERTY_HINT_RESOURCE_TYPE) {
      class_name = hint_string;
    } else {
      class_name = p_class_name;
    }
  }

  PropertyInfo(const StringName& p_class_name) : type(Variant::OBJECT), class_name(p_class_name) {}

  /*explicit PropertyInfo(const GDExtensionPropertyInfo& pinfo) :
			type((Variant::Type)pinfo.type),
			name(*reinterpret_cast<StringName*>(pinfo.name)),
			class_name(*reinterpret_cast<StringName*>(pinfo.class_name)),
			hint((PropertyHint)pinfo.hint),
			hint_string(*reinterpret_cast<String*>(pinfo.hint_string)),
			usage(pinfo.usage) {}*/

  bool operator==(const PropertyInfo& p_info) const {
    return ((type == p_info.type) && (name == p_info.name) && (class_name == p_info.class_name) && (hint == p_info.hint) && (hint_string == p_info.hint_string) &&
            (usage == p_info.usage));
  }

  bool operator<(const PropertyInfo& p_info) const { return name < p_info.name; }
};

REFLECTION_TYPE(Object)
class Object {
  REFLECTION_BODY(Object);

 public:
  Object() { _construct_object(false); }
  Object(bool p_reference) { _construct_object(p_reference); }
  // META
  META(WhiteListFields)
  HashMap<StringName, Variant> metadata;
  HashMap<StringName, Variant*> metadata_properties;
  mutable const StringName* _class_name_ptr = nullptr;

  L_INLINE bool is_ref_counted() const { return m_type_is_reference; }
  L_INLINE ObjectID get_instance_id() const { return m_instance_id; }
  /// static method
  template <class T>
  static T* cast_to(Object* p_object) {
    return dynamic_cast<T*>(p_object);
  }
  template <class T>
  static const T* cast_to(const Object* p_object) {
    return dynamic_cast<const T*>(p_object);
  }
  virtual String get_class() const { return String("Object"); }
  virtual char* get_c_class() const { return "Object"; }
  static String get_class_static() { return String("Object"); }
  virtual String get_save_class() const { return get_class(); }  //class stored when saving
  virtual bool is_class_ptr(void* p_ptr) const { return get_class_ptr_static() == p_ptr; }

  virtual const StringName* _get_class_namev() const {
    static StringName _class_name_static;
    if (unlikely(!_class_name_static)) {
      StringName::assign_static_unique_class_name(&_class_name_static, "Object");
    }
    return &_class_name_static;
  }
  static String get_parent_class_static() { return String(); }

  _FORCE_INLINE_ const StringName& get_class_name() const {
    //if (_extension) {
    //	// Can't put inside the unlikely as constructor can run it
    //	return _extension->class_name;
    //}

    if (unlikely(!_class_name_ptr)) {
      // While class is initializing / deinitializing, constructors and destructurs
      // need access to the proper class at the proper stage.
      return *_get_class_namev();
    }
    return *_class_name_ptr;
  }
  // 其目的是允许对象响应可能与其相关的各种引擎级回调
  void notification(int p_notification, bool p_reversed = false);
  virtual void _notificationv(int p_notification, bool p_reversed) {}
	virtual void _get_property_listv(List<PropertyInfo> *p_list, bool p_reversed) const {};
  void _get_property_list(List<PropertyInfo> *p_list) const {};
  void _notification(int p_notification) {}  // 用这个写，不virtual的重载
  _FORCE_INLINE_ void (Object::*_get_notification() const)(int) { return &Object::_notification; }
  static void _bind_methods() {}
  static void _bind_compatibility_methods() {}
  static void initialize_class();

  _FORCE_INLINE_ static void (*_get_bind_methods())() { return &Object::_bind_methods; }
  _FORCE_INLINE_ static void (*_get_bind_compatibility_methods())() { return &Object::_bind_compatibility_methods; }

  Variant callp(const StringName& p_method, const Variant** p_args, int p_argcount, Callable::CallError& r_error);
  L_INLINE virtual String to_string() const { return "<" + get_class() + "#" + itos(get_instance_id()) + ">"; }

  Variant get(const StringName& p_name, bool* r_valid= nullptr) const;
  void set(const StringName& p_name, const Variant& p_value, bool* r_valid = nullptr);
	virtual bool _setv(const StringName &p_name, const Variant &p_property) { return false; };
	virtual bool _getv(const StringName &p_name, Variant &r_property) const { return false; };
  void get_property_list(List<PropertyInfo> *p_list, bool p_reversed = false) const;

  _FORCE_INLINE_ void (Object::*_get_get_property_list() const)(List<PropertyInfo> *p_list) const {
		return &Object::_get_property_list;
	} 
  private : ObjectID m_instance_id;
  bool m_type_is_reference = false;

  void _construct_object(bool p_reference);
  struct SignalData {
    struct Slot {
      int reference_count = 0;
      Connection conn;
      List<Connection>::Element* cE = nullptr;
    };
    //MethodInfo user;
    HashMap<Callable, Slot, HashableHasher<Callable>> slot_map;
  };
  HashMap<StringName, SignalData> signal_map;

 public:
  // 在godot里，通过宏 和 propertyinfo 注册有哪些属性和函数，并绑定修改这个属性的方法(set, get)
  // 之后在 classdb里调用这个方法修改
  // 全部通过variant进行

  // piccolo这种写法根本都没法回调，set那个数值就完了

  // 目前先按下面的写法，
  // 返回一个数据类，后者比较容易反射，并提供一个from_data进行数据的设置
  // 这是因为设置时具有一些副作用，因此使用数据类再在from_data中进行调用可能更稳定
  virtual void* get_instance_data() const { return nullptr; }  // 仅在一些特殊情况下使用，一般直接序列化就可以了
  virtual StringName get_data_classname() const { return get_class() + "Data"; }
  virtual void _from_datav(void* p_data, bool p_reversed = false) {}
  void _from_data(void* p_data) {}  // 子类写这个
  void from_data(void* p_data, bool p_reversed = false);
  _FORCE_INLINE_ void (Object::*_get_from_data() const)(int) { return (void(Object::*)(int)) & Object::_from_data; }
  void notify_property_list_changed() {}
  static constexpr bool _class_is_enabled = true;
  typedef Object self_type;
  static void* get_class_ptr_static() {  // 这是何意
    static int ptr;
    return &ptr;
  }

	bool _is_gpl_reversed() const { return false; }
	virtual String to_string(){
    return "<" + get_class() + "#" + itos(get_instance_id()) + ">";
  }


};

enum MethodFlags {
  METHOD_FLAG_NORMAL = 1,
  METHOD_FLAG_EDITOR = 2,
  METHOD_FLAG_CONST = 4,
  METHOD_FLAG_VIRTUAL = 8,
  METHOD_FLAG_VARARG = 16,
  METHOD_FLAG_STATIC = 32,
  METHOD_FLAG_OBJECT_CORE = 64,
  METHOD_FLAGS_DEFAULT = METHOD_FLAG_NORMAL,
};

struct MethodInfo {
  String name;
  PropertyInfo return_val;
  uint32_t flags = METHOD_FLAGS_DEFAULT;
  int id = 0;
  List<PropertyInfo> arguments;
  Vector<Variant> default_arguments;

	MethodInfo(const String &p_name) { name = p_name; }
	MethodInfo(Variant::Type ret) { return_val.type = ret; }
  // 无参数
	MethodInfo(const PropertyInfo &p_ret, const String &p_name) {
		return_val = p_ret;
		name = p_name;
	}
	void _push_params(const PropertyInfo &p_param) {
		arguments.push_back(p_param);
	}

	MethodInfo() {}
	template <typename... VarArgs>
	void _push_params(const PropertyInfo &p_param, VarArgs... p_params) {
		arguments.push_back(p_param);
		_push_params(p_params...);
	}
  // 有返回值，有参数
  	template <typename... VarArgs>
	MethodInfo(Variant::Type ret, const String &p_name, VarArgs... p_params) {
		name = p_name;
		return_val.type = ret;
		_push_params(p_params...);
	}
	template <typename... VarArgs>
	MethodInfo(const PropertyInfo &p_ret, const String &p_name, VarArgs... p_params) {
		return_val = p_ret;
		name = p_name;
		_push_params(p_params...);
	}
  // 无返回值
  	template <typename... VarArgs>
	MethodInfo(const String &p_name, VarArgs... p_params) {
		name = p_name;
		_push_params(p_params...);
	}

};

}  // namespace lain
#define ADD_PROPERTY(m_property, m_setter, m_getter) lain::ClassDB::add_property(get_class_static(), m_property, _scs_create(m_setter), _scs_create(m_getter))
#define ADD_PROPERTYI(m_property, m_setter, m_getter, m_index) lain::ClassDB::add_property(get_class_static(), m_property, _scs_create(m_setter), _scs_create(m_getter), m_index)
#define ADD_SIGNAL(m_signal) lain::ClassDB::add_signal(get_class_static(), m_signal)

// const 限定符是必要的，因为const对象拒绝调用非const方法

#endif  // !__CORE_OBJECT_H__
