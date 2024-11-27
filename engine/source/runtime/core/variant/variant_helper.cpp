#include "variant_helper.h"
#include "core/object/object.h"
#include "variant_internal.h"
namespace lain {
Vector<String> VariantHelper::variant_basic_reflect_types{"String", "int", "float", "bool", "Array"};
bool VariantHelper::is_serializable_type(const char* type_name) {
  return is_serializable_type(String(type_name));
}
bool VariantHelper::is_serializable_type(const String& type_name) {
  bool is_basic = variant_basic_reflect_types.has(type_name);
  if (is_basic)
    return true;
  bool serializable = Reflection::TypeMeta::is_valid_type(type_name);
  return serializable;
}
// 不太对，没有考虑 vector<>
bool VariantHelper::is_serializable(const Variant& p_var) {
  const char* type_name = Variant::get_c_type_name(p_var.get_type());
  if (p_var.get_type() == Variant::OBJECT) {
    type_name = p_var.operator Object*()->get_c_class();
  }
  bool is_basic = variant_basic_reflect_types.has(type_name);
  if (is_basic)
    return true;
  bool serializable = Reflection::TypeMeta::is_valid_type(type_name);
  return serializable;
  // 有反射或者是基本类型
}
Variant::Type VariantHelper::get_type_from_name(const String& p_name) {
  String name = p_name.to_lower();

  if (name == "nil" || name == "null") {
    return Variant::NIL;
  }

  if (name == "bool") {
    return Variant::BOOL;
  }
  if (name == "int") {
    return Variant::INT;
  }

  if (name == "float") {
    return Variant::FLOAT;
  }

  if (name == "string") {
    return Variant::STRING;
  }

  // Math types.
  if (name == "vector2") {
    return Variant::VECTOR2;
  }

  if (name == "vector2i") {
    return Variant::VECTOR2I;
  }

  if (name == "rect2") {
    return Variant::RECT2;
  }

  if (name == "rect2i") {
    return Variant::RECT2I;
  }

  if (name == "transform2D") {
    return Variant::TRANSFORM2D;
  }

  if (name == "vector3") {
    return Variant::VECTOR3;
  }

  if (name == "vector3i") {
    return Variant::VECTOR3I;
  }

  if (name == "vector4") {
    return Variant::VECTOR4;
  }

  if (name == "vector4i") {
    return Variant::VECTOR4I;
  }

  if (name == "plane") {
    return Variant::PLANE;
  }

  if (name == "aabb") {
    return Variant::AABB;
  }

  if (name == "quaternion") {
    return Variant::QUATERNION;
  }

  if (name == "basis") {
    return Variant::BASIS;
  }

  if (name == "transform3d") {
    return Variant::TRANSFORM3D;
  }

  if (name == "projection") {
    return Variant::PROJECTION;
  }

  // Miscellaneous types.
  if (name == "color") {
    return Variant::COLOR;
  }

  if (name == "rid") {
    return Variant::RID;
  }

  if (name == "object") {
    return Variant::OBJECT;
  }

  if (name == "callable") {
    return Variant::CALLABLE;
  }

  if (name == "signal") {
    return Variant::SIGNAL;
  }

  if (name == "stringname") {
    return Variant::STRING_NAME;
  }

  if (name == "gobjectpath") {
    return Variant::GOBJECT_PATH;
  }

  if (name == "dictionary") {
    return Variant::DICTIONARY;
  }

  if (name == "array") {
    return Variant::ARRAY;
  }

  // Arrays.
  if (name == "packedbytearray" || p_name == "vector<uint8_t>") {
    return Variant::PACKED_BYTE_ARRAY;
  }

  if (name == "PackedInt32Array" || p_name == "vector<int32_t>") {
    return Variant::PACKED_INT32_ARRAY;
  }

  if (name == "PackedInt64Array" || p_name == "vector<int64_t>") {
    return Variant::PACKED_INT64_ARRAY;
  }

  if (name == "PackedFloat32Array" || p_name == "vector<float>") {
    return Variant::PACKED_FLOAT32_ARRAY;
  }

  if (name == "PackedFloat64Array" || p_name == "vector<double>") {
    return Variant::PACKED_FLOAT64_ARRAY;
  }

  if (name == "PackedStringArray" || p_name == "vector<string>") {
    return Variant::PACKED_STRING_ARRAY;
  }

  if (name == "PackedVector2Array" || p_name == "vector<vector2>") {
    return Variant::PACKED_VECTOR2_ARRAY;
  }

  if (name == "PackedVector3Array" || p_name == "vector<vector4>") {
    return Variant::PACKED_VECTOR3_ARRAY;
  }

  if (name == "PackedColorArray" || p_name == "vector<color>") {
    return Variant::PACKED_COLOR_ARRAY;
  }

  if (name == "ReflectionInstance") {
    return Variant::REFLECTIONINSTANCE;
  }
  // 然后在这里查是否继承了object，如果是的话就是object

  // 否则就报错，但是我们特地使用只有这些类型的反射，所以不会出问题（应该) @todo @bug?
  return Variant::OBJECT;
}
Variant::Type VariantHelper::get_type_from_name(const char* p_name_c_str) {
  String p_name = p_name_c_str;
  return get_type_from_name(p_name);
}

void VariantHelper::variant_from_data(const Variant::Type p_type, const void* p_data, Variant& r_variant) {
	if (r_variant.get_type() != p_type && r_variant.get_type() != Variant::NIL) {
		r_variant.clear();
	}
	r_variant.set_type(p_type);
	switch(p_type){
		case Variant::NIL:
			break;
		case Variant::BOOL:
			r_variant = *(bool*)p_data;
			break;
		case Variant::INT:
			r_variant = *(int64_t*)p_data;
			break;
		case Variant::FLOAT:
			r_variant = *(float*)p_data;
			break;
		case Variant::STRING:
			r_variant = *(String*)p_data;
			break;
		case Variant::VECTOR2:
			r_variant = *(Vector2*)p_data;
			break;
		case Variant::VECTOR2I:
			r_variant = *(Vector2i*)p_data;
			break;
		case Variant::RECT2:
			r_variant = *(Rect2*)p_data;
			break;
		case Variant::RECT2I:
			r_variant = *(Rect2i*)p_data;
			break;
		case Variant::VECTOR3:
			r_variant = *(Vector3*)p_data;
			break;
		case Variant::VECTOR3I:
			r_variant = *(Vector3i*)p_data;
			break;
		case Variant::TRANSFORM2D:
			r_variant = *(Transform2D*)p_data;
			break;
		case Variant::TRANSFORM3D:
			r_variant = *(Transform3D*)p_data;
			break;
		case Variant::VECTOR4:
			r_variant = *(Vector4*)p_data;
			break;
		case Variant::VECTOR4I:
			r_variant = *(Vector4i*)p_data;
			break;
		case Variant::PLANE:
			r_variant = *(Plane*)p_data;
			break;
		case Variant::QUATERNION:
			r_variant = *(Quaternion*)p_data;
			break;
		case Variant::AABB:
			r_variant = *(AABB*)p_data;
			break;
		case Variant::BASIS:
			r_variant = *(Basis*)p_data;
			break;

		case Variant::PROJECTION:
			r_variant = *(Projection*)p_data;
			break;
		case Variant::COLOR:
			r_variant = *(Color*)p_data;
			break;
		case Variant::STRING_NAME:
			r_variant = *(StringName*)p_data;
			break;
		case Variant::GOBJECT_PATH:
			r_variant = *(GObjectPath*)p_data;
			break;
		case Variant::RID:
			r_variant = *(RID*)p_data;
			break;
		case Variant::OBJECT:
			r_variant = (Object *)p_data;
			break;
		case Variant::CALLABLE:
			r_variant = *(Callable*)p_data;
			break;
		case Variant::SIGNAL:
			r_variant = *(Signal*)p_data;
			break;
		case Variant::DICTIONARY:
			r_variant = *(Dictionary*)p_data;
			break;
		case Variant::ARRAY:
			r_variant = *(Array*)p_data;
			break;
		case Variant::PACKED_BYTE_ARRAY:
			r_variant = *(PackedByteArray*)p_data;
			break;
		case Variant::PACKED_INT32_ARRAY:
			r_variant = *(PackedInt32Array*)p_data;
			break;
		case Variant::PACKED_INT64_ARRAY:
			r_variant = *(PackedInt64Array*)p_data;
			break;
		case Variant::PACKED_FLOAT32_ARRAY:
			r_variant = *(PackedFloat32Array*)p_data;
			break;
		case Variant::PACKED_FLOAT64_ARRAY:
			r_variant = *(PackedFloat64Array*)p_data;
			break;
		case Variant::PACKED_STRING_ARRAY:
			r_variant = *(PackedStringArray*)p_data;
			break;
		case Variant::PACKED_VECTOR2_ARRAY:
			r_variant = *(PackedVector2Array*)p_data;
			break;
		case Variant::PACKED_VECTOR3_ARRAY:
			r_variant = *(PackedVector3Array*)p_data;
			break;
		case Variant::PACKED_COLOR_ARRAY:
			r_variant = *(PackedColorArray*)p_data;
			break;
		case Variant::REFLECTIONINSTANCE:
			break;
		default:
			break;
	}

}

void VariantHelper::object_set_data(void* p_ptr, const Variant& r_variant) {
  switch(r_variant.get_type()){
    case Variant::NIL:
      break;
    case Variant::BOOL:
      *(bool*)p_ptr = r_variant._data._bool;
      break;
    case Variant::INT:
      *(int*)p_ptr = r_variant._data._int;
      break;
    case Variant::FLOAT:
      *(float*)p_ptr = r_variant._data._float;
      break;
    case Variant::STRING:
      *(String*)p_ptr = *reinterpret_cast<const String *>(r_variant._data._mem);
    case Variant::VECTOR2:
      *(Vector2*)p_ptr = *reinterpret_cast<const Vector2 *>(r_variant._data._mem);
      break;
    case Variant::VECTOR2I:
      *(Vector2i*)p_ptr = *reinterpret_cast<const Vector2i *>(r_variant._data._mem);
      break;
    case Variant::RECT2:
      *(Rect2*)p_ptr = *reinterpret_cast<const Rect2 *>(r_variant._data._mem);
      break;
    case Variant::RECT2I:
      *(Rect2i*)p_ptr = *reinterpret_cast<const Rect2i *>(r_variant._data._mem);
      break;
    case Variant::VECTOR3:
      *(Vector3*)p_ptr = *reinterpret_cast<const Vector3 *>(r_variant._data._mem);
      break;
    case Variant::VECTOR3I:
      *(Vector3i*)p_ptr = *reinterpret_cast<const Vector3i *>(r_variant._data._mem);
      break;
    case Variant::TRANSFORM2D:
      *(Transform2D*)p_ptr = *reinterpret_cast<const Transform2D *>(r_variant._data._mem);
      break;
    case Variant::TRANSFORM3D:
      *(Transform3D*)p_ptr = *reinterpret_cast<const Transform3D *>(r_variant._data._mem);
      break; 
    case Variant::VECTOR4:
      *(Vector4*)p_ptr = *reinterpret_cast<const Vector4 *>(r_variant._data._mem);
      break;
    case Variant::VECTOR4I:
      *(Vector4i*)p_ptr = *reinterpret_cast<const Vector4i *>(r_variant._data._mem);
      break;
    case Variant::PLANE:
      *(Plane*)p_ptr = *reinterpret_cast<const Plane *>(r_variant._data._mem);
      break;
    case Variant::QUATERNION:
      *(Quaternion*)p_ptr = *reinterpret_cast<const Quaternion *>(r_variant._data._mem);
      break;
    case Variant::AABB:
      *(AABB*)p_ptr = *reinterpret_cast<const AABB *>(r_variant._data._mem);
      break;  
    case Variant::BASIS:
      *(Basis*)p_ptr = *reinterpret_cast<const Basis *>(r_variant._data._mem);
      break;
    case Variant::PROJECTION:
      *(Projection*)p_ptr = *reinterpret_cast<const Projection *>(r_variant._data._mem);
      break;
    case Variant::COLOR:
      *(Color*)p_ptr = *reinterpret_cast<const Color *>(r_variant._data._mem);
      break;
    case Variant::STRING_NAME:
      *(StringName*)p_ptr = *reinterpret_cast<const StringName *>(r_variant._data._mem);
      break;
    case Variant::GOBJECT_PATH:
      *(GObjectPath*)p_ptr = *reinterpret_cast<const GObjectPath *>(r_variant._data._mem);
      break;
    case Variant::RID:
      *(RID*)p_ptr = *reinterpret_cast<const RID *>(r_variant._data._mem);
      break;  
    case Variant::OBJECT: // 这种情况似乎不会出现
     {
       L_CORE_WARN("assign object with another object directly?");
        Object* obj = r_variant.operator Object*();
        *(Object*)p_ptr = *obj;
      break;
     }
     case Variant::CALLABLE:
      *(Callable*)p_ptr = *reinterpret_cast<const Callable *>(r_variant._data._mem);
      break;
    case Variant::SIGNAL:
      *(Signal*)p_ptr = *reinterpret_cast<const Signal *>(r_variant._data._mem);
      break; 
    case Variant::DICTIONARY:
      *(Dictionary*)p_ptr = *reinterpret_cast<const Dictionary *>(r_variant._data._mem);
      break;
    case Variant::ARRAY:
      *(Array*)p_ptr = *reinterpret_cast<const Array *>(r_variant._data._mem);
      break;
    case Variant::PACKED_BYTE_ARRAY:
      *(PackedByteArray*)p_ptr = static_cast<Variant::PackedArrayRef<uint8_t> *>(r_variant._data.packed_array)->array;
      break;
    case Variant::PACKED_INT32_ARRAY:
      *(PackedInt32Array*)p_ptr = static_cast<Variant::PackedArrayRef<int32_t> *>(r_variant._data.packed_array)->array;
      break;
    case Variant::PACKED_INT64_ARRAY:
      *(PackedInt64Array*)p_ptr = static_cast<Variant::PackedArrayRef<int64_t> *>(r_variant._data.packed_array)->array;
      break;
    case Variant::PACKED_FLOAT32_ARRAY:
      *(PackedFloat32Array*)p_ptr = static_cast<Variant::PackedArrayRef<float> *>(r_variant._data.packed_array)->array;
      break;
    case Variant::PACKED_FLOAT64_ARRAY:
      *(PackedFloat64Array*)p_ptr = static_cast<Variant::PackedArrayRef<double> *>(r_variant._data.packed_array)->array;
      break;
    case Variant::PACKED_STRING_ARRAY:
      *(PackedStringArray*)p_ptr = static_cast<Variant::PackedArrayRef<String> *>(r_variant._data.packed_array)->array;
      break;
    case Variant::PACKED_VECTOR2_ARRAY:
      *(PackedVector2Array*)p_ptr = static_cast<Variant::PackedArrayRef<Vector2> *>(r_variant._data.packed_array)->array;
      break;
    case Variant::PACKED_VECTOR3_ARRAY:
      *(PackedVector3Array*)p_ptr = static_cast<Variant::PackedArrayRef<Vector3> *>(r_variant._data.packed_array)->array;
      break;
    case Variant::PACKED_COLOR_ARRAY:
      *(PackedColorArray*)p_ptr = static_cast<Variant::PackedArrayRef<Color> *>(r_variant._data.packed_array)->array;
      break;
    case Variant::REFLECTIONINSTANCE: 
      break;
    default:
      break;      

  }
}

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
    true,   //NODE_PATH,
    false,  //RID,
    true,   //OBJECT,
    true,   //CALLABLE,
    true,   //SIGNAL,
    true,   //DICTIONARY,
    true,   //ARRAY,

    // typed arrays
    true,  //PACKED_BYTE_ARRAY,
    true,  //PACKED_INT32_ARRAY,
    true,  //PACKED_INT64_ARRAY,
    true,  //PACKED_FLOAT32_ARRAY,
    true,  //PACKED_FLOAT64_ARRAY,
    true,  //PACKED_STRING_ARRAY,
    true,  //PACKED_VECTOR2_ARRAY,
    true,  //PACKED_VECTOR3_ARRAY,
    true,  //PACKED_COLOR_ARRAY,
    true,  // reflectionInstance
};

}  // namespace lain