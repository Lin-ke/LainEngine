#include "variant.h"
#include "method_ptrcall.h"
namespace lain{
// 如果是 Object的子类，转成Object
template <typename T>
struct VariantCaster {
	static _FORCE_INLINE_ T cast(const Variant &p_variant) {
		using TStripped = std::remove_pointer_t<T>;
		if constexpr (std::is_base_of_v<Object, TStripped>) {
			return Object::cast_to<TStripped>(p_variant);
		} else {
			return p_variant;
		}
	}
};

template <typename T>
struct VariantCaster<T &> {
	static _FORCE_INLINE_ T cast(const Variant &p_variant) {
		using TStripped = std::remove_pointer_t<T>;
		if constexpr (std::is_base_of_v<Object, TStripped>) {
			return Object::cast_to<TStripped>(p_variant);
		} else {
			return p_variant;
		}
	}
};

template <typename T>
struct VariantCaster<const T &> {
	static _FORCE_INLINE_ T cast(const Variant &p_variant) {
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
	static _FORCE_INLINE_ char32_t cast(const Variant &p_variant) {
		return (char32_t)p_variant.operator int();
	}
};

template <>
struct PtrToArg<char32_t> {
	_FORCE_INLINE_ static char32_t convert(const void *p_ptr) {
		return char32_t(*reinterpret_cast<const int *>(p_ptr));
	}
	typedef int64_t EncodeT;
	_FORCE_INLINE_ static void encode(char32_t p_val, const void *p_ptr) {
		*(int *)p_ptr = p_val;
	}
};


}
