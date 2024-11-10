#pragma once
#ifndef __VARIANT_H__
#define __VARIANT_H__
#include "core/variant/variant_header.h"

namespace lain {
class Object;  // Object 里需要Variant （property），variant内部需要通过object指针
typedef Vector<uint8_t> PackedByteArray;
typedef Vector<int32_t> PackedInt32Array;
typedef Vector<int64_t> PackedInt64Array;
typedef Vector<float> PackedFloat32Array;
typedef Vector<double> PackedFloat64Array;
typedef Vector<String> PackedStringArray;
typedef Vector<Vector2> PackedVector2Array;
typedef Vector<Vector3> PackedVector3Array;
typedef Vector<Color> PackedColorArray;
typedef Vector<Vector4> PackedVector4Array;
class Object;
class ConfigFile;
class VariantInternal;
class RID;
class Serializer;
struct MethodInfo;

// variant的实现和lua是一样的，但是多一些基本类，如果只有数学类和table就是lua了
// 就像lua一样，主要类是依靠字典来实现的
// 注意有一些引用计数的情况可能会导致问题

class Variant {
  friend class VariantInternal;
  friend class Serializer;

 public:
  enum Type {
    NIL,

    // atomic types
    BOOL,
    INT,
    FLOAT,
    STRING,

    // math types
    VECTOR2,
    VECTOR2I,
    RECT2,
    RECT2I,
    VECTOR3,
    VECTOR3I,
    TRANSFORM2D,
    VECTOR4,
    VECTOR4I,
    PLANE,
    QUATERNION,
    AABB,
    BASIS,
    TRANSFORM3D,
    PROJECTION,

    // misc types
    COLOR,
    STRING_NAME,
    GOBJECT_PATH,
    RID,
    OBJECT,
    CALLABLE,
    SIGNAL,
    DICTIONARY,
    ARRAY,

    // typed arrays
    PACKED_BYTE_ARRAY,
    PACKED_INT32_ARRAY,
    PACKED_INT64_ARRAY,
    PACKED_FLOAT32_ARRAY,
    PACKED_FLOAT64_ARRAY,
    PACKED_STRING_ARRAY,
    PACKED_VECTOR2_ARRAY,
    PACKED_VECTOR3_ARRAY,
    PACKED_COLOR_ARRAY,
    PACKED_VECTOR4_ARRAY,

    // reflect
    REFLECTIONINSTANCE,  //Reflection instance真的有必要吗？
    VARIANT_MAX,

  };

  enum {
    // Maximum recursion depth allowed when serializing variants.
    MAX_RECURSION_DEPTH = 1024,
  };

 private:
  Type type = NIL;

 public:
  static bool can_convert(Type p_type_from, Type p_type_to);
  static bool can_convert_strict(Type from, Type to);

 private:
  /// <summary>
  /// array helpers
  /// </summary>
  // 注意用这种技术来进行类型擦除，子类型通过模板特化，只需保留一个对父类的引用
  // 内部是一个Vector<T> array;

  struct PackedArrayRefBase {
    SafeRefCount refcount;
    _FORCE_INLINE_ PackedArrayRefBase* reference() {
      if (this->refcount.ref()) {
        return this;
      } else {
        return nullptr;
      }
    }
    static _FORCE_INLINE_ PackedArrayRefBase* reference_from(PackedArrayRefBase* p_base, PackedArrayRefBase* p_from) {
      if (p_base == p_from) {
        return p_base;  //same thing, do nothing
      }

      if (p_from->reference()) {
        if (p_base->refcount.unref()) {
          memdelete(p_base);
        }
        return p_from;
      } else {
        return p_base;  //keep, could not reference new
      }
    }
    static _FORCE_INLINE_ void destroy(PackedArrayRefBase* p_array) {
      if (p_array->refcount.unref()) {
        memdelete(p_array);
      }
    }
    _FORCE_INLINE_ virtual ~PackedArrayRefBase() {}  //needs virtual destructor, but make inline
  };
  template <class T>
  struct PackedArrayRef : public PackedArrayRefBase {
    Vector<T> array;
    static _FORCE_INLINE_ PackedArrayRef<T>* create() { return memnew(PackedArrayRef<T>); }
    static _FORCE_INLINE_ PackedArrayRef<T>* create(const Vector<T>& p_from) { return memnew(PackedArrayRef<T>(p_from)); }

