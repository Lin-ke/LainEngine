#ifndef VARIANT_BINDER_COMMON_H 
#define VARIANT_BINDER_COMMON_H
#include "core/object/object.h"
#include "core/variant/method_ptrcall.h"
#include "core/meta/type_info.h"
#include "core/typedefs.h"
#include "variant.h"
#include "variant_internal.h"

namespace lain {
// 如果是 Object的子类，转成Object
template <typename T>
struct VariantCaster {
  static _FORCE_INLINE_ T cast(const Variant& p_variant) {
    using TStripped = std::remove_pointer_t<T>;
    if constexpr (std::is_base_of_v<Object, TStripped>) {
      return Object::cast_to<TStripped>(p_variant);
    } else {
      return p_variant;
    }
  }
};

template <typename T>
struct VariantCaster<T&> {
  static _FORCE_INLINE_ T cast(const Variant& p_variant) {
    using TStripped = std::remove_pointer_t<T>;
    if constexpr (std::is_base_of_v<Object, TStripped>) {
      return Object::cast_to<TStripped>(p_variant);
    } else {
      return p_variant;
    }
  }
};

template <typename T>
struct VariantCaster<const T&> {
  static _FORCE_INLINE_ T cast(const Variant& p_variant) {
    using TStripped = std::remove_pointer_t<T>;
    if constexpr (std::is_base_of_v<Object, TStripped>) {
      return Object::cast_to<TStripped>(p_variant);
    } else {
      return p_variant;
    }
  }
};
template <>
struct VariantCaster<char32_t> {
  static _FORCE_INLINE_ char32_t cast(const Variant& p_variant) { return (char32_t)p_variant.operator int(); }
};

template <>
struct PtrToArg<char32_t> {
  _FORCE_INLINE_ static char32_t convert(const void* p_ptr) { return char32_t(*reinterpret_cast<const int*>(p_ptr)); }
  typedef int64_t EncodeT;
  _FORCE_INLINE_ static void encode(char32_t p_val, const void* p_ptr) { *(int*)p_ptr = p_val; }
};

template <typename Q>
void call_get_argument_type_helper(int p_arg, int& index, Variant::Type& type) {
  if (p_arg == index) {
    type = GetTypeInfo<Q>::VARIANT_TYPE;
  }
  index++;
}

template <typename... P>
Variant::Type call_get_argument_type(int p_arg) {
  Variant::Type type = Variant::NIL;
  int index = 0;
  // I think rocket science is simpler than modern C++.
  using expand_type = int[];
  // 逗号表达式
  expand_type a{0, (call_get_argument_type_helper<P>(p_arg, index, type), 0)...};
  (void)a;      // Suppress (valid, but unavoidable) -Wunused-variable warning.
  (void)index;  // Suppress GCC warning.
  return type;
}

template <typename T>
struct VariantObjectClassChecker {
	static _FORCE_INLINE_ bool check(const Variant &p_variant) {
		using TStripped = std::remove_pointer_t<T>;
		if constexpr (std::is_base_of_v<Object, TStripped>) {
			Object *obj = p_variant;
			return Object::cast_to<TStripped>(p_variant) || !obj;
		} else {
			return true;
		}
	}
};

template <typename T>
class Ref;

template <typename T>
struct VariantObjectClassChecker<const Ref<T> &> {
	static _FORCE_INLINE_ bool check(const Variant &p_variant) {
		Object *obj = p_variant;
		const Ref<T> node = p_variant;
		return node.ptr() || !obj;
	}
};


template <typename T>
struct VariantCasterAndValidate {
  static _FORCE_INLINE_ T cast(const Variant** p_args, uint32_t p_arg_idx, Callable::CallError& r_error) {
    Variant::Type argtype = GetTypeInfo<T>::VARIANT_TYPE;
    if (!Variant::can_convert_strict(p_args[p_arg_idx]->get_type(), argtype) || !VariantObjectClassChecker<T>::check(*p_args[p_arg_idx])) {
      r_error.error = Callable::CallError::CALL_ERROR_INVALID_ARGUMENT;
      r_error.argument = p_arg_idx;
      r_error.expected = argtype;
    }

    return VariantCaster<T>::cast(*p_args[p_arg_idx]);
  }
};

template <typename T>
struct VariantCasterAndValidate<T&> {
  static _FORCE_INLINE_ T cast(const Variant** p_args, uint32_t p_arg_idx, Callable::CallError& r_error) {
    Variant::Type argtype = GetTypeInfo<T>::VARIANT_TYPE;
    if (!Variant::can_convert_strict(p_args[p_arg_idx]->get_type(), argtype) || !VariantObjectClassChecker<T>::check(*p_args[p_arg_idx])) {
      r_error.error = Callable::CallError::CALL_ERROR_INVALID_ARGUMENT;
      r_error.argument = p_arg_idx;
      r_error.expected = argtype;
    }

    return VariantCaster<T>::cast(*p_args[p_arg_idx]);
  }
};

template <typename T>
struct VariantCasterAndValidate<const T&> {
  static _FORCE_INLINE_ T cast(const Variant** p_args, uint32_t p_arg_idx, Callable::CallError& r_error) {
    Variant::Type argtype = GetTypeInfo<T>::VARIANT_TYPE;
    if (!Variant::can_convert_strict(p_args[p_arg_idx]->get_type(), argtype) || !VariantObjectClassChecker<T>::check(*p_args[p_arg_idx])) {
      r_error.error = Callable::CallError::CALL_ERROR_INVALID_ARGUMENT;
      r_error.argument = p_arg_idx;
      r_error.expected = argtype;
    }

    return VariantCaster<T>::cast(*p_args[p_arg_idx]);
  }
};

#define VARIANT_ENUM_CAST(m_enum)                                        \
  MAKE_ENUM_TYPE_INFO(m_enum)                                            \
  template <>                                                            \
  struct VariantCaster<m_enum> {                                         \
    static _FORCE_INLINE_ m_enum cast(const Variant& p_variant) {        \
      return (m_enum)p_variant.operator int64_t();                       \
    }                                                                    \
  };                                                                     \
  template <>                                                            \
  struct PtrToArg<m_enum> {                                              \
    _FORCE_INLINE_ static m_enum convert(const void* p_ptr) {            \
      return m_enum(*reinterpret_cast<const int64_t*>(p_ptr));           \
    }                                                                    \
    typedef int64_t EncodeT;                                             \
    _FORCE_INLINE_ static void encode(m_enum p_val, const void* p_ptr) { \
      *(int64_t*)p_ptr = (int64_t)p_val;                                 \
    }                                                                    \
  };                                                                     \
  template <>                                                            \
  struct ZeroInitializer<m_enum> {                                       \
    static void initialize(m_enum& value) {                              \
      value = (m_enum)0;                                                 \
    }                                                                    \
  };                                                                     \
  template <>                                                            \
  struct VariantInternalAccessor<m_enum> {                               \
    static _FORCE_INLINE_ m_enum get(const Variant* v) {                 \
      return m_enum(*VariantInternal::get_int(v));                       \
    }                                                                    \
    static _FORCE_INLINE_ void set(Variant* v, m_enum p_value) {         \
      *VariantInternal::get_int(v) = (int64_t)p_value;                   \
    }                                                                    \
  };

  VARIANT_ENUM_CAST(Vector2::Axis);
  VARIANT_ENUM_CAST(Vector2i::Axis);
  VARIANT_ENUM_CAST(Vector3::Axis);
  VARIANT_ENUM_CAST(Vector3i::Axis);
  VARIANT_ENUM_CAST(Vector4::Axis);
  VARIANT_ENUM_CAST(Vector4i::Axis);
  VARIANT_ENUM_CAST(Error);
  VARIANT_ENUM_CAST(Variant::Type);
} // namespace lain
#endif