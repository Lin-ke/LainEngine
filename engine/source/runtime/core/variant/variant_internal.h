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
			case Variant::OBJECT:
				init_object(v);
				break;
			default:
				break;
			}
		}
			_FORCE_INLINE_ static void init_string(Variant *v) {
		memnew_placement(v->_data._mem, String);
		v->type = Variant::STRING;
	}
	_FORCE_INLINE_ static void init_transform2d(Variant *v) {
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
	}
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

	static void object_assign(Variant *v, const Object *o){} // Needs RefCounted, so it's implemented elsewhere.
	_FORCE_INLINE_ static void object_assign(Variant *v, const Variant *o) {
		object_assign(v, o->_get_obj().obj);
	}
	// 注意对 object类型是初始化为 null 的
	_FORCE_INLINE_ static void object_assign_null(Variant *v) {
		v->_get_obj().obj = nullptr;
		v->_get_obj().id = ObjectID();
	}

	_FORCE_INLINE_ static void init_object(Variant *v) {
		object_assign_null(v);
		v->type = Variant::OBJECT;
	}


	// Atomic types.
	_FORCE_INLINE_ static bool *get_bool(Variant *v) { return &v->_data._bool; }
	_FORCE_INLINE_ static const bool *get_bool(const Variant *v) { return &v->_data._bool; }
	_FORCE_INLINE_ static int64_t *get_int(Variant *v) { return &v->_data._int; }
	_FORCE_INLINE_ static const int64_t *get_int(const Variant *v) { return &v->_data._int; }
	_FORCE_INLINE_ static double *get_float(Variant *v) { return &v->_data._float; }
	_FORCE_INLINE_ static const double *get_float(const Variant *v) { return &v->_data._float; }
	_FORCE_INLINE_ static String *get_string(Variant *v) { return reinterpret_cast<String *>(v->_data._mem); }
	_FORCE_INLINE_ static const String *get_string(const Variant *v) { return reinterpret_cast<const String *>(v->_data._mem); }

	// Math types.
	_FORCE_INLINE_ static Vector2 *get_vector2(Variant *v) { return reinterpret_cast<Vector2 *>(v->_data._mem); }
	_FORCE_INLINE_ static const Vector2 *get_vector2(const Variant *v) { return reinterpret_cast<const Vector2 *>(v->_data._mem); }
	_FORCE_INLINE_ static Vector2i *get_vector2i(Variant *v) { return reinterpret_cast<Vector2i *>(v->_data._mem); }
	_FORCE_INLINE_ static const Vector2i *get_vector2i(const Variant *v) { return reinterpret_cast<const Vector2i *>(v->_data._mem); }
	_FORCE_INLINE_ static Rect2 *get_rect2(Variant *v) { return reinterpret_cast<Rect2 *>(v->_data._mem); }
	_FORCE_INLINE_ static const Rect2 *get_rect2(const Variant *v) { return reinterpret_cast<const Rect2 *>(v->_data._mem); }
	_FORCE_INLINE_ static Rect2i *get_rect2i(Variant *v) { return reinterpret_cast<Rect2i *>(v->_data._mem); }
	_FORCE_INLINE_ static const Rect2i *get_rect2i(const Variant *v) { return reinterpret_cast<const Rect2i *>(v->_data._mem); }
	_FORCE_INLINE_ static Vector3 *get_vector3(Variant *v) { return reinterpret_cast<Vector3 *>(v->_data._mem); }
	_FORCE_INLINE_ static const Vector3 *get_vector3(const Variant *v) { return reinterpret_cast<const Vector3 *>(v->_data._mem); }
	_FORCE_INLINE_ static Vector3i *get_vector3i(Variant *v) { return reinterpret_cast<Vector3i *>(v->_data._mem); }
	_FORCE_INLINE_ static const Vector3i *get_vector3i(const Variant *v) { return reinterpret_cast<const Vector3i *>(v->_data._mem); }
	_FORCE_INLINE_ static Vector4 *get_vector4(Variant *v) { return reinterpret_cast<Vector4 *>(v->_data._mem); }
	_FORCE_INLINE_ static const Vector4 *get_vector4(const Variant *v) { return reinterpret_cast<const Vector4 *>(v->_data._mem); }
	_FORCE_INLINE_ static Vector4i *get_vector4i(Variant *v) { return reinterpret_cast<Vector4i *>(v->_data._mem); }
	_FORCE_INLINE_ static const Vector4i *get_vector4i(const Variant *v) { return reinterpret_cast<const Vector4i *>(v->_data._mem); }
	_FORCE_INLINE_ static Transform2D *get_transform2d(Variant *v) { return v->_data._transform2d; }
	_FORCE_INLINE_ static const Transform2D *get_transform2d(const Variant *v) { return v->_data._transform2d; }
	_FORCE_INLINE_ static Plane *get_plane(Variant *v) { return reinterpret_cast<Plane *>(v->_data._mem); }
	_FORCE_INLINE_ static const Plane *get_plane(const Variant *v) { return reinterpret_cast<const Plane *>(v->_data._mem); }
	_FORCE_INLINE_ static Quaternion *get_quaternion(Variant *v) { return reinterpret_cast<Quaternion *>(v->_data._mem); }
	_FORCE_INLINE_ static const Quaternion *get_quaternion(const Variant *v) { return reinterpret_cast<const Quaternion *>(v->_data._mem); }
	_FORCE_INLINE_ static lain::AABB *get_aabb(Variant *v) { return v->_data._aabb; }
	_FORCE_INLINE_ static const lain::AABB *get_aabb(const Variant *v) { return v->_data._aabb; }
	_FORCE_INLINE_ static Basis *get_basis(Variant *v) { return v->_data._basis; }
	_FORCE_INLINE_ static const Basis *get_basis(const Variant *v) { return v->_data._basis; }
	_FORCE_INLINE_ static Transform3D *get_transform(Variant *v) { return v->_data._transform3d; }
	_FORCE_INLINE_ static const Transform3D *get_transform(const Variant *v) { return v->_data._transform3d; }
	_FORCE_INLINE_ static Projection *get_projection(Variant *v) { return v->_data._projection; }
	_FORCE_INLINE_ static const Projection *get_projection(const Variant *v) { return v->_data._projection; }

	// Misc types.
	_FORCE_INLINE_ static Color *get_color(Variant *v) { return reinterpret_cast<Color *>(v->_data._mem); }
	_FORCE_INLINE_ static const Color *get_color(const Variant *v) { return reinterpret_cast<const Color *>(v->_data._mem); }
	_FORCE_INLINE_ static StringName *get_string_name(Variant *v) { return reinterpret_cast<StringName *>(v->_data._mem); }
	_FORCE_INLINE_ static const StringName *get_string_name(const Variant *v) { return reinterpret_cast<const StringName *>(v->_data._mem); }
	_FORCE_INLINE_ static GObjectPath *get_node_path(Variant *v) { return reinterpret_cast<GObjectPath *>(v->_data._mem); }
	_FORCE_INLINE_ static const GObjectPath *get_node_path(const Variant *v) { return reinterpret_cast<const GObjectPath *>(v->_data._mem); }
	_FORCE_INLINE_ static lain::RID *get_rid(Variant *v) { return reinterpret_cast<lain::RID *>(v->_data._mem); }
	_FORCE_INLINE_ static const lain::RID *get_rid(const Variant *v) { return reinterpret_cast<const lain::RID *>(v->_data._mem); }
	_FORCE_INLINE_ static Callable *get_callable(Variant *v) { return reinterpret_cast<Callable *>(v->_data._mem); }
	_FORCE_INLINE_ static const Callable *get_callable(const Variant *v) { return reinterpret_cast<const Callable *>(v->_data._mem); }
	_FORCE_INLINE_ static Signal *get_signal(Variant *v) { return reinterpret_cast<Signal *>(v->_data._mem); }
	_FORCE_INLINE_ static const Signal *get_signal(const Variant *v) { return reinterpret_cast<const Signal *>(v->_data._mem); }
	_FORCE_INLINE_ static Dictionary *get_dictionary(Variant *v) { return reinterpret_cast<Dictionary *>(v->_data._mem); }
	_FORCE_INLINE_ static const Dictionary *get_dictionary(const Variant *v) { return reinterpret_cast<const Dictionary *>(v->_data._mem); }
	_FORCE_INLINE_ static Array *get_array(Variant *v) { return reinterpret_cast<Array *>(v->_data._mem); }
	_FORCE_INLINE_ static const Array *get_array(const Variant *v) { return reinterpret_cast<const Array *>(v->_data._mem); }

	// Typed arrays.
	_FORCE_INLINE_ static PackedByteArray *get_byte_array(Variant *v) { return &static_cast<Variant::PackedArrayRef<uint8_t> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static const PackedByteArray *get_byte_array(const Variant *v) { return &static_cast<const Variant::PackedArrayRef<uint8_t> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static PackedInt32Array *get_int32_array(Variant *v) { return &static_cast<Variant::PackedArrayRef<int32_t> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static const PackedInt32Array *get_int32_array(const Variant *v) { return &static_cast<const Variant::PackedArrayRef<int32_t> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static PackedInt64Array *get_int64_array(Variant *v) { return &static_cast<Variant::PackedArrayRef<int64_t> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static const PackedInt64Array *get_int64_array(const Variant *v) { return &static_cast<const Variant::PackedArrayRef<int64_t> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static PackedFloat32Array *get_float32_array(Variant *v) { return &static_cast<Variant::PackedArrayRef<float> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static const PackedFloat32Array *get_float32_array(const Variant *v) { return &static_cast<const Variant::PackedArrayRef<float> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static PackedFloat64Array *get_float64_array(Variant *v) { return &static_cast<Variant::PackedArrayRef<double> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static const PackedFloat64Array *get_float64_array(const Variant *v) { return &static_cast<const Variant::PackedArrayRef<double> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static PackedStringArray *get_string_array(Variant *v) { return &static_cast<Variant::PackedArrayRef<String> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static const PackedStringArray *get_string_array(const Variant *v) { return &static_cast<const Variant::PackedArrayRef<String> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static PackedVector2Array *get_vector2_array(Variant *v) { return &static_cast<Variant::PackedArrayRef<Vector2> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static const PackedVector2Array *get_vector2_array(const Variant *v) { return &static_cast<const Variant::PackedArrayRef<Vector2> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static PackedVector3Array *get_vector3_array(Variant *v) { return &static_cast<Variant::PackedArrayRef<Vector3> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static const PackedVector3Array *get_vector3_array(const Variant *v) { return &static_cast<const Variant::PackedArrayRef<Vector3> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static PackedColorArray *get_color_array(Variant *v) { return &static_cast<Variant::PackedArrayRef<Color> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static const PackedColorArray *get_color_array(const Variant *v) { return &static_cast<const Variant::PackedArrayRef<Color> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static PackedVector4Array *get_vector4_array(Variant *v) { return &static_cast<Variant::PackedArrayRef<Vector4> *>(v->_data.packed_array)->array; }
	_FORCE_INLINE_ static const PackedVector4Array *get_vector4_array(const Variant *v) { return &static_cast<const Variant::PackedArrayRef<Vector4> *>(v->_data.packed_array)->array; }

	_FORCE_INLINE_ static Object **get_object(Variant *v) { return (Object **)&v->_get_obj().obj; }
	_FORCE_INLINE_ static const Object **get_object(const Variant *v) { return (const Object **)&v->_get_obj().obj; }

	_FORCE_INLINE_ static const ObjectID get_object_id(const Variant *v) { return v->_get_obj().id; }


	_FORCE_INLINE_ static void clear(Variant *v) {
		v->clear();
	}
	
	};
// 为 variant 类型转换提供支持
// clear之前的并init

template <typename T>
struct VariantTypeChanger {
	static _FORCE_INLINE_ void change(Variant *v) {
		if (v->get_type() != GetTypeInfo<T>::VARIANT_TYPE || GetTypeInfo<T>::VARIANT_TYPE >= Variant::PACKED_BYTE_ARRAY) { //second condition removed by optimizer
			VariantInternal::clear(v);
			VariantInitializer<T>::init(v);
		}
	}
	static _FORCE_INLINE_ void change_and_reset(Variant *v) {
		if (v->get_type() != GetTypeInfo<T>::VARIANT_TYPE || GetTypeInfo<T>::VARIANT_TYPE >= Variant::PACKED_BYTE_ARRAY) { //second condition removed by optimizer
			VariantInternal::clear(v);
			VariantInitializer<T>::init(v);
		}

		VariantDefaultInitializer<T>::init(v);
	}
};
template <typename T>
using GetSimpleTypeT = typename std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T>
struct VariantTypeAdjust {
	_FORCE_INLINE_ static void adjust(Variant *r_ret) {
		VariantTypeChanger<GetSimpleTypeT<T>>::change(r_ret);
	}
};


template <typename T>
struct VariantGetInternalPtr {
};

template <>
struct VariantGetInternalPtr<bool> {
	static bool *get_ptr(Variant *v) { return VariantInternal::get_bool(v); }
	static const bool *get_ptr(const Variant *v) { return VariantInternal::get_bool(v); }
};

template <>
struct VariantGetInternalPtr<int8_t> {
	static int64_t *get_ptr(Variant *v) { return VariantInternal::get_int(v); }
	static const int64_t *get_ptr(const Variant *v) { return VariantInternal::get_int(v); }
};

template <>
struct VariantGetInternalPtr<uint8_t> {
	static int64_t *get_ptr(Variant *v) { return VariantInternal::get_int(v); }
	static const int64_t *get_ptr(const Variant *v) { return VariantInternal::get_int(v); }
};

template <>
struct VariantGetInternalPtr<int16_t> {
	static int64_t *get_ptr(Variant *v) { return VariantInternal::get_int(v); }
	static const int64_t *get_ptr(const Variant *v) { return VariantInternal::get_int(v); }
};

template <>
struct VariantGetInternalPtr<uint16_t> {
	static int64_t *get_ptr(Variant *v) { return VariantInternal::get_int(v); }
	static const int64_t *get_ptr(const Variant *v) { return VariantInternal::get_int(v); }
};

template <>
struct VariantGetInternalPtr<int32_t> {
	static int64_t *get_ptr(Variant *v) { return VariantInternal::get_int(v); }
	static const int64_t *get_ptr(const Variant *v) { return VariantInternal::get_int(v); }
};

template <>
struct VariantGetInternalPtr<uint32_t> {
	static int64_t *get_ptr(Variant *v) { return VariantInternal::get_int(v); }
	static const int64_t *get_ptr(const Variant *v) { return VariantInternal::get_int(v); }
};

template <>
struct VariantGetInternalPtr<int64_t> {
	static int64_t *get_ptr(Variant *v) { return VariantInternal::get_int(v); }
	static const int64_t *get_ptr(const Variant *v) { return VariantInternal::get_int(v); }
};

template <>
struct VariantGetInternalPtr<uint64_t> {
	static int64_t *get_ptr(Variant *v) { return VariantInternal::get_int(v); }
	static const int64_t *get_ptr(const Variant *v) { return VariantInternal::get_int(v); }
};

template <>
struct VariantGetInternalPtr<char32_t> {
	static int64_t *get_ptr(Variant *v) { return VariantInternal::get_int(v); }
	static const int64_t *get_ptr(const Variant *v) { return VariantInternal::get_int(v); }
};

template <>
struct VariantGetInternalPtr<ObjectID> {
	static int64_t *get_ptr(Variant *v) { return VariantInternal::get_int(v); }
	static const int64_t *get_ptr(const Variant *v) { return VariantInternal::get_int(v); }
};

template <>
struct VariantGetInternalPtr<Error> {
	static int64_t *get_ptr(Variant *v) { return VariantInternal::get_int(v); }
	static const int64_t *get_ptr(const Variant *v) { return VariantInternal::get_int(v); }
};

template <>
struct VariantGetInternalPtr<float> {
	static double *get_ptr(Variant *v) { return VariantInternal::get_float(v); }
	static const double *get_ptr(const Variant *v) { return VariantInternal::get_float(v); }
};

template <>
struct VariantGetInternalPtr<double> {
	static double *get_ptr(Variant *v) { return VariantInternal::get_float(v); }
	static const double *get_ptr(const Variant *v) { return VariantInternal::get_float(v); }
};

template <>
struct VariantGetInternalPtr<String> {
	static String *get_ptr(Variant *v) { return VariantInternal::get_string(v); }
	static const String *get_ptr(const Variant *v) { return VariantInternal::get_string(v); }
};

template <>
struct VariantGetInternalPtr<Vector2> {
	static Vector2 *get_ptr(Variant *v) { return VariantInternal::get_vector2(v); }
	static const Vector2 *get_ptr(const Variant *v) { return VariantInternal::get_vector2(v); }
};

template <>
struct VariantGetInternalPtr<Vector2i> {
	static Vector2i *get_ptr(Variant *v) { return VariantInternal::get_vector2i(v); }
	static const Vector2i *get_ptr(const Variant *v) { return VariantInternal::get_vector2i(v); }
};

template <>
struct VariantGetInternalPtr<Rect2> {
	static Rect2 *get_ptr(Variant *v) { return VariantInternal::get_rect2(v); }
	static const Rect2 *get_ptr(const Variant *v) { return VariantInternal::get_rect2(v); }
};

template <>
struct VariantGetInternalPtr<Rect2i> {
	static Rect2i *get_ptr(Variant *v) { return VariantInternal::get_rect2i(v); }
	static const Rect2i *get_ptr(const Variant *v) { return VariantInternal::get_rect2i(v); }
};

template <>
struct VariantGetInternalPtr<Vector3> {
	static Vector3 *get_ptr(Variant *v) { return VariantInternal::get_vector3(v); }
	static const Vector3 *get_ptr(const Variant *v) { return VariantInternal::get_vector3(v); }
};

template <>
struct VariantGetInternalPtr<Vector3i> {
	static Vector3i *get_ptr(Variant *v) { return VariantInternal::get_vector3i(v); }
	static const Vector3i *get_ptr(const Variant *v) { return VariantInternal::get_vector3i(v); }
};

template <>
struct VariantGetInternalPtr<Vector4> {
	static Vector4 *get_ptr(Variant *v) { return VariantInternal::get_vector4(v); }
	static const Vector4 *get_ptr(const Variant *v) { return VariantInternal::get_vector4(v); }
};

template <>
struct VariantGetInternalPtr<Vector4i> {
	static Vector4i *get_ptr(Variant *v) { return VariantInternal::get_vector4i(v); }
	static const Vector4i *get_ptr(const Variant *v) { return VariantInternal::get_vector4i(v); }
};
template <>
struct VariantGetInternalPtr<Transform2D> {
	static Transform2D *get_ptr(Variant *v) { return VariantInternal::get_transform2d(v); }
	static const Transform2D *get_ptr(const Variant *v) { return VariantInternal::get_transform2d(v); }
};

template <>
struct VariantGetInternalPtr<Transform3D> {
	static Transform3D *get_ptr(Variant *v) { return VariantInternal::get_transform(v); }
	static const Transform3D *get_ptr(const Variant *v) { return VariantInternal::get_transform(v); }
};

template <>
struct VariantGetInternalPtr<Projection> {
	static Projection *get_ptr(Variant *v) { return VariantInternal::get_projection(v); }
	static const Projection *get_ptr(const Variant *v) { return VariantInternal::get_projection(v); }
};

template <>
struct VariantGetInternalPtr<Plane> {
	static Plane *get_ptr(Variant *v) { return VariantInternal::get_plane(v); }
	static const Plane *get_ptr(const Variant *v) { return VariantInternal::get_plane(v); }
};

template <>
struct VariantGetInternalPtr<Quaternion> {
	static Quaternion *get_ptr(Variant *v) { return VariantInternal::get_quaternion(v); }
	static const Quaternion *get_ptr(const Variant *v) { return VariantInternal::get_quaternion(v); }
};

template <>
struct VariantGetInternalPtr<lain::AABB> {
	static lain::AABB *get_ptr(Variant *v) { return VariantInternal::get_aabb(v); }
	static const lain::AABB *get_ptr(const Variant *v) { return VariantInternal::get_aabb(v); }
};

template <>
struct VariantGetInternalPtr<Basis> {
	static Basis *get_ptr(Variant *v) { return VariantInternal::get_basis(v); }
	static const Basis *get_ptr(const Variant *v) { return VariantInternal::get_basis(v); }
};

//

template <>
struct VariantGetInternalPtr<Color> {
	static Color *get_ptr(Variant *v) { return VariantInternal::get_color(v); }
	static const Color *get_ptr(const Variant *v) { return VariantInternal::get_color(v); }
};

template <>
struct VariantGetInternalPtr<StringName> {
	static StringName *get_ptr(Variant *v) { return VariantInternal::get_string_name(v); }
	static const StringName *get_ptr(const Variant *v) { return VariantInternal::get_string_name(v); }
};

template <>
struct VariantGetInternalPtr<GObjectPath> {
	static GObjectPath *get_ptr(Variant *v) { return VariantInternal::get_node_path(v); }
	static const GObjectPath *get_ptr(const Variant *v) { return VariantInternal::get_node_path(v); }
};

template <>
struct VariantGetInternalPtr<lain::RID> {
	static lain::RID *get_ptr(Variant *v) { return VariantInternal::get_rid(v); }
	static const lain::RID *get_ptr(const Variant *v) { return VariantInternal::get_rid(v); }
};

template <>
struct VariantGetInternalPtr<Callable> {
	static Callable *get_ptr(Variant *v) { return VariantInternal::get_callable(v); }
	static const Callable *get_ptr(const Variant *v) { return VariantInternal::get_callable(v); }
};

template <>
struct VariantGetInternalPtr<Signal> {
	static Signal *get_ptr(Variant *v) { return VariantInternal::get_signal(v); }
	static const Signal *get_ptr(const Variant *v) { return VariantInternal::get_signal(v); }
};

template <>
struct VariantGetInternalPtr<Dictionary> {
	static Dictionary *get_ptr(Variant *v) { return VariantInternal::get_dictionary(v); }
	static const Dictionary *get_ptr(const Variant *v) { return VariantInternal::get_dictionary(v); }
};

template <>
struct VariantGetInternalPtr<Array> {
	static Array *get_ptr(Variant *v) { return VariantInternal::get_array(v); }
	static const Array *get_ptr(const Variant *v) { return VariantInternal::get_array(v); }
};

template <>
struct VariantGetInternalPtr<PackedByteArray> {
	static PackedByteArray *get_ptr(Variant *v) { return VariantInternal::get_byte_array(v); }
	static const PackedByteArray *get_ptr(const Variant *v) { return VariantInternal::get_byte_array(v); }
};

template <>
struct VariantGetInternalPtr<PackedInt32Array> {
	static PackedInt32Array *get_ptr(Variant *v) { return VariantInternal::get_int32_array(v); }
	static const PackedInt32Array *get_ptr(const Variant *v) { return VariantInternal::get_int32_array(v); }
};

template <>
struct VariantGetInternalPtr<PackedInt64Array> {
	static PackedInt64Array *get_ptr(Variant *v) { return VariantInternal::get_int64_array(v); }
	static const PackedInt64Array *get_ptr(const Variant *v) { return VariantInternal::get_int64_array(v); }
};

template <>
struct VariantGetInternalPtr<PackedFloat32Array> {
	static PackedFloat32Array *get_ptr(Variant *v) { return VariantInternal::get_float32_array(v); }
	static const PackedFloat32Array *get_ptr(const Variant *v) { return VariantInternal::get_float32_array(v); }
};

template <>
struct VariantGetInternalPtr<PackedFloat64Array> {
	static PackedFloat64Array *get_ptr(Variant *v) { return VariantInternal::get_float64_array(v); }
	static const PackedFloat64Array *get_ptr(const Variant *v) { return VariantInternal::get_float64_array(v); }
};

template <>
struct VariantGetInternalPtr<PackedStringArray> {
	static PackedStringArray *get_ptr(Variant *v) { return VariantInternal::get_string_array(v); }
	static const PackedStringArray *get_ptr(const Variant *v) { return VariantInternal::get_string_array(v); }
};

template <>
struct VariantGetInternalPtr<PackedVector2Array> {
	static PackedVector2Array *get_ptr(Variant *v) { return VariantInternal::get_vector2_array(v); }
	static const PackedVector2Array *get_ptr(const Variant *v) { return VariantInternal::get_vector2_array(v); }
};

template <>
struct VariantGetInternalPtr<PackedVector3Array> {
	static PackedVector3Array *get_ptr(Variant *v) { return VariantInternal::get_vector3_array(v); }
	static const PackedVector3Array *get_ptr(const Variant *v) { return VariantInternal::get_vector3_array(v); }
};

template <>
struct VariantGetInternalPtr<PackedColorArray> {
	static PackedColorArray *get_ptr(Variant *v) { return VariantInternal::get_color_array(v); }
	static const PackedColorArray *get_ptr(const Variant *v) { return VariantInternal::get_color_array(v); }
};

template <>
struct VariantGetInternalPtr<PackedVector4Array> {
	static PackedVector4Array *get_ptr(Variant *v) { return VariantInternal::get_vector4_array(v); }
	static const PackedVector4Array *get_ptr(const Variant *v) { return VariantInternal::get_vector4_array(v); }
};

// get set 方法
template <typename T>
struct VariantInternalAccessor {
};

template <>
struct VariantInternalAccessor<bool> {
	static _FORCE_INLINE_ bool get(const Variant *v) { return *VariantInternal::get_bool(v); }
	static _FORCE_INLINE_ void set(Variant *v, bool p_value) { *VariantInternal::get_bool(v) = p_value; }
};

#define VARIANT_ACCESSOR_NUMBER(m_type)                                                                        \
	template <>                                                                                                \
	struct VariantInternalAccessor<m_type> {                                                                   \
		static _FORCE_INLINE_ m_type get(const Variant *v) { return (m_type) * VariantInternal::get_int(v); }  \
		static _FORCE_INLINE_ void set(Variant *v, m_type p_value) { *VariantInternal::get_int(v) = p_value; } \
	};

VARIANT_ACCESSOR_NUMBER(int8_t)
VARIANT_ACCESSOR_NUMBER(uint8_t)
VARIANT_ACCESSOR_NUMBER(int16_t)
VARIANT_ACCESSOR_NUMBER(uint16_t)
VARIANT_ACCESSOR_NUMBER(int32_t)
VARIANT_ACCESSOR_NUMBER(uint32_t)
VARIANT_ACCESSOR_NUMBER(int64_t)
VARIANT_ACCESSOR_NUMBER(uint64_t)
VARIANT_ACCESSOR_NUMBER(char32_t)

template <>
struct VariantInternalAccessor<ObjectID> {
	static _FORCE_INLINE_ ObjectID get(const Variant *v) { return ObjectID(*VariantInternal::get_int(v)); }
	static _FORCE_INLINE_ void set(Variant *v, ObjectID p_value) { *VariantInternal::get_int(v) = p_value; }
};

template <typename T>
struct VariantInternalAccessor<T *> {
	static _FORCE_INLINE_ T *get(const Variant *v) { return const_cast<T *>(static_cast<const T *>(*VariantInternal::get_object(v))); }
	static _FORCE_INLINE_ void set(Variant *v, const T *p_value) { VariantInternal::object_assign(v, p_value); }
};

template <typename T>
struct VariantInternalAccessor<const T *> {
	static _FORCE_INLINE_ const T *get(const Variant *v) { return static_cast<const T *>(*VariantInternal::get_object(v)); }
	static _FORCE_INLINE_ void set(Variant *v, const T *p_value) { VariantInternal::object_assign(v, p_value); }
};

// template <>
// struct VariantInternalAccessor<IPAddress> {
// 	static _FORCE_INLINE_ IPAddress get(const Variant *v) { return IPAddress(*VariantInternal::get_string(v)); }
// 	static _FORCE_INLINE_ void set(Variant *v, IPAddress p_value) { *VariantInternal::get_string(v) = p_value; }
// };

template <>
struct VariantInternalAccessor<float> {
	static _FORCE_INLINE_ float get(const Variant *v) { return *VariantInternal::get_float(v); }
	static _FORCE_INLINE_ void set(Variant *v, float p_value) { *VariantInternal::get_float(v) = p_value; }
};

template <>
struct VariantInternalAccessor<double> {
	static _FORCE_INLINE_ double get(const Variant *v) { return *VariantInternal::get_float(v); }
	static _FORCE_INLINE_ void set(Variant *v, double p_value) { *VariantInternal::get_float(v) = p_value; }
};

template <>
struct VariantInternalAccessor<String> {
	static _FORCE_INLINE_ const String &get(const Variant *v) { return *VariantInternal::get_string(v); }
	static _FORCE_INLINE_ void set(Variant *v, const String &p_value) { *VariantInternal::get_string(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Vector2> {
	static _FORCE_INLINE_ const Vector2 &get(const Variant *v) { return *VariantInternal::get_vector2(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Vector2 &p_value) { *VariantInternal::get_vector2(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Vector2i> {
	static _FORCE_INLINE_ const Vector2i &get(const Variant *v) { return *VariantInternal::get_vector2i(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Vector2i &p_value) { *VariantInternal::get_vector2i(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Rect2> {
	static _FORCE_INLINE_ const Rect2 &get(const Variant *v) { return *VariantInternal::get_rect2(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Rect2 &p_value) { *VariantInternal::get_rect2(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Rect2i> {
	static _FORCE_INLINE_ const Rect2i &get(const Variant *v) { return *VariantInternal::get_rect2i(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Rect2i &p_value) { *VariantInternal::get_rect2i(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Vector3> {
	static _FORCE_INLINE_ const Vector3 &get(const Variant *v) { return *VariantInternal::get_vector3(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Vector3 &p_value) { *VariantInternal::get_vector3(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Vector3i> {
	static _FORCE_INLINE_ const Vector3i &get(const Variant *v) { return *VariantInternal::get_vector3i(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Vector3i &p_value) { *VariantInternal::get_vector3i(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Vector4> {
	static _FORCE_INLINE_ const Vector4 &get(const Variant *v) { return *VariantInternal::get_vector4(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Vector4 &p_value) { *VariantInternal::get_vector4(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Vector4i> {
	static _FORCE_INLINE_ const Vector4i &get(const Variant *v) { return *VariantInternal::get_vector4i(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Vector4i &p_value) { *VariantInternal::get_vector4i(v) = p_value; }
};
template <>
struct VariantInternalAccessor<Transform2D> {
	static _FORCE_INLINE_ const Transform2D &get(const Variant *v) { return *VariantInternal::get_transform2d(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Transform2D &p_value) { *VariantInternal::get_transform2d(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Transform3D> {
	static _FORCE_INLINE_ const Transform3D &get(const Variant *v) { return *VariantInternal::get_transform(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Transform3D &p_value) { *VariantInternal::get_transform(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Projection> {
	static _FORCE_INLINE_ const Projection &get(const Variant *v) { return *VariantInternal::get_projection(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Projection &p_value) { *VariantInternal::get_projection(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Plane> {
	static _FORCE_INLINE_ const Plane &get(const Variant *v) { return *VariantInternal::get_plane(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Plane &p_value) { *VariantInternal::get_plane(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Quaternion> {
	static _FORCE_INLINE_ const Quaternion &get(const Variant *v) { return *VariantInternal::get_quaternion(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Quaternion &p_value) { *VariantInternal::get_quaternion(v) = p_value; }
};

template <>
struct VariantInternalAccessor<AABB> {
	static _FORCE_INLINE_ const AABB &get(const Variant *v) { return *VariantInternal::get_aabb(v); }
	static _FORCE_INLINE_ void set(Variant *v, const AABB &p_value) { *VariantInternal::get_aabb(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Basis> {
	static _FORCE_INLINE_ const Basis &get(const Variant *v) { return *VariantInternal::get_basis(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Basis &p_value) { *VariantInternal::get_basis(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Color> {
	static _FORCE_INLINE_ const Color &get(const Variant *v) { return *VariantInternal::get_color(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Color &p_value) { *VariantInternal::get_color(v) = p_value; }
};

template <>
struct VariantInternalAccessor<StringName> {
	static _FORCE_INLINE_ const StringName &get(const Variant *v) { return *VariantInternal::get_string_name(v); }
	static _FORCE_INLINE_ void set(Variant *v, const StringName &p_value) { *VariantInternal::get_string_name(v) = p_value; }
};

template <>
struct VariantInternalAccessor<GObjectPath> {
	static _FORCE_INLINE_ const GObjectPath &get(const Variant *v) { return *VariantInternal::get_node_path(v); }
	static _FORCE_INLINE_ void set(Variant *v, const GObjectPath &p_value) { *VariantInternal::get_node_path(v) = p_value; }
};

template <>
struct VariantInternalAccessor<lain::RID> {
	static _FORCE_INLINE_ const lain::RID &get(const Variant *v) { return *VariantInternal::get_rid(v); }
	static _FORCE_INLINE_ void set(Variant *v, const lain::RID &p_value) { *VariantInternal::get_rid(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Callable> {
	static _FORCE_INLINE_ const Callable &get(const Variant *v) { return *VariantInternal::get_callable(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Callable &p_value) { *VariantInternal::get_callable(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Signal> {
	static _FORCE_INLINE_ const Signal &get(const Variant *v) { return *VariantInternal::get_signal(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Signal &p_value) { *VariantInternal::get_signal(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Dictionary> {
	static _FORCE_INLINE_ const Dictionary &get(const Variant *v) { return *VariantInternal::get_dictionary(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Dictionary &p_value) { *VariantInternal::get_dictionary(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Array> {
	static _FORCE_INLINE_ const Array &get(const Variant *v) { return *VariantInternal::get_array(v); }
	static _FORCE_INLINE_ void set(Variant *v, const Array &p_value) { *VariantInternal::get_array(v) = p_value; }
};

template <>
struct VariantInternalAccessor<PackedByteArray> {
	static _FORCE_INLINE_ const PackedByteArray &get(const Variant *v) { return *VariantInternal::get_byte_array(v); }
	static _FORCE_INLINE_ void set(Variant *v, const PackedByteArray &p_value) { *VariantInternal::get_byte_array(v) = p_value; }
};

template <>
struct VariantInternalAccessor<PackedInt32Array> {
	static _FORCE_INLINE_ const PackedInt32Array &get(const Variant *v) { return *VariantInternal::get_int32_array(v); }
	static _FORCE_INLINE_ void set(Variant *v, const PackedInt32Array &p_value) { *VariantInternal::get_int32_array(v) = p_value; }
};

template <>
struct VariantInternalAccessor<PackedInt64Array> {
	static _FORCE_INLINE_ const PackedInt64Array &get(const Variant *v) { return *VariantInternal::get_int64_array(v); }
	static _FORCE_INLINE_ void set(Variant *v, const PackedInt64Array &p_value) { *VariantInternal::get_int64_array(v) = p_value; }
};

template <>
struct VariantInternalAccessor<PackedFloat32Array> {
	static _FORCE_INLINE_ const PackedFloat32Array &get(const Variant *v) { return *VariantInternal::get_float32_array(v); }
	static _FORCE_INLINE_ void set(Variant *v, const PackedFloat32Array &p_value) { *VariantInternal::get_float32_array(v) = p_value; }
};

template <>
struct VariantInternalAccessor<PackedFloat64Array> {
	static _FORCE_INLINE_ const PackedFloat64Array &get(const Variant *v) { return *VariantInternal::get_float64_array(v); }
	static _FORCE_INLINE_ void set(Variant *v, const PackedFloat64Array &p_value) { *VariantInternal::get_float64_array(v) = p_value; }
};

template <>
struct VariantInternalAccessor<PackedStringArray> {
	static _FORCE_INLINE_ const PackedStringArray &get(const Variant *v) { return *VariantInternal::get_string_array(v); }
	static _FORCE_INLINE_ void set(Variant *v, const PackedStringArray &p_value) { *VariantInternal::get_string_array(v) = p_value; }
};

template <>
struct VariantInternalAccessor<PackedVector2Array> {
	static _FORCE_INLINE_ const PackedVector2Array &get(const Variant *v) { return *VariantInternal::get_vector2_array(v); }
	static _FORCE_INLINE_ void set(Variant *v, const PackedVector2Array &p_value) { *VariantInternal::get_vector2_array(v) = p_value; }
};

template <>
struct VariantInternalAccessor<PackedVector3Array> {
	static _FORCE_INLINE_ const PackedVector3Array &get(const Variant *v) { return *VariantInternal::get_vector3_array(v); }
	static _FORCE_INLINE_ void set(Variant *v, const PackedVector3Array &p_value) { *VariantInternal::get_vector3_array(v) = p_value; }
};

template <>
struct VariantInternalAccessor<PackedColorArray> {
	static _FORCE_INLINE_ const PackedColorArray &get(const Variant *v) { return *VariantInternal::get_color_array(v); }
	static _FORCE_INLINE_ void set(Variant *v, const PackedColorArray &p_value) { *VariantInternal::get_color_array(v) = p_value; }
};

template <>
struct VariantInternalAccessor<PackedVector4Array> {
	static _FORCE_INLINE_ const PackedVector4Array &get(const Variant *v) { return *VariantInternal::get_vector4_array(v); }
	static _FORCE_INLINE_ void set(Variant *v, const PackedVector4Array &p_value) { *VariantInternal::get_vector4_array(v) = p_value; }
};

template <>
struct VariantInternalAccessor<Object *> {
	static _FORCE_INLINE_ Object *get(const Variant *v) { return const_cast<Object *>(*VariantInternal::get_object(v)); }
	static _FORCE_INLINE_ void set(Variant *v, const Object *p_value) { VariantInternal::object_assign(v, p_value); }
};

template <>
struct VariantInternalAccessor<Variant> {
	static _FORCE_INLINE_ Variant &get(Variant *v) { return *v; }
	static _FORCE_INLINE_ const Variant &get(const Variant *v) { return *v; }
	static _FORCE_INLINE_ void set(Variant *v, const Variant &p_value) { *v = p_value; }
};

template <>
struct VariantInternalAccessor<Vector<Variant>> {
	static _FORCE_INLINE_ Vector<Variant> get(const Variant *v) {
		Vector<Variant> ret;
		int s = VariantInternal::get_array(v)->size();
		ret.resize(s);
		for (int i = 0; i < s; i++) {
			ret.write[i] = VariantInternal::get_array(v)->get(i);
		}

		return ret;
	}
	static _FORCE_INLINE_ void set(Variant *v, const Vector<Variant> &p_value) {
		int s = p_value.size();
		VariantInternal::get_array(v)->resize(s);
		for (int i = 0; i < s; i++) {
			VariantInternal::get_array(v)->set(i, p_value[i]);
		}
	}
};


}

#endif // !VARIANT_INTERNAL_H
