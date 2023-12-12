#include "variant.h"
#include "core/os/memory.h"
#include "core/object/refcounted.h"
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


	Variant::Variant(int64_t p_int) {
		type = INT;
		_data._int = p_int;
	}

	Variant::Variant(uint64_t p_int) {
		type = INT;
		_data._int = p_int;
	}

	Variant::Variant(float p_float) {
		type = FLOAT;
		_data._float = p_float;
	}

	Variant::Variant(double p_double) {
		type = FLOAT;
		_data._float = p_double;
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
	Variant::Variant(const Vector<float>& p_float32_array) {
		type = PACKED_FLOAT32_ARRAY;
		_data.packed_array = PackedArrayRef<float>::create(p_float32_array);
	}
	Variant::Variant(const Vector<int32_t>& p_int32_array) {
		type = PACKED_INT32_ARRAY;
		_data.packed_array = PackedArrayRef<int32_t>::create(p_int32_array);
	}

	Variant::Variant(const Reflection::ReflectionInstance* p_instance) {
		type = REFLECTIONINSTANCE;
		memnew_placement(_data._ptr,void*);
		_data._ptr = const_cast<Reflection::ReflectionInstance*>(p_instance);
	}
	Variant::ObjData& Variant::_get_obj() {
		return *reinterpret_cast<ObjData*>(&_data._mem[0]);
	}
	Variant::Variant(const Object* p_obj) {
		type = OBJECT;
		memnew_placement(_data._mem, ObjData);
		if (p_obj) {
			if (p_obj->is_ref_counted()) {
				RefCounted* ref_counted = const_cast<RefCounted*>(static_cast<const RefCounted*>(p_obj));
				if (!ref_counted->init_ref()) {
					// 如果引用计数初始化失败，则将obj置空并返回
					_get_obj().obj = nullptr;
					_get_obj().id = ObjectID();
					return;
				}
			}
			// if refcount refcount + 1 ;
			_get_obj().obj = const_cast<Object*>(p_obj);
			_get_obj().id = p_obj->get_instance_id();
		}
		else {
			// null input.
			_get_obj().obj = nullptr;
			_get_obj().id = ObjectID();
		}
	}

	u32 Variant::recursive_hash(int recursion_count) const {
		return 114514;
	}


	String Variant::stringify(int recursion_count) const {
		switch (type) {
		case NIL:
			return "<null>";
		case BOOL:
			return _data._bool ? "true" : "false";
		case INT:
			return itos(_data._int);
		case FLOAT:
			return rtos(_data._float);
		case STRING:
			return *reinterpret_cast<const String*>(_data._mem);
		case REFLECTIONINSTANCE: {
			auto ptr = reinterpret_cast<Reflection::ReflectionInstance*>(_data._ptr);
			
			return Reflection::TypeMeta::writeByName(ptr->m_meta.getTypeName(), ptr->m_instance).dump();
		}

			
		case DICTIONARY: {
			const Dictionary& d = *reinterpret_cast<const Dictionary*>(_data._mem);
			if (recursion_count > MAX_RECURSION) {
				ERR_PRINT("Maximum dictionary recursion reached!");
				return "{ ... }";
			}

			// Add leading and trailing space to Dictionary printing. This distinguishes it
			// from array printing on fonts that have similar-looking {} and [] characters.
			String str("{ ");
			List<Variant> keys;
			d.get_key_list(&keys);

			Vector<_VariantStrPair> pairs;

			recursion_count++;
			for (List<Variant>::Element* E = keys.front(); E; E = E->next()) {
				_VariantStrPair sp;
				sp.key = stringify_variant_clean(E->get(), recursion_count);
				sp.value = stringify_variant_clean(d[E->get()], recursion_count);

				pairs.push_back(sp);
			}

			for (int i = 0; i < pairs.size(); i++) {
				if (i > 0) {
					str += ", ";
				}
				str += pairs[i].key + ": " + pairs[i].value;
			}
			str += " }";

			return str;
		}
		case PACKED_VECTOR2_ARRAY: {
			return stringify_vector(operator Vector<Vector2>(), recursion_count);
		}
		case PACKED_VECTOR3_ARRAY: {
			return stringify_vector(operator Vector<Vector3>(), recursion_count);
		}
		case PACKED_COLOR_ARRAY: {
			return stringify_vector(operator Vector<Color>(), recursion_count);
		}
		case PACKED_STRING_ARRAY: {
			return stringify_vector(operator Vector<String>(), recursion_count);
		}
		case PACKED_BYTE_ARRAY: {
			return stringify_vector(operator Vector<uint8_t>(), recursion_count);
		}
		case PACKED_INT32_ARRAY: {
			return stringify_vector(operator Vector<int32_t>(), recursion_count);
		}
		case PACKED_INT64_ARRAY: {
			return stringify_vector(operator Vector<int64_t>(), recursion_count);
		}
		case PACKED_FLOAT32_ARRAY: {
			return stringify_vector(operator Vector<float>(), recursion_count);
		}
		case PACKED_FLOAT64_ARRAY: {
			return stringify_vector(operator Vector<double>(), recursion_count);
		}
		case ARRAY: {
			Array arr = operator Array();
			if (recursion_count > MAX_RECURSION) {
				ERR_PRINT("Maximum array recursion reached!");
				return "[...]";
			}

			return stringify_vector(arr, recursion_count);
		}
		case OBJECT: {
			if (_get_obj().obj) {
				if (!_get_obj().id.is_ref_counted() && ObjectDB::get_instance(_get_obj().id) == nullptr) {
					return "<Freed Object>";
				}

				return _get_obj().obj->to_string();
			}
			else {
				return "<Object#null>";
			}
		}
		case CALLABLE: {
			const Callable& c = *reinterpret_cast<const Callable*>(_data._mem);
			return c;
		}
		case SIGNAL: {
			const Signal& s = *reinterpret_cast<const Signal*>(_data._mem);
			return s;
		}
		case RID: {
			const ::RID& s = *reinterpret_cast<const ::RID*>(_data._mem);
			return "RID(" + itos(s.get_id()) + ")";
		}
		default: {
			return "<" + get_type_name(type) + ">";
		}
		}
	}
}