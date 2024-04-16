#pragma once
#ifndef VARIANT_HELPER
#define VARIANT_HELPER
#include "core/variant/variant.h"
#include "core/meta/reflection/reflection.h"
namespace lain {

class VariantHelper {
public:
	static Vector<const char*> variant_basic_reflect_types;
	//static String get_type_name(const Variant& p_var);
	static bool is_serializable(const Variant& p_var);
	static bool is_serializable_type(const char* type_name);

	static bool is_serializable_type(const Variant::Type p_type) {
		return is_serializable_type(Variant::get_c_type_name(p_type));
	}
	static Variant::Type get_type_from_name(const char* p_name);
	static void* get_ptr();
};
}

#endif // !VARIANT_HELPER