    static _FORCE_INLINE_ const Vector<T>& get_array(PackedArrayRefBase* p_base) { return static_cast<PackedArrayRef<T>*>(p_base)->array; }
    static _FORCE_INLINE_ Vector<T>* get_array_ptr(const PackedArrayRefBase* p_base) {
      return &const_cast<PackedArrayRef<T>*>(static_cast<const PackedArrayRef<T>*>(p_base))->array;
    }

    _FORCE_INLINE_ PackedArrayRef(const Vector<T>& p_from) {
      array = p_from;
      refcount.init();
    }
    _FORCE_INLINE_ PackedArrayRef() { refcount.init(); }
  };

  struct ObjData {
    ObjectID id;
    Object* obj = nullptr;
  };

  union {
    bool _bool;
    int64_t _int;
    double _float;
    void* _ptr;  //generic pointer
    ::lain::AABB* _aabb;
    Basis* _basis;
    Transform3D* _transform3d;
    Transform2D* _transform2d;
    Projection* _projection;
    PackedArrayRefBase* packed_array;
    uint8_t _mem[sizeof(ObjData) > (sizeof(real_t) * 4) ? sizeof(ObjData) : (sizeof(real_t) * 4)]{0};
  } _data alignas(8);

  void reference(const Variant& p_variant);
  static bool initialize_ref(Object* p_object);

  L_INLINE const ObjData& _get_obj() const{
	return *reinterpret_cast<const ObjData *>(&_data._mem[0]);
	}
  L_INLINE ObjData& _get_obj(){
		return *reinterpret_cast<ObjData*>(&_data._mem[0]);
	}
  // constructor
  Variant(const Variant*);
  Variant(const Variant**);

  struct Pools {  // 使用这个Allocator分配这几种类型资源的内存
    union BucketSmall {
      BucketSmall() {}
      ~BucketSmall() {}
      Transform2D _transform2d;
      ::lain::AABB _aabb;
    };
    union BucketMedium {
      BucketMedium() {}
      ~BucketMedium() {}
      Basis _basis;
      Transform3D _transform3d;
    };
    union BucketLarge {
      BucketLarge() {}
      ~BucketLarge() {}
      Projection _projection;
    };

    static PagedAllocator<BucketSmall, true> _bucket_small;
    static PagedAllocator<BucketMedium, true> _bucket_medium;
    static PagedAllocator<BucketLarge, true> _bucket_large;
  };

 public:
  String stringify(int recursion_count) const;
  String to_json_string() const;

  _FORCE_INLINE_ Type get_type() const { return type; }
  static const char* get_c_type_name(Variant::Type p_type);
  static String get_type_name(Variant::Type p_type) { return get_c_type_name(p_type); }
  L_INLINE String get_type_name() { return get_type_name(type); }
  void operator=(const Variant& p_variant);  // only this is enough for all the other
  bool operator!=(const Variant& p_variant) const;
  bool operator==(const Variant& p_variant) const;
  bool operator<(const Variant& p_variant) const;
  bool is_type_shared(Variant::Type p_type) const;
  bool is_read_only() const;

  void static_assign(const Variant& p_variant);

  typedef void (*ValidatedConstructor)(Variant* r_base, const Variant** p_args);
  typedef void (*PTRConstructor)(void* base, const void** p_args);

  typedef void (*ObjectConstruct)(const String& p_text, void* ud, Variant& r_value);
  static void construct_from_string(const String& p_string, Variant& r_value, ObjectConstruct p_obj_construct = nullptr, void* p_construct_ud = nullptr);
  static int get_constructor_count(Variant::Type p_type);

  static ValidatedConstructor get_validated_constructor(Variant::Type p_type, int p_constructor);
  static PTRConstructor get_ptr_constructor(Variant::Type p_type, int p_constructor);
  static int get_constructor_argument_count(Variant::Type p_type, int p_constructor);
  static Variant::Type get_constructor_argument_type(Variant::Type p_type, int p_constructor, int p_argument);
  static String get_constructor_argument_name(Variant::Type p_type, int p_constructor, int p_argument);
	// 反射构造
  static void construct(Variant::Type, Variant& base, const Variant** p_args, int p_argcount, Callable::CallError& r_error);
  static void get_constructor_list(Type p_type, List<MethodInfo>* r_list);  //convenience

