#ifndef TYPED_ARRAY_H
#define TYPED_ARRAY_H

#include "core/variant/array.h"
#include "core/variant/variant.h"
namespace lain {

template <class T>
class TypedArray : public Array {
public:
	_FORCE_INLINE_ void operator=(const Array& p_array) {
		ERR_FAIL_COND_MSG(!is_same_typed(p_array), "Cannot assign an array with a different element type.");
		_ref(p_array);
	}
	_FORCE_INLINE_ TypedArray(const Variant& p_variant) :
		Array(Array(p_variant), Variant::OBJECT, T::get_class_static(), Variant()) {
	}
	_FORCE_INLINE_ TypedArray(const Array& p_array) :
		Array(p_array, Variant::OBJECT, T::get_class_static(), Variant()) {
	}
	_FORCE_INLINE_ TypedArray() {
		set_typed(Variant::OBJECT, T::get_class_static(), Variant());
	}
};
}

#endif