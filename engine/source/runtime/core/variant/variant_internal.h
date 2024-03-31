#pragma once
#ifndef VARIANT_INTERNAL_H
#define VARIANT_INTERNAL_H
#include "variant.h"
namespace lain {

	class VariantInternal {
		friend class Variant;
	public:
		_FORCE_INLINE_ static StringName* get_string_name(Variant* v) { return reinterpret_cast<StringName*>(v->_data._mem); }
		_FORCE_INLINE_ static const StringName* get_string_name(const Variant* v) { return reinterpret_cast<const StringName*>(v->_data._mem); }

	private:

	};
	
}

#endif // !VARIANT_INTERNAL_H