  uint32_t recursive_hash(int recursion_count) const;
  // 比较相等
  bool hash_compare(const Variant& p_variant, int recursion_count = 0, bool semantic_comparison = true) const;
  bool identity_compare(const Variant& p_variant) const;
  uint32_t Variant::hash() const { return recursive_hash(0); }

  void zero();
  Variant duplicate(bool p_deep = false) const;
  Variant recursive_duplicate(bool p_deep, int recursion_count) const;
  void set_type(Type p_type) { type = p_type; };
  // 装箱
  // containers
  /// Variant transform
  Variant(const Array& p_array);
  Variant(const PackedByteArray& p_byte_array);
  Variant(const PackedInt32Array& p_int32_array);
  Variant(const PackedInt64Array& p_int64_array);
  Variant(const PackedFloat32Array& p_float32_array);
  Variant(const PackedFloat64Array& p_float64_array);
  Variant(const PackedStringArray& p_string_array);
  Variant(const PackedVector2Array& p_vector2_array);
  Variant(const PackedVector3Array& p_vector3_array);
  Variant(const PackedColorArray& p_color_array);
  Variant(const PackedVector4Array& p_vector4_array);

  Variant(const Vector<::lain::RID>& p_array);  // helper
  Variant(const Vector<Plane>& p_array);      // helper
  Variant(const Vector<Face3>& p_face_array);
  Variant(const Vector<Variant>& p_array);
  Variant(const Vector<StringName>& p_array);

  Variant(const Dictionary& p_dictionary);

  // object
  Variant(const Object* p_obj);
  Variant(const Reflection::ReflectionInstance& p_instance);

  // basics
  Variant(bool p_bool);
  Variant(int64_t p_int64);
  Variant(int32_t p_int32);
  Variant(int16_t p_int16);
  Variant(int8_t p_int8);
  Variant(uint64_t p_uint64);
  Variant(uint32_t p_uint32);
  Variant(uint16_t p_uint16);
  Variant(uint8_t p_uint8);
  Variant(float p_float);
  Variant(double p_double);
  // array

  //other class
  Variant(const ObjectID& p_id);
  Variant(const String& p_string);
  Variant(const StringName& p_string);
  Variant(const char* const p_cstring);
  Variant(const char32_t* p_wstring);
  Variant(const Vector2& p_vector2);
  Variant(const Vector2i& p_vector2i);
  Variant(const Rect2& p_rect2);
  Variant(const Rect2i& p_rect2i);
  Variant(const Vector3& p_vector3);
  Variant(const Vector3i& p_vector3i);
  Variant(const Vector4& p_vector4);
  Variant(const Vector4i& p_vector4i);
  Variant(const Plane& p_plane);
  Variant(const ::lain::AABB& p_aabb);
  Variant(const Quaternion& p_quat);
  Variant(const Basis& p_matrix);
  Variant(const Transform2D& p_transform);
  Variant(const Transform3D& p_transform);
  Variant(const Projection& p_projection);
  Variant(const Color& p_color);
  Variant(const GObjectPath& p_node_path);
  Variant(const ::lain::RID& p_rid);
  Variant(const Callable& p_callable);
  Variant(const Signal& p_signal);
  Variant(const IPAddress& p_address);

  // copy construct
  Variant(const Variant& p_variant);

  _FORCE_INLINE_ Variant() { type = NIL; }
  L_INLINE ~Variant() { clear(); }

  // 拆箱
  operator bool() const;
  operator signed int() const;
  operator unsigned int() const;  // this is the real one
  operator signed short() const;
  operator unsigned short() const;
  operator signed char() const;
  operator unsigned char() const;
  //operator long unsigned int() const;
  operator int64_t() const;
  operator uint64_t() const;
#ifdef NEED_LONG_INT
  operator signed long() const;
  operator unsigned long() const;
#endif

  operator ObjectID() const;

  operator char32_t() const;
  operator float() const;
  operator double() const;
  operator String() const;
  operator StringName() const;
  operator Vector2() const;
  operator Vector2i() const;
  operator Rect2() const;
  operator Rect2i() const;
  operator Vector3() const;
  operator Vector3i() const;
  operator Vector4() const;
  operator Vector4i() const;
  operator Plane() const;
  operator ::lain::AABB() const;
  operator Quaternion() const;
  operator Basis() const;
  operator Transform3D() const;
  operator Transform2D() const;
  operator Projection() const;

