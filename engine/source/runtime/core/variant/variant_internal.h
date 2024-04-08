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
		static void initialize(Variant* v, Variant::Type p_type) {
			v->clear();
			v->type = p_type;
		}

			/*switch (p_type) {
			case Variant::STRING:
				init_string(v);
				break;
			case Variant::TRANSFORM2D:
				init_transform2d(v);
				break;
			case Variant::AABB:
				init_aabb(v);
				break;
			case Variant::BASIS:
				init_basis(v);
				break;
			case Variant::TRANSFORM3D:
				init_transform3d(v);
				break;
			case Variant::PROJECTION:
				init_projection(v);
				break;
			case Variant::COLOR:
				init_color(v);
				break;
			case Variant::STRING_NAME:
				init_string_name(v);
				break;
			case Variant::GOBJECT_PATH:
				init_node_path(v);
				break;
			case Variant::CALLABLE:
				init_callable(v);
				break;
			case Variant::SIGNAL:
				init_signal(v);
				break;
			case Variant::DICTIONARY:
				init_dictionary(v);
				break;
			case Variant::ARRAY:
				init_array(v);
				break;
			case Variant::PACKED_BYTE_ARRAY:
				init_byte_array(v);
				break;
			case Variant::PACKED_INT32_ARRAY:
				init_int32_array(v);
				break;
			case Variant::PACKED_INT64_ARRAY:
				init_int64_array(v);
				break;
			case Variant::PACKED_FLOAT32_ARRAY:
				init_float32_array(v);
				break;
			case Variant::PACKED_FLOAT64_ARRAY:
				init_float64_array(v);
				break;
			case Variant::PACKED_STRING_ARRAY:
				init_string_array(v);
				break;
			case Variant::PACKED_VECTOR2_ARRAY:
				init_vector2_array(v);
				break;
			case Variant::PACKED_VECTOR3_ARRAY:
				init_vector3_array(v);
				break;
			case Variant::PACKED_COLOR_ARRAY:
				init_color_array(v);
				break;
			case Variant::OBJECT:
				init_object(v);
				break;
			default:
				break;
			}*/
	};
	
}

#endif // !VARIANT_INTERNAL_H
