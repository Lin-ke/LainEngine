#include "variant.h"
#include "core/os/memory.h"
namespace lain {
	String Variant::get_type_name(Variant::Type p_type) {
		switch (p_type) {
		case NIL: {
			return "Nil";
		}

				// Atomic types.
		case BOOL: {
			return "bool";
		}
		case INT: {
			return "int";
		}
		case FLOAT: {
			return "float";
		}
		case STRING: {
			return "String";
		}

				   // Math types.
		case VECTOR2: {
			return "Vector2";
		}
		case VECTOR2I: {
			return "Vector2i";
		}
		case RECT2: {
			return "Rect2";
		}
		case RECT2I: {
			return "Rect2i";
		}
		case TRANSFORM2D: {
			return "Transform2D";
		}
		case VECTOR3: {
			return "Vector3";
		}
		case VECTOR3I: {
			return "Vector3i";
		}
		case VECTOR4: {
			return "Vector4";
		}
		case VECTOR4I: {
			return "Vector4i";
		}
		case PLANE: {
			return "Plane";
		}
		case AABB: {
			return "AABB";
		}
		case QUATERNION: {
			return "Quaternion";
		}
		//case BASIS: {
		//	return "Basis";
		//}
		case TRANSFORM3D: {
			return "Transform3D";
		}
		case PROJECTION: {
			return "Projection";
		}

					   // Miscellaneous types.
		//case COLOR: {
		//	return "Color";
		//}
		case RID: {
			return "RID";
		}
		case OBJECT: {
			return "Object";
		}
		case CALLABLE: {
			return "Callable";
		}
		default:
			return "UNKNOWN";
		};
	}

	Variant::Variant(const String& p_string) {
		type = STRING;
		memnew_placement(_data._mem, String(p_string));
	}
	Variant::Variant(const char* const p_cstring) {
		type = STRING;
		memnew_placement(_data._mem, String((const char*)p_cstring));
	}

	Variant::Variant(const Vector<String>& p_string_array) {
		type = PACKED_STRING_ARRAY;
		_data.packed_array = PackedArrayRef<String>::create(p_string_array);
	}
	u32 Variant::recursive_hash(int recursion_count) const {
		return 114514;
	}

		
}