  operator Color() const;
  operator GObjectPath() const;
  operator ::lain::RID() const;

  operator Object*() const;

  operator Callable() const;
  operator Signal() const;

  operator Dictionary() const;
  operator Array() const;

  operator PackedByteArray() const;
  operator PackedInt32Array() const;
  operator PackedInt64Array() const;
  operator PackedFloat32Array() const;
  operator PackedFloat64Array() const;
  operator PackedStringArray() const;
  operator PackedVector3Array() const;
  operator PackedVector2Array() const;
  operator PackedVector4Array() const;
  operator PackedColorArray() const;

  operator Vector<::lain::RID>() const;
  operator Vector<Plane>() const;
  operator Vector<Face3>() const;
  operator Vector<Variant>() const;
  operator Vector<StringName>() const;

  // some core type enums to convert to
  operator Side() const;
  operator Orientation() const;

  operator IPAddress() const;

  Object* get_validated_object() const;

  // operator

  _FORCE_INLINE_ void clear() {
    static const bool needs_deinit[Variant::VARIANT_MAX] = {
        false,  //NIL,
        false,  //BOOL,
        false,  //INT,
        false,  //FLOAT,
        true,   //STRING,
        false,  //VECTOR2,
        false,  //VECTOR2I,
        false,  //RECT2,
        false,  //RECT2I,
        false,  //VECTOR3,
        false,  //VECTOR3I,
        true,   //TRANSFORM2D,
        false,  //VECTOR4,
        false,  //VECTOR4I,
        false,  //PLANE,
        false,  //QUATERNION,
        true,   //AABB,
        true,   //BASIS,
        true,   //TRANSFORM,
        true,   //PROJECTION,

        // misc types
        false,  //COLOR,
        true,   //STRING_NAME,
        true,   //GOBJECT_PATH,
        false,  //RID,
        true,   //OBJECT,
        true,   //CALLABLE,
        true,   //SIGNAL,
        true,   //DICTIONARY,
        true,   //ARRAY,

        // typed arrays
        true,   //PACKED_BYTE_ARRAY,
        true,   //PACKED_INT32_ARRAY,
        true,   //PACKED_INT64_ARRAY,
        true,   //PACKED_FLOAT32_ARRAY,
        true,   //PACKED_FLOAT64_ARRAY,
        true,   //PACKED_STRING_ARRAY,
        true,   //PACKED_VECTOR2_ARRAY,
        true,   //PACKED_VECTOR3_ARRAY,
        true,   //PACKED_COLOR_ARRAY,
        true,   // PACKED_VECTOR4_ARRAY
        false,  //ReflecionInstance
    };

    if (unlikely(needs_deinit[type])) {  // Make it fast for types that don't need deinit.
      _clear_internal();
    }
    type = NIL;
  }

  void _clear_internal();
  // convert function, type hint
  bool is_ref_counted() const;
  _FORCE_INLINE_ bool is_num() const { return type == INT || type == FLOAT; }
  _FORCE_INLINE_ bool is_string() const { return type == STRING || type == STRING_NAME; }
  _FORCE_INLINE_ bool is_array() const { return type >= ARRAY; }
  bool is_shared() const;
  bool is_zero() const;
  bool is_one() const;
  bool is_null() const;

  /// <summary>
  /// variant operator
  /// </summary>
 public:
  enum Operator {
    //comparison
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL,
    //mathematic
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_POSITIVE,
    OP_MODULE,
    OP_POWER,
    //bitwise
    OP_SHIFT_LEFT,
    OP_SHIFT_RIGHT,
    OP_BIT_AND,
    OP_BIT_OR,
    OP_BIT_XOR,
    OP_BIT_NEGATE,
    //logic
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_NOT,
    //containment
    OP_IN,
    OP_MAX

  };

	static String get_operator_name(Operator p_op);
  static void evaluate(const Operator& p_op, const Variant& p_a, const Variant& p_b, Variant& r_ret, bool& r_valid);
  static _FORCE_INLINE_ Variant evaluate(const Operator& p_op, const Variant& p_a, const Variant& p_b) {
    bool valid = true;
    Variant res;
    evaluate(p_op, p_a, p_b, res, valid);
    return res;
  }

