#include "variant_helper.h"
#include "core/object/object.h"
namespace lain {
	Vector<const char*> VariantHelper::variant_basic_reflect_types{
	"String",
	"int",
	"float",
	"bool",
	"Array"
	};
	bool VariantHelper::is_serializable_type(const char* type_name) {
		bool is_basic = variant_basic_reflect_types.has(type_name);
		if (is_basic) return true;
		bool serializable = Reflection::TypeMeta::is_valid_type(type_name);
		return serializable;
	}

	bool VariantHelper::is_serializable(const Variant& p_var){
		const char* type_name = Variant::get_c_type_name(p_var.get_type());
		if (p_var.get_type() == Variant::OBJECT) {
			type_name = p_var.operator Object * ()->get_c_class();
		}
		bool is_basic = variant_basic_reflect_types.has(type_name);
		if (is_basic) return true;
		bool serializable = Reflection::TypeMeta::is_valid_type(type_name);
		return serializable;
		// 有反射或者是基本类型
	}

	Variant::Type VariantHelper::get_type_from_name(const char* p_name_c_str) {
		String p_name = p_name_c_str;
				if(p_name == "Nil"){ return Variant::NIL; }

				if(p_name == "bool"){ return Variant::BOOL; }
		if(p_name == "int"){ return Variant::INT; }

		if(p_name == "float"){ return Variant::FLOAT; }

		if(p_name == "String"){ return Variant::STRING; }


	// Math types.
		if(p_name == "Vector2"){ return Variant::VECTOR2; }

		if(p_name == "Vector2i"){ return Variant::VECTOR2I; }

		if(p_name == "Rect2"){ return Variant::RECT2; }

		if(p_name == "Rect2i"){ return Variant::RECT2I; }

		if(p_name == "Transform2D"){ return Variant::TRANSFORM2D; }

		if(p_name == "Vector3"){ return Variant::VECTOR3; }

		if(p_name == "Vector3i"){ return Variant::VECTOR3I; }

		if(p_name == "Vector4"){ return Variant::VECTOR4; }

		if(p_name == "Vector4i"){ return Variant::VECTOR4I; }

		if(p_name == "Plane"){ return Variant::PLANE; }

		if(p_name == "AABB"){ return Variant::AABB; }

		if(p_name == "Quaternion"){ return Variant::QUATERNION; }

		if(p_name == "Basis"){ return Variant::BASIS; }

		if(p_name == "Transform3D"){ return Variant::TRANSFORM3D; }

		if(p_name == "Projection"){ return Variant::PROJECTION; }


		// Miscellaneous types.
		if(p_name == "Color"){ return Variant::COLOR; }

		if(p_name == "RID"){ return Variant::RID; }

		if(p_name == "Object"){ return Variant::OBJECT; }

		if(p_name == "Callable"){ return Variant::CALLABLE; }

		if(p_name == "Signal"){ return Variant::SIGNAL; }

		if(p_name == "StringName"){ return Variant::STRING_NAME; }

		if(p_name == "GObjectPath"){ return Variant::GOBJECT_PATH; }

		if(p_name == "Dictionary"){ return Variant::DICTIONARY; }

		if(p_name == "Array"){ return Variant::ARRAY; }


	// Arrays.
		if(p_name == "PackedByteArray"){ return Variant::PACKED_BYTE_ARRAY; }

		if(p_name == "PackedInt32Array"){ return Variant::PACKED_INT32_ARRAY; }

		if(p_name == "PackedInt64Array"){ return Variant::PACKED_INT64_ARRAY; }

		if(p_name == "PackedFloat32Array"){ return Variant::PACKED_FLOAT32_ARRAY; }

		if(p_name == "PackedFloat64Array"){ return Variant::PACKED_FLOAT64_ARRAY; }

		if(p_name == "PackedStringArray"){ return Variant::PACKED_STRING_ARRAY; }

		if(p_name == "PackedVector2Array"){ return Variant::PACKED_VECTOR2_ARRAY; }

		if(p_name == "PackedVector3Array"){ return Variant::PACKED_VECTOR3_ARRAY; }

		if(p_name == "PackedColorArray"){ return Variant::PACKED_COLOR_ARRAY; }

		if (p_name == "ReflectionInstance") { return Variant::REFLECTIONINSTANCE; }

		return Variant::VARIANT_MAX;
	}

	static const bool needs_deinit[Variant::VARIANT_MAX] = {
			false, //NIL,
			false, //BOOL,
			false, //INT,
			false, //FLOAT,
			true, //STRING,
			false, //VECTOR2,
			false, //VECTOR2I,
			false, //RECT2,
			false, //RECT2I,
			false, //VECTOR3,
			false, //VECTOR3I,
			true, //TRANSFORM2D,
			false, //VECTOR4,
			false, //VECTOR4I,
			false, //PLANE,
			false, //QUATERNION,
			true, //AABB,
			true, //BASIS,
			true, //TRANSFORM,
			true, //PROJECTION,

			// misc types
			false, //COLOR,
			true, //STRING_NAME,
			true, //NODE_PATH,
			false, //RID,
			true, //OBJECT,
			true, //CALLABLE,
			true, //SIGNAL,
			true, //DICTIONARY,
			true, //ARRAY,

			// typed arrays
			true, //PACKED_BYTE_ARRAY,
			true, //PACKED_INT32_ARRAY,
			true, //PACKED_INT64_ARRAY,
			true, //PACKED_FLOAT32_ARRAY,
			true, //PACKED_FLOAT64_ARRAY,
			true, //PACKED_STRING_ARRAY,
			true, //PACKED_VECTOR2_ARRAY,
			true, //PACKED_VECTOR3_ARRAY,
			true, //PACKED_COLOR_ARRAY,
			true, // reflectionInstance
	};

}