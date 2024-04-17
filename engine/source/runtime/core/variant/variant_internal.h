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
			switch (p_type) {
			case Variant::STRING:
				init_string(v);
				break;
			/*case Variant::TRANSFORM2D:
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
				break;*/
			case Variant::COLOR:
				init_color(v);
				break;
			case Variant::STRING_NAME:
				init_string_name(v);
				break;
			case Variant::GOBJECT_PATH:
				init_GOBJECT_PATH(v);
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
			/*case Variant::OBJECT:
				init_object(v);
				break;*/
			default:
				break;
			}
		}
			_FORCE_INLINE_ static void init_string(Variant *v) {
		memnew_placement(v->_data._mem, String);
		v->type = Variant::STRING;
	}
	/*_FORCE_INLINE_ static void init_transform2d(Variant *v) {
		v->_data._transform2d = (Transform2D *)Variant::Pools::_bucket_small.alloc();
		memnew_placement(v->_data._transform2d, Transform2D);
		v->type = Variant::TRANSFORM2D;
	}
	_FORCE_INLINE_ static void init_aabb(Variant *v) {
		v->_data._aabb = (AABB *)Variant::Pools::_bucket_small.alloc();
		memnew_placement(v->_data._aabb, AABB);
		v->type = Variant::AABB;
	}
	_FORCE_INLINE_ static void init_basis(Variant *v) {
		v->_data._basis = (Basis *)Variant::Pools::_bucket_medium.alloc();
		memnew_placement(v->_data._basis, Basis);
		v->type = Variant::BASIS;
	}
	_FORCE_INLINE_ static void init_transform3d(Variant *v) {
		v->_data._transform3d = (Transform3D *)Variant::Pools::_bucket_medium.alloc();
		memnew_placement(v->_data._transform3d, Transform3D);
		v->type = Variant::TRANSFORM3D;
	}
	_FORCE_INLINE_ static void init_projection(Variant *v) {
		v->_data._projection = (Projection *)Variant::Pools::_bucket_large.alloc();
		memnew_placement(v->_data._projection, Projection);
		v->type = Variant::PROJECTION;
	}*/
	_FORCE_INLINE_ static void init_color(Variant *v) {
		memnew_placement(v->_data._mem, Color);
		v->type = Variant::COLOR;
	}
	_FORCE_INLINE_ static void init_string_name(Variant *v) {
		memnew_placement(v->_data._mem, StringName);
		v->type = Variant::STRING_NAME;
	}
	_FORCE_INLINE_ static void init_GOBJECT_PATH(Variant *v) {
		memnew_placement(v->_data._mem, GObjectPath);
		v->type = Variant::GOBJECT_PATH;
	}
	_FORCE_INLINE_ static void init_callable(Variant *v) {
		memnew_placement(v->_data._mem, Callable);
		v->type = Variant::CALLABLE;
	}
	_FORCE_INLINE_ static void init_signal(Variant *v) {
		memnew_placement(v->_data._mem, Signal);
		v->type = Variant::SIGNAL;
	}
	_FORCE_INLINE_ static void init_dictionary(Variant *v) {
		memnew_placement(v->_data._mem, Dictionary);
		v->type = Variant::DICTIONARY;
	}
	_FORCE_INLINE_ static void init_array(Variant *v) {
		memnew_placement(v->_data._mem, Array);
		v->type = Variant::ARRAY;
	}
	_FORCE_INLINE_ static void init_byte_array(Variant *v) {
		v->_data.packed_array = Variant::PackedArrayRef<uint8_t>::create(Vector<uint8_t>());
		v->type = Variant::PACKED_BYTE_ARRAY;
	}
	_FORCE_INLINE_ static void init_int32_array(Variant *v) {
		v->_data.packed_array = Variant::PackedArrayRef<int32_t>::create(Vector<int32_t>());
		v->type = Variant::PACKED_INT32_ARRAY;
	}
	_FORCE_INLINE_ static void init_int64_array(Variant *v) {
		v->_data.packed_array = Variant::PackedArrayRef<int64_t>::create(Vector<int64_t>());
		v->type = Variant::PACKED_INT64_ARRAY;
	}
	_FORCE_INLINE_ static void init_float32_array(Variant *v) {
		v->_data.packed_array = Variant::PackedArrayRef<float>::create(Vector<float>());
		v->type = Variant::PACKED_FLOAT32_ARRAY;
	}
	_FORCE_INLINE_ static void init_float64_array(Variant *v) {
		v->_data.packed_array = Variant::PackedArrayRef<double>::create(Vector<double>());
		v->type = Variant::PACKED_FLOAT64_ARRAY;
	}
	_FORCE_INLINE_ static void init_string_array(Variant *v) {
		v->_data.packed_array = Variant::PackedArrayRef<String>::create(Vector<String>());
		v->type = Variant::PACKED_STRING_ARRAY;
	}
	_FORCE_INLINE_ static void init_vector2_array(Variant *v) {
		v->_data.packed_array = Variant::PackedArrayRef<Vector2>::create(Vector<Vector2>());
		v->type = Variant::PACKED_VECTOR2_ARRAY;
	}
	_FORCE_INLINE_ static void init_vector3_array(Variant *v) {
		v->_data.packed_array = Variant::PackedArrayRef<Vector3>::create(Vector<Vector3>());
		v->type = Variant::PACKED_VECTOR3_ARRAY;
	}
	_FORCE_INLINE_ static void init_color_array(Variant *v) {
		v->_data.packed_array = Variant::PackedArrayRef<Color>::create(Vector<Color>());
		v->type = Variant::PACKED_COLOR_ARRAY;
	}
	/*_FORCE_INLINE_ static void init_object(Variant *v) {
		object_assign_null(v);
		v->type = Variant::OBJECT;
	}*/
	};
	
}

#endif // !VARIANT_INTERNAL_H