	static Variant::Type get_operator_return_type(Operator p_operator, Type p_type_a, Type p_type_b);
	typedef void (*ValidatedOperatorEvaluator)(const Variant *left, const Variant *right, Variant *r_ret);
	static ValidatedOperatorEvaluator get_validated_operator_evaluator(Operator p_operator, Type p_type_a, Type p_type_b);
	typedef void (*PTROperatorEvaluator)(const void *left, const void *right, void *r_ret);
	static PTROperatorEvaluator get_ptr_operator_evaluator(Operator p_operator, Type p_type_a, Type p_type_b);




  /// <summary>
  /// Keying
  /// </summary>
	// @todo

	void set_keyed(const Variant &p_key, const Variant &p_value, bool &r_valid);
  Variant get_keyed(const Variant& p_key, bool& r_valid) const;
	bool has_key(const Variant &p_key, bool &r_valid) const;


  /// <summary>
  /// Properties
  /// set ，get方法根据类型在表中查找对应的方法。
  /// </summary>


  enum VariantSetError { SET_OK, SET_KEYED_ERR, SET_NAMED_ERR, SET_INDEXED_ERR };
  enum VariantGetError { GET_OK, GET_KEYED_ERR, GET_NAMED_ERR, GET_INDEXED_ERR };
	void set(const Variant &p_index, const Variant &p_value, bool *r_valid = nullptr, VariantSetError *err_code = nullptr);
  Variant get(const Variant& p_index, bool* r_valid, VariantGetError* err_code = nullptr) const;
	bool in(const Variant &p_index, bool *r_valid) const;

	void set_named(const StringName &p_member, const Variant &p_value, bool &r_valid);
	Variant get_named(const StringName &p_member, bool &r_valid) const;

	typedef void (*ValidatedSetter)(Variant *base, const Variant *value);
	typedef void (*ValidatedGetter)(const Variant *base, Variant *value);

	static bool has_member(Variant::Type p_type, const StringName &p_member);
	static Variant::Type get_member_type(Variant::Type p_type, const StringName &p_member);
	static void get_member_list(Type p_type, List<StringName> *r_members);
	static int get_member_count(Type p_type);

	static ValidatedSetter get_member_validated_setter(Variant::Type p_type, const StringName &p_member);
	static ValidatedGetter get_member_validated_getter(Variant::Type p_type, const StringName &p_member);

	typedef void (*PTRSetter)(void *base, const void *value);
	typedef void (*PTRGetter)(const void *base, void *value);

	static PTRSetter get_member_ptr_setter(Variant::Type p_type, const StringName &p_member);
	static PTRGetter get_member_ptr_getter(Variant::Type p_type, const StringName &p_member);
/// <summary>
	/* Indexing */
/// </summary>
	static bool has_indexing(Variant::Type p_type);
	static Variant::Type get_indexed_element_type(Variant::Type p_type);
	static uint32_t get_indexed_element_usage(Variant::Type p_type);

	typedef void (*ValidatedIndexedSetter)(Variant *base, int64_t index, const Variant *value, bool *oob);
	typedef void (*ValidatedIndexedGetter)(const Variant *base, int64_t index, Variant *value, bool *oob);

	static ValidatedIndexedSetter get_member_validated_indexed_setter(Variant::Type p_type);
	static ValidatedIndexedGetter get_member_validated_indexed_getter(Variant::Type p_type);

	typedef void (*PTRIndexedSetter)(void *base, int64_t index, const void *value);
	typedef void (*PTRIndexedGetter)(const void *base, int64_t index, void *value);

	static PTRIndexedSetter get_member_ptr_indexed_setter(Variant::Type p_type);
	static PTRIndexedGetter get_member_ptr_indexed_getter(Variant::Type p_type);

	void set_indexed(int64_t p_index, const Variant &p_value, bool &r_valid, bool &r_oob);
	Variant get_indexed(int64_t p_index, bool &r_valid, bool &r_oob) const;

	uint64_t get_indexed_size() const;


  /// <summary>
  /// UtilityFunction
  /// </summary>

  enum UtilityFunctionType {
    UTILITY_FUNC_TYPE_MATH,
    UTILITY_FUNC_TYPE_RANDOM,
    UTILITY_FUNC_TYPE_GENERAL,
  };

