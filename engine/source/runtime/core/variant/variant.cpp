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

	uint32_t Variant::recursive_hash(int recursion_count) const {
			switch (type) {
			case NIL: {
				return 0;
			} break;
			case BOOL: {
				return _data._bool ? 1 : 0;
			} break;
			case INT: {
				return hash_one_uint64((uint64_t)_data._int);
			} break;
			case FLOAT: {
				return hash_murmur3_one_double(_data._float);
			} break;
			case STRING: {
				return reinterpret_cast<const String*>(_data._mem)->hash();
			} break;

				// math types
			case VECTOR2: {
				return HashMapHasherDefault::hash(*reinterpret_cast<const Vector2*>(_data._mem));
			} break;
			case VECTOR2I: {
				return HashMapHasherDefault::hash(*reinterpret_cast<const Vector2i*>(_data._mem));
			} break;
			case RECT2: {
				return HashMapHasherDefault::hash(*reinterpret_cast<const Rect2*>(_data._mem));
			} break;
			case RECT2I: {
				return HashMapHasherDefault::hash(*reinterpret_cast<const Rect2i*>(_data._mem));
			} break;
			case TRANSFORM2D: {
				uint32_t h = HASH_MURMUR3_SEED;
				const Transform2D& t = *_data._transform2d;
				h = hash_murmur3_one_real(t[0].x, h);
				h = hash_murmur3_one_real(t[0].y, h);
				h = hash_murmur3_one_real(t[1].x, h);
				h = hash_murmur3_one_real(t[1].y, h);
				h = hash_murmur3_one_real(t[2].x, h);
				h = hash_murmur3_one_real(t[2].y, h);

				return hash_fmix32(h);
			} break;
			case VECTOR3: {
				return HashMapHasherDefault::hash(*reinterpret_cast<const Vector3*>(_data._mem));
			} break;
			case VECTOR3I: {
				return HashMapHasherDefault::hash(*reinterpret_cast<const Vector3i*>(_data._mem));
			} break;
			case VECTOR4: {
				return HashMapHasherDefault::hash(*reinterpret_cast<const Vector4*>(_data._mem));
			} break;
			case VECTOR4I: {
				return HashMapHasherDefault::hash(*reinterpret_cast<const Vector4i*>(_data._mem));
			} break;
			case PLANE: {
				uint32_t h = HASH_MURMUR3_SEED;
				const Plane& p = *reinterpret_cast<const Plane*>(_data._mem);
				h = hash_murmur3_one_real(p.normal.x, h);
				h = hash_murmur3_one_real(p.normal.y, h);
				h = hash_murmur3_one_real(p.normal.z, h);
				h = hash_murmur3_one_real(p.d, h);
				return hash_fmix32(h);
			} break;
			case AABB: {
				return HashMapHasherDefault::hash(*_data._aabb);
			} break;
			case QUATERNION: {
				uint32_t h = HASH_MURMUR3_SEED;
				const Quaternion& q = *reinterpret_cast<const Quaternion*>(_data._mem);
				h = hash_murmur3_one_real(q.x, h);
				h = hash_murmur3_one_real(q.y, h);
				h = hash_murmur3_one_real(q.z, h);
				h = hash_murmur3_one_real(q.w, h);
				return hash_fmix32(h);
			} break;
			case BASIS: {
				uint32_t h = HASH_MURMUR3_SEED;
				const Basis& b = *_data._basis;
				h = hash_murmur3_one_real(b[0].x, h);
				h = hash_murmur3_one_real(b[0].y, h);
				h = hash_murmur3_one_real(b[0].z, h);
				h = hash_murmur3_one_real(b[1].x, h);
				h = hash_murmur3_one_real(b[1].y, h);
				h = hash_murmur3_one_real(b[1].z, h);
				h = hash_murmur3_one_real(b[2].x, h);
				h = hash_murmur3_one_real(b[2].y, h);
				h = hash_murmur3_one_real(b[2].z, h);
				return hash_fmix32(h);
			} break;
			case TRANSFORM3D: {
				uint32_t h = HASH_MURMUR3_SEED;
				const Transform3D& t = *_data._transform3d;
				h = hash_murmur3_one_real(t.basis[0].x, h);
				h = hash_murmur3_one_real(t.basis[0].y, h);
				h = hash_murmur3_one_real(t.basis[0].z, h);
				h = hash_murmur3_one_real(t.basis[1].x, h);
				h = hash_murmur3_one_real(t.basis[1].y, h);
				h = hash_murmur3_one_real(t.basis[1].z, h);
				h = hash_murmur3_one_real(t.basis[2].x, h);
				h = hash_murmur3_one_real(t.basis[2].y, h);
				h = hash_murmur3_one_real(t.basis[2].z, h);
				h = hash_murmur3_one_real(t.origin.x, h);
				h = hash_murmur3_one_real(t.origin.y, h);
				h = hash_murmur3_one_real(t.origin.z, h);
				return hash_fmix32(h);
			} break;
			case PROJECTION: {
				uint32_t h = HASH_MURMUR3_SEED;
				const Projection& t = *_data._projection;
				h = hash_murmur3_one_real(t.columns[0].x, h);
				h = hash_murmur3_one_real(t.columns[0].y, h);
				h = hash_murmur3_one_real(t.columns[0].z, h);
				h = hash_murmur3_one_real(t.columns[0].w, h);
				h = hash_murmur3_one_real(t.columns[1].x, h);
				h = hash_murmur3_one_real(t.columns[1].y, h);
				h = hash_murmur3_one_real(t.columns[1].z, h);
				h = hash_murmur3_one_real(t.columns[1].w, h);
				h = hash_murmur3_one_real(t.columns[2].x, h);
				h = hash_murmur3_one_real(t.columns[2].y, h);
				h = hash_murmur3_one_real(t.columns[2].z, h);
				h = hash_murmur3_one_real(t.columns[2].w, h);
				h = hash_murmur3_one_real(t.columns[3].x, h);
				h = hash_murmur3_one_real(t.columns[3].y, h);
				h = hash_murmur3_one_real(t.columns[3].z, h);
				h = hash_murmur3_one_real(t.columns[3].w, h);
				return hash_fmix32(h);
			} break;
				// misc types
			case COLOR: {
				uint32_t h = HASH_MURMUR3_SEED;
				const Color& c = *reinterpret_cast<const Color*>(_data._mem);
				h = hash_murmur3_one_float(c.r, h);
				h = hash_murmur3_one_float(c.g, h);
				h = hash_murmur3_one_float(c.b, h);
				h = hash_murmur3_one_float(c.a, h);
				return hash_fmix32(h);
			} break;
			case RID: {
				return hash_one_uint64(reinterpret_cast<const ::RID*>(_data._mem)->get_id());
			} break;
			case OBJECT: {
				return hash_one_uint64(hash_make_uint64_t(_get_obj().obj));
			} break;
			case STRING_NAME: {
				return reinterpret_cast<const StringName*>(_data._mem)->hash();
			} break;
			case NODE_PATH: {
				return reinterpret_cast<const NodePath*>(_data._mem)->hash();
			} break;
			case DICTIONARY: {
				return reinterpret_cast<const Dictionary*>(_data._mem)->recursive_hash(recursion_count);

			} break;
			case CALLABLE: {
				return reinterpret_cast<const Callable*>(_data._mem)->hash();

			} break;
			case SIGNAL: {
				const Signal& s = *reinterpret_cast<const Signal*>(_data._mem);
				uint32_t hash = s.get_name().hash();
				return hash_murmur3_one_64(s.get_object_id(), hash);
			} break;
			case ARRAY: {
				const Array& arr = *reinterpret_cast<const Array*>(_data._mem);
				return arr.recursive_hash(recursion_count);

			} break;
			case PACKED_BYTE_ARRAY: {
				const Vector<uint8_t>& arr = PackedArrayRef<uint8_t>::get_array(_data.packed_array);
				int len = arr.size();
				if (likely(len)) {
					const uint8_t* r = arr.ptr();
					return hash_murmur3_buffer((uint8_t*)&r[0], len);
				}
				else {
					return hash_murmur3_one_64(0);
				}

			} break;
			case PACKED_INT32_ARRAY: {
				const Vector<int32_t>& arr = PackedArrayRef<int32_t>::get_array(_data.packed_array);
				int len = arr.size();
				if (likely(len)) {
					const int32_t* r = arr.ptr();
					return hash_murmur3_buffer((uint8_t*)&r[0], len * sizeof(int32_t));
				}
				else {
					return hash_murmur3_one_64(0);
				}

			} break;
			case PACKED_INT64_ARRAY: {
				const Vector<int64_t>& arr = PackedArrayRef<int64_t>::get_array(_data.packed_array);
				int len = arr.size();
				if (likely(len)) {
					const int64_t* r = arr.ptr();
					return hash_murmur3_buffer((uint8_t*)&r[0], len * sizeof(int64_t));
				}
				else {
					return hash_murmur3_one_64(0);
				}

			} break;
			case PACKED_FLOAT32_ARRAY: {
				const Vector<float>& arr = PackedArrayRef<float>::get_array(_data.packed_array);
				int len = arr.size();

				if (likely(len)) {
					const float* r = arr.ptr();
					uint32_t h = HASH_MURMUR3_SEED;
					for (int32_t i = 0; i < len; i++) {
						h = hash_murmur3_one_float(r[i], h);
					}
					return hash_fmix32(h);
				}
				else {
					return hash_murmur3_one_float(0.0);
				}

			} break;
			case PACKED_FLOAT64_ARRAY: {
				const Vector<double>& arr = PackedArrayRef<double>::get_array(_data.packed_array);
				int len = arr.size();

				if (likely(len)) {
					const double* r = arr.ptr();
					uint32_t h = HASH_MURMUR3_SEED;
					for (int32_t i = 0; i < len; i++) {
						h = hash_murmur3_one_double(r[i], h);
					}
					return hash_fmix32(h);
				}
				else {
					return hash_murmur3_one_double(0.0);
				}

			} break;
			case PACKED_STRING_ARRAY: {
				uint32_t hash = HASH_MURMUR3_SEED;
				const Vector<String>& arr = PackedArrayRef<String>::get_array(_data.packed_array);
				int len = arr.size();

				if (likely(len)) {
					const String* r = arr.ptr();

					for (int i = 0; i < len; i++) {
						hash = hash_murmur3_one_32(r[i].hash(), hash);
					}
					hash = hash_fmix32(hash);
				}

				return hash;
			} break;
			case PACKED_VECTOR2_ARRAY: {
				uint32_t hash = HASH_MURMUR3_SEED;
				const Vector<Vector2>& arr = PackedArrayRef<Vector2>::get_array(_data.packed_array);
				int len = arr.size();

				if (likely(len)) {
					const Vector2* r = arr.ptr();

					for (int i = 0; i < len; i++) {
						hash = hash_murmur3_one_real(r[i].x, hash);
						hash = hash_murmur3_one_real(r[i].y, hash);
					}
					hash = hash_fmix32(hash);
				}

				return hash;
			} break;
			case PACKED_VECTOR3_ARRAY: {
				uint32_t hash = HASH_MURMUR3_SEED;
				const Vector<Vector3>& arr = PackedArrayRef<Vector3>::get_array(_data.packed_array);
				int len = arr.size();

				if (likely(len)) {
					const Vector3* r = arr.ptr();

					for (int i = 0; i < len; i++) {
						hash = hash_murmur3_one_real(r[i].x, hash);
						hash = hash_murmur3_one_real(r[i].y, hash);
						hash = hash_murmur3_one_real(r[i].z, hash);
					}
					hash = hash_fmix32(hash);
				}

				return hash;
			} break;
			case PACKED_COLOR_ARRAY: {
				uint32_t hash = HASH_MURMUR3_SEED;
				const Vector<Color>& arr = PackedArrayRef<Color>::get_array(_data.packed_array);
				int len = arr.size();

				if (likely(len)) {
					const Color* r = arr.ptr();

					for (int i = 0; i < len; i++) {
						hash = hash_murmur3_one_float(r[i].r, hash);
						hash = hash_murmur3_one_float(r[i].g, hash);
						hash = hash_murmur3_one_float(r[i].b, hash);
						hash = hash_murmur3_one_float(r[i].a, hash);
					}
					hash = hash_fmix32(hash);
				}

				return hash;
			} break;
			default: {
			}
			}

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