	typedef void (*ValidatedUtilityFunction)(Variant* r_ret, const Variant** p_args, int p_argcount);
  typedef void (*PTRUtilityFunction)(void* r_ret, const void** p_args, int p_argcount);
  bool has_utility_function(const StringName& p_name);
  static ValidatedUtilityFunction get_validated_utility_function(const StringName& p_name);
  static PTRUtilityFunction get_ptr_utility_function(const StringName& p_name);
  static UtilityFunctionType get_utility_function_type(const StringName& p_name);
  static MethodInfo get_utility_function_info(const StringName& p_name);
  static int get_utility_function_argument_count(const StringName& p_name);
  static Variant::Type get_utility_function_argument_type(const StringName& p_name, int p_arg);
  static String get_utility_function_argument_name(const StringName& p_name, int p_arg);
  static bool has_utility_function_return_value(const StringName& p_name);
  static Variant::Type get_utility_function_return_type(const StringName& p_name);
  static bool is_utility_function_vararg(const StringName& p_name);
  static uint32_t get_utility_function_hash(const StringName& p_name);

  static void get_utility_function_list(List<StringName>* r_functions);
  static int get_utility_function_count();

  static void call_utility_function(const StringName& p_name, Variant* r_ret, const Variant** p_args, int p_argcount, Callable::CallError& r_error);
  Object* get_validated_object_with_check(bool& r_previously_freed) const;
	/// buil-in  method
	/// buil-in  method
	/// built-in method 在 variant_callable.h中实现
		typedef void (*ValidatedBuiltInMethod)(Variant *base, const Variant **p_args, int p_argcount, Variant *r_ret);
	typedef void (*PTRBuiltInMethod)(void *p_base, const void **p_args, void *r_ret, int p_argcount);
	static bool has_builtin_method_return_value(Variant::Type p_type, const StringName &p_method) ;
	static bool has_builtin_method(Variant::Type p_type, const StringName &p_method) ;
	static int get_builtin_method_argument_count(Variant::Type p_type, const StringName &p_method) ;
	static uint32_t get_builtin_method_hash(Variant::Type p_type, const StringName &p_method);
	void callp(const StringName &p_method, const Variant **p_args, int p_argcount, Variant &r_ret, Callable::CallError &r_error);

	/// <summary>
	/// Constructor
	/// </summary>
  String get_construct_string() const;
  static String get_call_error_text(const StringName& p_method, const Variant** p_argptrs, int p_argcount, const Callable::CallError& ce);
  static String get_call_error_text(Object* p_base, const StringName& p_method, const Variant** p_argptrs, int p_argcount, const Callable::CallError& ce);
  static String get_callable_error_text(const Callable& p_callable, const Variant** p_argptrs, int p_argcount, const Callable::CallError& ce);
  void _variant_call_error(const String& p_method, Callable::CallError& error);

  static void _register_variant_operators();
  static void _unregister_variant_operators();
  static void _register_variant_methods();
  static void _unregister_variant_methods();
  static void _register_variant_setters_getters();
  static void _unregister_variant_setters_getters();
  static void _register_variant_constructors();
  static void _unregister_variant_destructors();
  static void _register_variant_destructors();
  static void _unregister_variant_constructors();
  static void _register_variant_utility_functions();
  static void _unregister_variant_utility_functions();


	// helper
  L_INLINE bool booleanize() const { return !is_zero(); }

};

/// <summary>
/// helper functions
/// </summary>

template <typename... VarArgs>
String vformat(const String& p_text, const VarArgs... p_args) {
  Variant args[sizeof...(p_args) + 1] = {p_args..., Variant()};  // +1 makes sure zero sized arrays are also supported.
  Array args_array;
  args_array.resize(sizeof...(p_args));
  for (uint32_t i = 0; i < sizeof...(p_args); i++) {
    args_array[i] = args[i];
  }

  bool error = false;
  String fmt = p_text.sprintf(args_array, &error);

  ERR_FAIL_COND_V_MSG(error, String(), fmt);

  return fmt;
}
struct VariantHasher {
  static _FORCE_INLINE_ uint32_t hash(const Variant& p_variant) { return p_variant.hash(); }
};
struct StringLikeVariantComparator {
  static bool compare(const Variant& p_lhs, const Variant& p_rhs);
};

struct VariantComparator {
  static _FORCE_INLINE_ bool compare(const Variant& p_lhs, const Variant& p_rhs) { return p_lhs.hash_compare(p_rhs); }
};


};  // namespace lain

#endif  // !__VARIANT_H__
