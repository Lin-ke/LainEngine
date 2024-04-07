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
		case COLOR: {
			return "Color";
		}
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

	Variant::Variant(bool p_bool) {
		type = BOOL;
		_data._bool = p_bool;
	}
	Variant::Variant(signed int p_int) {
		type = INT;
		_data._int = p_int;
	}

	Variant::Variant(unsigned int p_int) {
		type = INT;
		_data._int = p_int;
	}

	Variant::Variant(const String& p_string) {
		type = STRING;
		memnew_placement(_data._mem, String(p_string));
	}
	Variant::Variant(const char* const p_cstring) {
		type = STRING;
		memnew_placement(_data._mem, String((const char*)p_cstring));
	}

	Variant::Variant(const Dictionary& p_dictionary) {
		type = DICTIONARY;
		memnew_placement(_data._mem, Dictionary(p_dictionary));
	}

	Variant::Variant(const Array& p_array) {
		type = ARRAY;
		memnew_placement(_data._mem, Array(p_array));
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
	Variant::Variant(const Vector<double>& p_double_array) {
		type = PACKED_FLOAT64_ARRAY;
		_data.packed_array = PackedArrayRef<double>::create(p_double_array);
	}

	Variant::Variant(const Reflection::ReflectionInstance* p_instance) {
		type = REFLECTIONINSTANCE;
		memnew_placement(_data._ptr,void*);
		_data._ptr = const_cast<Reflection::ReflectionInstance*>(p_instance);
	}

	Variant::Variant(const Vector3& p_vector3) {
		type = VECTOR3;
		memnew_placement(_data._mem, Vector3(p_vector3));
	}

	Variant::Variant(const Vector2& p_vector2) {
		type = VECTOR2;
		memnew_placement(_data._mem, Vector2(p_vector2));
	}

	const Variant::ObjData& Variant::_get_obj() const {
		return *reinterpret_cast<const ObjData*>(&_data._mem[0]);
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
	Variant::Variant(const Variant& p_variant) {
		reference(p_variant);
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
			//case VECTOR2I: {
			//	return HashMapHasherDefault::hash(*reinterpret_cast<const Vector2i*>(_data._mem));
			//} break;
			//case RECT2: {
			//	return HashMapHasherDefault::hash(*reinterpret_cast<const Rect2*>(_data._mem));
			//} break;
			//case RECT2I: {
			//	return HashMapHasherDefault::hash(*reinterpret_cast<const Rect2i*>(_data._mem));
			//} break;
			//case TRANSFORM2D: {
			//	uint32_t h = HASH_MURMUR3_SEED;
			//	const Transform2D& t = *_data._transform2d;
			//	h = hash_murmur3_one_real(t[0].x, h);
			//	h = hash_murmur3_one_real(t[0].y, h);
			//	h = hash_murmur3_one_real(t[1].x, h);
			//	h = hash_murmur3_one_real(t[1].y, h);
			//	h = hash_murmur3_one_real(t[2].x, h);
			//	h = hash_murmur3_one_real(t[2].y, h);

			//	return hash_fmix32(h);
			//} break;
			//case VECTOR3: {
			//	return HashMapHasherDefault::hash(*reinterpret_cast<const Vector3*>(_data._mem));
			//} break;
			//case VECTOR3I: {
			//	return HashMapHasherDefault::hash(*reinterpret_cast<const Vector3i*>(_data._mem));
			//} break;
			//case VECTOR4: {
			//	return HashMapHasherDefault::hash(*reinterpret_cast<const Vector4*>(_data._mem));
			//} break;
			//case VECTOR4I: {
			//	return HashMapHasherDefault::hash(*reinterpret_cast<const Vector4i*>(_data._mem));
			//} break;
			///*case PLANE: {
			//	uint32_t h = HASH_MURMUR3_SEED;
			//	const Plane& p = *reinterpret_cast<const Plane*>(_data._mem);
			//	h = hash_murmur3_one_real(p.normal.x, h);
			//	h = hash_murmur3_one_real(p.normal.y, h);
			//	h = hash_murmur3_one_real(p.normal.z, h);
			//	h = hash_murmur3_one_real(p.d, h);
			//	return hash_fmix32(h);
			//} break;
			//case AABB: {
			//	return HashMapHasherDefault::hash(*_data._aabb);
			//} break;*/
			//case QUATERNION: {
			//	uint32_t h = HASH_MURMUR3_SEED;
			//	const Quaternion& q = *reinterpret_cast<const Quaternion*>(_data._mem);
			//	h = hash_murmur3_one_real(q.x, h);
			//	h = hash_murmur3_one_real(q.y, h);
			//	h = hash_murmur3_one_real(q.z, h);
			//	h = hash_murmur3_one_real(q.w, h);
			//	return hash_fmix32(h);
			//} break;
			/*case BASIS: {
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
			} break;*/
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
				return hash_one_uint64(reinterpret_cast<const lain::RID*>(_data._mem)->get_id());
			} break;
			case OBJECT: {
				return hash_one_uint64(hash_make_uint64_t(_get_obj().obj));
			} break;
			case STRING_NAME: {
				return reinterpret_cast<const StringName*>(_data._mem)->hash();
			} break;
			/*case GOBJECT_PATH: {
				return reinterpret_cast<const GObjectPath*>(_data._mem)->hash();
			} break;
			case DICTIONARY: {
				return reinterpret_cast<const Dictionary*>(_data._mem)->recursive_hash(recursion_count);

			} break;*/
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
		//case OBJECT: {
		//	if (_get_obj().obj) {
		//		if (!_get_obj().id.is_ref_counted() && ObjectDB::get_instance(_get_obj().id) == nullptr) {
		//			return "<Freed Object>";
		//		}

		//		return _get_obj().obj->to_string();
		//	}
		//	else {
		//		return "<Object#null>";
		//	}
		//}
		/*case CALLABLE: {
			const Callable& c = *reinterpret_cast<const Callable*>(_data._mem);
			return c;
		}
		case SIGNAL: {
			const Signal& s = *reinterpret_cast<const Signal*>(_data._mem);
			return s;
		}*/
		case RID: {
			const lain::RID& s = *reinterpret_cast<const lain::RID*>(_data._mem);
			return "RID(" + itos(s.get_id()) + ")";
		}
		default: {
			return "<" + get_type_name(type) + ">";
		}
		}
	}


	/*
	* hash compare
	* 
	*/
	bool Variant::operator==(const Variant& p_variant) const {
		return hash_compare(p_variant);
	}

	bool Variant::hash_compare(const Variant& p_variant, int recursion_count) const {
		if (type != p_variant.type) {
			return false;
		}
		// switch type
		return false;
	}

	/*
	* copy
	*/
	Variant Variant::duplicate(bool p_deep) const {
		return recursive_duplicate(p_deep, 0);
	}
	Variant Variant::recursive_duplicate(bool p_deep, int recursion_count) const {
		return Variant();
	}

	// equal to another variant
	void Variant::reference(const Variant& p_variant) {
		switch (type) {
		case NIL:
		case BOOL:
		case INT:
		case FLOAT:
			break;
		default:
			//clear(); // _clear_internal
			type = NIL;
		}
		type = p_variant.type;

		switch (p_variant.type) {
		case NIL: {
			// None.
		} break;

			// Atomic types.
		case BOOL: {
			_data._bool = p_variant._data._bool;
		} break;
		case INT: {
			_data._int = p_variant._data._int;
		} break;
		case FLOAT: {
			_data._float = p_variant._data._float;
		} break;
		case STRING: {
			memnew_placement(_data._mem, String(*reinterpret_cast<const String*>(p_variant._data._mem)));
		} break;

			// Math types.
		case VECTOR2: {
			memnew_placement(_data._mem, Vector2(*reinterpret_cast<const Vector2*>(p_variant._data._mem)));
		} break;
		case VECTOR2I: {
			memnew_placement(_data._mem, Vector2i(*reinterpret_cast<const Vector2i*>(p_variant._data._mem)));
		} break;
			/*
		case RECT2: {
			memnew_placement(_data._mem, Rect2(*reinterpret_cast<const Rect2*>(p_variant._data._mem)));
		} break;
		case RECT2I: {
			memnew_placement(_data._mem, Rect2i(*reinterpret_cast<const Rect2i*>(p_variant._data._mem)));
		} break;
		case TRANSFORM2D: {
			_data._transform2d = (Transform2D*)Pools::_bucket_small.alloc();
			memnew_placement(_data._transform2d, Transform2D(*p_variant._data._transform2d));
		} break;
		case VECTOR3: {
			memnew_placement(_data._mem, Vector3(*reinterpret_cast<const Vector3*>(p_variant._data._mem)));
		} break;
		case VECTOR3I: {
			memnew_placement(_data._mem, Vector3i(*reinterpret_cast<const Vector3i*>(p_variant._data._mem)));
		} break;
		case VECTOR4: {
			memnew_placement(_data._mem, Vector4(*reinterpret_cast<const Vector4*>(p_variant._data._mem)));
		} break;
		case VECTOR4I: {
			memnew_placement(_data._mem, Vector4i(*reinterpret_cast<const Vector4i*>(p_variant._data._mem)));
		} break;
		case PLANE: {
			memnew_placement(_data._mem, Plane(*reinterpret_cast<const Plane*>(p_variant._data._mem)));
		} break;
		case AABB: {
			_data._aabb = (::AABB*)Pools::_bucket_small.alloc();
			memnew_placement(_data._aabb, ::AABB(*p_variant._data._aabb));
		} break;
		case QUATERNION: {
			memnew_placement(_data._mem, Quaternion(*reinterpret_cast<const Quaternion*>(p_variant._data._mem)));
		} break;
		case BASIS: {
			_data._basis = (Basis*)Pools::_bucket_medium.alloc();
			memnew_placement(_data._basis, Basis(*p_variant._data._basis));
		} break;
		case TRANSFORM3D: {
			_data._transform3d = (Transform3D*)Pools::_bucket_medium.alloc();
			memnew_placement(_data._transform3d, Transform3D(*p_variant._data._transform3d));
		} break;
		case PROJECTION: {
			_data._projection = (Projection*)Pools::_bucket_large.alloc();
			memnew_placement(_data._projection, Projection(*p_variant._data._projection));
		} break;*/

			// Miscellaneous types.
		case COLOR: {
			memnew_placement(_data._mem, Color(*reinterpret_cast<const Color*>(p_variant._data._mem)));
		} break;
		case RID: {
			memnew_placement(_data._mem, lain::RID(*reinterpret_cast<const lain::RID*>(p_variant._data._mem)));
		} break;
		case OBJECT: {
			memnew_placement(_data._mem, ObjData);

			if (p_variant._get_obj().obj && p_variant._get_obj().id.is_ref_counted()) {
				RefCounted* ref_counted = static_cast<RefCounted*>(p_variant._get_obj().obj);
				if (!ref_counted->reference()) {
					_get_obj().obj = nullptr;
					_get_obj().id = ObjectID();
					break;
				}
			}

			_get_obj().obj = const_cast<Object*>(p_variant._get_obj().obj);
			_get_obj().id = p_variant._get_obj().id;
		} break;
		/*case CALLABLE: {
			memnew_placement(_data._mem, Callable(*reinterpret_cast<const Callable*>(p_variant._data._mem)));
		} break;*/
		case SIGNAL: {
			memnew_placement(_data._mem, Signal(*reinterpret_cast<const Signal*>(p_variant._data._mem)));
		} break;
		case STRING_NAME: {
			memnew_placement(_data._mem, StringName(*reinterpret_cast<const StringName*>(p_variant._data._mem)));
		} break;
		case DICTIONARY: {
			memnew_placement(_data._mem, Dictionary(*reinterpret_cast<const Dictionary*>(p_variant._data._mem)));
		} break;
		case GOBJECT_PATH: {
			memnew_placement(_data._mem, GObjectPath(*reinterpret_cast<const GObjectPath*>(p_variant._data._mem)));
		} break;
		case ARRAY: {
			memnew_placement(_data._mem, Array(*reinterpret_cast<const Array*>(p_variant._data._mem)));
		} break;

			// Arrays.
		case PACKED_BYTE_ARRAY: {
			_data.packed_array = static_cast<PackedArrayRef<uint8_t> *>(p_variant._data.packed_array)->reference();
			if (!_data.packed_array) {
				_data.packed_array = PackedArrayRef<uint8_t>::create();
			}
		} break;
		case PACKED_INT32_ARRAY: {
			_data.packed_array = static_cast<PackedArrayRef<int32_t> *>(p_variant._data.packed_array)->reference();
			if (!_data.packed_array) {
				_data.packed_array = PackedArrayRef<int32_t>::create();
			}
		} break;
		case PACKED_INT64_ARRAY: {
			_data.packed_array = static_cast<PackedArrayRef<int64_t> *>(p_variant._data.packed_array)->reference();
			if (!_data.packed_array) {
				_data.packed_array = PackedArrayRef<int64_t>::create();
			}
		} break;
		case PACKED_FLOAT32_ARRAY: {
			_data.packed_array = static_cast<PackedArrayRef<float> *>(p_variant._data.packed_array)->reference();
			if (!_data.packed_array) {
				_data.packed_array = PackedArrayRef<float>::create();
			}
		} break;
		case PACKED_FLOAT64_ARRAY: {
			_data.packed_array = static_cast<PackedArrayRef<double> *>(p_variant._data.packed_array)->reference();
			if (!_data.packed_array) {
				_data.packed_array = PackedArrayRef<double>::create();
			}
		} break;
		case PACKED_STRING_ARRAY: {
			_data.packed_array = static_cast<PackedArrayRef<String> *>(p_variant._data.packed_array)->reference();
			if (!_data.packed_array) {
				_data.packed_array = PackedArrayRef<String>::create();
			}
		} break;
		case PACKED_VECTOR2_ARRAY: {
			_data.packed_array = static_cast<PackedArrayRef<Vector2> *>(p_variant._data.packed_array)->reference();
			if (!_data.packed_array) {
				_data.packed_array = PackedArrayRef<Vector2>::create();
			}
		} break;
		case PACKED_VECTOR3_ARRAY: {
			_data.packed_array = static_cast<PackedArrayRef<Vector3> *>(p_variant._data.packed_array)->reference();
			if (!_data.packed_array) {
				_data.packed_array = PackedArrayRef<Vector3>::create();
			}
		} break;
		case PACKED_COLOR_ARRAY: {
			_data.packed_array = static_cast<PackedArrayRef<Color> *>(p_variant._data.packed_array)->reference();
			if (!_data.packed_array) {
				_data.packed_array = PackedArrayRef<Color>::create();
			}
		} break;
		default: {
		}
		}
	}

	void Variant::operator=(const Variant& p_variant) {
		if (unlikely(this == &p_variant)) {
			return;
		}

		if (unlikely(type != p_variant.type)) {
			reference(p_variant);
			return;
		}

		switch (p_variant.type) {
		case NIL: {
			// none
		} break;

			// atomic types
		case BOOL: {
			_data._bool = p_variant._data._bool;
		} break;
		case INT: {
			_data._int = p_variant._data._int;
		} break;
		case FLOAT: {
			_data._float = p_variant._data._float;
		} break;
		case STRING: {
			*reinterpret_cast<String*>(_data._mem) = *reinterpret_cast<const String*>(p_variant._data._mem);
		} break;

			// math types
		case VECTOR2: {
			*reinterpret_cast<Vector2*>(_data._mem) = *reinterpret_cast<const Vector2*>(p_variant._data._mem);
		} break;
		case VECTOR2I: {
			*reinterpret_cast<Vector2i*>(_data._mem) = *reinterpret_cast<const Vector2i*>(p_variant._data._mem);
		} break;
		case RECT2: {
			*reinterpret_cast<Rect2*>(_data._mem) = *reinterpret_cast<const Rect2*>(p_variant._data._mem);
		} break;
		//case RECT2I: {
		//	*reinterpret_cast<Rect2i*>(_data._mem) = *reinterpret_cast<const Rect2i*>(p_variant._data._mem);
		//} break;
		//case TRANSFORM2D: {
		//	*_data._transform2d = *(p_variant._data._transform2d);
		//} break;
		case VECTOR3: {
			*reinterpret_cast<Vector3*>(_data._mem) = *reinterpret_cast<const Vector3*>(p_variant._data._mem);
		} break;
		case VECTOR3I: {
			*reinterpret_cast<Vector3i*>(_data._mem) = *reinterpret_cast<const Vector3i*>(p_variant._data._mem);
		} break;
		case VECTOR4: {
			*reinterpret_cast<Vector4*>(_data._mem) = *reinterpret_cast<const Vector4*>(p_variant._data._mem);
		} break;
		case VECTOR4I: {
			*reinterpret_cast<Vector4i*>(_data._mem) = *reinterpret_cast<const Vector4i*>(p_variant._data._mem);
		} break;
		//case PLANE: {
		//	*reinterpret_cast<Plane*>(_data._mem) = *reinterpret_cast<const Plane*>(p_variant._data._mem);
		//} break;

		case AABB: {
			*_data._aabb = *(p_variant._data._aabb);
		} break;
		//case QUATERNION: {
		//	*reinterpret_cast<Quaternion*>(_data._mem) = *reinterpret_cast<const Quaternion*>(p_variant._data._mem);
		//} break;
		//case BASIS: {
		//	*_data._basis = *(p_variant._data._basis);
		//} break;
		//case TRANSFORM3D: {
		//	*_data._transform3d = *(p_variant._data._transform3d);
		//} break;
		//case PROJECTION: {
		//	*_data._projection = *(p_variant._data._projection);
		//} break;

		//	// misc types
		case COLOR: {
			*reinterpret_cast<Color*>(_data._mem) = *reinterpret_cast<const Color*>(p_variant._data._mem);
		} break;
		case RID: {
			*reinterpret_cast<lain::RID*>(_data._mem) = *reinterpret_cast<const lain::RID*>(p_variant._data._mem);
		} break;
		/*case OBJECT: {
			if (_get_obj().id.is_ref_counted()) {
				//we are safe that there is a reference here
				RefCounted* ref_counted = static_cast<RefCounted*>(_get_obj().obj);
				if (ref_counted->unreference()) {
					memdelete(ref_counted);
				}
			}

			if (p_variant._get_obj().obj && p_variant._get_obj().id.is_ref_counted()) {
				RefCounted* ref_counted = static_cast<RefCounted*>(p_variant._get_obj().obj);
				if (!ref_counted->reference()) {
					_get_obj().obj = nullptr;
					_get_obj().id = ObjectID();
					break;
				}
			}

			_get_obj().obj = const_cast<Object*>(p_variant._get_obj().obj);
			_get_obj().id = p_variant._get_obj().id;

		} break;
			case CALLABLE: {
			*reinterpret_cast<Callable*>(_data._mem) = *reinterpret_cast<const Callable*>(p_variant._data._mem);
		} break;*/
		case SIGNAL: {
			*reinterpret_cast<Signal*>(_data._mem) = *reinterpret_cast<const Signal*>(p_variant._data._mem);
		} break;

		case ARRAY: {
			*reinterpret_cast<Array*>(_data._mem) = *reinterpret_cast<const Array*>(p_variant._data._mem);
		} break;

		case STRING_NAME: {
			*reinterpret_cast<StringName*>(_data._mem) = *reinterpret_cast<const StringName*>(p_variant._data._mem);
		} break;
		case GOBJECT_PATH: {
			*reinterpret_cast<GObjectPath*>(_data._mem) = *reinterpret_cast<const GObjectPath*>(p_variant._data._mem);
		} break;
		case DICTIONARY: {
			*reinterpret_cast<Dictionary*>(_data._mem) = *reinterpret_cast<const Dictionary*>(p_variant._data._mem);
		} break;
			// arrays
		case PACKED_BYTE_ARRAY: {
			_data.packed_array = PackedArrayRef<uint8_t>::reference_from(_data.packed_array, p_variant._data.packed_array);
		} break;
		case PACKED_INT32_ARRAY: {
			_data.packed_array = PackedArrayRef<int32_t>::reference_from(_data.packed_array, p_variant._data.packed_array);
		} break;
		case PACKED_INT64_ARRAY: {
			_data.packed_array = PackedArrayRef<int64_t>::reference_from(_data.packed_array, p_variant._data.packed_array);
		} break;
		case PACKED_FLOAT32_ARRAY: {
			_data.packed_array = PackedArrayRef<float>::reference_from(_data.packed_array, p_variant._data.packed_array);
		} break;
		case PACKED_FLOAT64_ARRAY: {
			_data.packed_array = PackedArrayRef<double>::reference_from(_data.packed_array, p_variant._data.packed_array);
		} break;
		case PACKED_STRING_ARRAY: {
			_data.packed_array = PackedArrayRef<String>::reference_from(_data.packed_array, p_variant._data.packed_array);
		} break;
		case PACKED_VECTOR2_ARRAY: {
			_data.packed_array = PackedArrayRef<Vector2>::reference_from(_data.packed_array, p_variant._data.packed_array);
		} break;
		case PACKED_VECTOR3_ARRAY: {
			_data.packed_array = PackedArrayRef<Vector3>::reference_from(_data.packed_array, p_variant._data.packed_array);
		} break;
		case PACKED_COLOR_ARRAY: {
			_data.packed_array = PackedArrayRef<Color>::reference_from(_data.packed_array, p_variant._data.packed_array);
		} break;
		default: {
		}
		}

	}
	/*
	* transfer to other types
	*/
	Variant::operator bool() const {
		switch (type) {
		case NIL:
			return false;
		case BOOL:
			return _data._bool;
		case INT:
			return _data._int == 0;
		case FLOAT:
			return _data._float == 0;
		case STRING:
			return operator String().to_int() == 0;
		default: {
			return false;
		}
		}
	}

	Variant::operator signed int() const {
		switch (type) {
		case NIL:
			return 0;
		case BOOL:
			return _data._bool ? 1 : 0;
		case INT:
			return static_cast<signed int>(_data._int);
		case FLOAT:
			return static_cast<signed int>(_data._float);
		case STRING:
			return operator String().to_int();
		default: {
			return 0;
		}
		}
	}

	Variant::operator unsigned int() const {
		switch (type) {
		case NIL:
			return 0;
		case BOOL:
			return _data._bool ? 1 : 0;
		case INT:
			return static_cast<unsigned int>(_data._int);
		case FLOAT:
			return static_cast<unsigned int>(_data._float);
		case STRING:
			return operator String().to_int();
		default: {
			return 0;
		}
		}
	}

	Variant::operator int64_t() const {
		switch (type) {
		case NIL:
			return 0;
		case BOOL:
			return _data._bool ? 1 : 0;
		case INT:
			return _data._int;
		case FLOAT:
			return static_cast<int64_t>(_data._float);
		case STRING:
			return operator String().to_int();
		default: {
			return 0;
		}
		}
	}

	Variant::operator uint64_t() const {
		switch (type) {
		case NIL:
			return 0;
		case BOOL:
			return _data._bool ? 1 : 0;
		case INT:
			return _data._int;
		case FLOAT:
			return static_cast<ui64>(_data._float);
		case STRING:
			return operator String().to_int();
		default: {
			return 0;
		}
		}
	}

	Variant::operator ObjectID() const {
		if (type == INT) {
			return ObjectID(_data._int);
		}
		else if (type == OBJECT) {
			return _get_obj().id;
		}
		else {
			return ObjectID();
		}
	}

#ifdef NEED_LONG_INT
	Variant::operator signed long() const {
		switch (type) {
		case NIL:
			return 0;
		case BOOL:
			return _data._bool ? 1 : 0;
		case INT:
			return _data._int;
		case FLOAT:
			return _data._float;
		case STRING:
			return operator String().to_int();
		default: {
			return 0;
		}
		}

		return 0;
	}

	Variant::operator unsigned long() const {
		switch (type) {
		case NIL:
			return 0;
		case BOOL:
			return _data._bool ? 1 : 0;
		case INT:
			return _data._int;
		case FLOAT:
			return _data._float;
		case STRING:
			return operator String().to_int();
		default: {
			return 0;
		}
		}

		return 0;
		}
#endif

	Variant::operator signed short() const {
		switch (type) {
		case NIL:
			return 0;
		case BOOL:
			return _data._bool ? 1 : 0;
		case INT:
			return static_cast<short>(_data._int);
		case FLOAT:
			return static_cast<short>(_data._float);
		case STRING:
			return operator String().to_int();
		default: {
			return 0;
		}
		}
	}

	Variant::operator unsigned short() const {
		switch (type) {
		case NIL:
			return 0;
		case BOOL:
			return _data._bool ? 1 : 0;
		case INT:
			return static_cast<unsigned short>(_data._int);
		case FLOAT:
			return  static_cast<unsigned short>(_data._float);
		case STRING:
			return operator String().to_int();
		default: {
			return 0;
		}
		}
	}

	Variant::operator signed char() const {
		switch (type) {
		case NIL:
			return 0;
		case BOOL:
			return _data._bool ? 1 : 0;
		case INT:
			return _data._int;
		case FLOAT:
			return _data._float;
		case STRING:
			return operator String().to_int();
		default: {
			return 0;
		}
		}
	}

	Variant::operator unsigned char() const {
		switch (type) {
		case NIL:
			return 0;
		case BOOL:
			return _data._bool ? 1 : 0;
		case INT:
			return _data._int;
		case FLOAT:
			return _data._float;
		case STRING:
			return operator String().to_int();
		default: {
			return 0;
		}
		}
	}

	Variant::operator char32_t() const {
		return operator unsigned int();
	}

	Variant::operator float() const {
		switch (type) {
		case NIL:
			return 0;
		case BOOL:
			return _data._bool ? 1.0f : 0.0f;
		case INT:
			return (float)_data._int;
		case FLOAT:
			return static_cast<float>(_data._float);
		case STRING:
			return (float) operator String().to_float();
		default: {
			return 0;
		}
		}
	}

	Variant::operator double() const {
		switch (type) {
		case NIL:
			return 0;
		case BOOL:
			return _data._bool ? 1.0 : 0.0;
		case INT:
			return (double)_data._int;
		case FLOAT:
			return _data._float;
		case STRING:
			return operator String().to_float();
		default: {
			return 0;
		}
		}
	}

	Variant::operator StringName() const {
		if (type == STRING_NAME) {
			return *reinterpret_cast<const StringName*>(_data._mem);
		}
		else if (type == STRING) {
			return *reinterpret_cast<const String*>(_data._mem);
		}

		return StringName();
	}

	Variant::operator Vector2() const {
		if (type == VECTOR2) {
			return *reinterpret_cast<const Vector2*>(_data._mem);
		}
		else if (type == VECTOR2I) {
			return *reinterpret_cast<const Vector2i*>(_data._mem);
		}
		else if (type == VECTOR3) {
			return Vector2(reinterpret_cast<const Vector3*>(_data._mem)->x, reinterpret_cast<const Vector3*>(_data._mem)->y);
		}
		else if (type == VECTOR3I) {
			return Vector2(reinterpret_cast<const Vector3i*>(_data._mem)->x, reinterpret_cast<const Vector3i*>(_data._mem)->y);
		}
		else if (type == VECTOR4) {
			return Vector2(reinterpret_cast<const Vector4*>(_data._mem)->x, reinterpret_cast<const Vector4*>(_data._mem)->y);
		}
		else if (type == VECTOR4I) {
			return Vector2(reinterpret_cast<const Vector4i*>(_data._mem)->x, reinterpret_cast<const Vector4i*>(_data._mem)->y);
		}
		else {
			return Vector2();
		}
	}

	//Variant::operator Vector2i() const {
	//	if (type == VECTOR2I) {
	//		return *reinterpret_cast<const Vector2i*>(_data._mem);
	//	}
	//	else if (type == VECTOR2) {
	//		return *reinterpret_cast<const Vector2*>(_data._mem);
	//	}
	//	else if (type == VECTOR3) {
	//		return Vector2(reinterpret_cast<const Vector3*>(_data._mem)->x, reinterpret_cast<const Vector3*>(_data._mem)->y);
	//	}
	//	else if (type == VECTOR3I) {
	//		return Vector2(reinterpret_cast<const Vector3i*>(_data._mem)->x, reinterpret_cast<const Vector3i*>(_data._mem)->y);
	//	}
	//	else if (type == VECTOR4) {
	//		return Vector2(reinterpret_cast<const Vector4*>(_data._mem)->x, reinterpret_cast<const Vector4*>(_data._mem)->y);
	//	}
	//	else if (type == VECTOR4I) {
	//		return Vector2(reinterpret_cast<const Vector4i*>(_data._mem)->x, reinterpret_cast<const Vector4i*>(_data._mem)->y);
	//	}
	//	else {
	//		return Vector2i();
	//	}
	//}

	Variant::operator Vector3() const {
		if (type == VECTOR3) {
			return *reinterpret_cast<const Vector3*>(_data._mem);
		}
		else if (type == VECTOR3I) {
			return *reinterpret_cast<const Vector3i*>(_data._mem);
		}
		else if (type == VECTOR2) {
			return Vector3(reinterpret_cast<const Vector2*>(_data._mem)->x, reinterpret_cast<const Vector2*>(_data._mem)->y, 0.0);
		}
		else if (type == VECTOR2I) {
			return Vector3(reinterpret_cast<const Vector2i*>(_data._mem)->x, reinterpret_cast<const Vector2i*>(_data._mem)->y, 0.0);
		}
		else if (type == VECTOR4) {
			return Vector3(reinterpret_cast<const Vector4*>(_data._mem)->x, reinterpret_cast<const Vector4*>(_data._mem)->y, reinterpret_cast<const Vector4*>(_data._mem)->z);
		}
		else if (type == VECTOR4I) {
			return Vector3(reinterpret_cast<const Vector4i*>(_data._mem)->x, reinterpret_cast<const Vector4i*>(_data._mem)->y, reinterpret_cast<const Vector4i*>(_data._mem)->z);
		}
		else {
			return Vector3();
		}
	}


	Variant::operator Vector4() const {
		if (type == VECTOR4) {
			return *reinterpret_cast<const Vector4*>(_data._mem);
		}
		else if (type == VECTOR4I) {
			return *reinterpret_cast<const Vector4i*>(_data._mem);
		}
		else if (type == VECTOR2) {
			return Vector4(reinterpret_cast<const Vector2*>(_data._mem)->x, reinterpret_cast<const Vector2*>(_data._mem)->y, 0.0, 0.0);
		}
		else if (type == VECTOR2I) {
			return Vector4(reinterpret_cast<const Vector2i*>(_data._mem)->x, reinterpret_cast<const Vector2i*>(_data._mem)->y, 0.0, 0.0);
		}
		else if (type == VECTOR3) {
			return Vector4(reinterpret_cast<const Vector3*>(_data._mem)->x, reinterpret_cast<const Vector3*>(_data._mem)->y, reinterpret_cast<const Vector3*>(_data._mem)->z, 0.0);
		}
		else if (type == VECTOR3I) {
			return Vector4(reinterpret_cast<const Vector3i*>(_data._mem)->x, reinterpret_cast<const Vector3i*>(_data._mem)->y, reinterpret_cast<const Vector3i*>(_data._mem)->z, 0.0);
		}
		else {
			return Vector4();
		}
	}


	Variant::operator Color() const {
		if (type == COLOR) {
			return *reinterpret_cast<const Color*>(_data._mem);
		}
		else if (type == STRING) {
			return Color(operator String());
		}
		else if (type == INT) {
			return Color::hex(operator int());
		}
		else {
			return Color();
		}
	}

	Variant::operator Quaternion() const {
		if (type == QUATERNION) {
			return *reinterpret_cast<const Quaternion*>(_data._mem);
		}
		/*else if (type == BASIS) {
			return *_data._basis;
		}
		else if (type == TRANSFORM3D) {
			return _data._transform3d->basis;
		}*/
		else {
			return Quaternion();
		}
	}

	Variant::operator Dictionary() const {
		if (type == DICTIONARY) {
			return *reinterpret_cast<const Dictionary*>(_data._mem);
		}
		else {
			return Dictionary();
		}
	}

	
	Variant::operator lain::RID() const {
		if (type == RID) {
			return *reinterpret_cast<const lain::RID*>(_data._mem);
		}
		else {
			return lain::RID();
		}
	}



	struct _VariantStrPair {
		String key;
		String value;

		bool operator<(const _VariantStrPair& p) const {
			return key < p.key;
		}
	};

	Variant::operator String() const {
		return stringify(0);
	}
	/// <summary>
	/// 处理Array
	/// </summary>
	/// <typeparam name="DA"></typeparam>
	/// <typeparam name="SA"></typeparam>
	/// <param name="p_array"></param>
	/// <returns></returns>
	template <class DA, class SA>
	inline DA _convert_array(const SA& p_array) {
		DA da;
		da.resize(p_array.size());

		for (int i = 0; i < p_array.size(); i++) {
			da.set(i, Variant(p_array.get(i)));
		}

		return da;
	}

	template <class DA>
	inline DA _convert_array_from_variant(const Variant& p_variant) {
		switch (p_variant.get_type()) {
		case Variant::ARRAY: {
			return _convert_array<DA, Array>(p_variant.operator Array());
		}
		case Variant::PACKED_BYTE_ARRAY: {
			return _convert_array<DA, PackedByteArray>(p_variant.operator PackedByteArray());
		}
		case Variant::PACKED_INT32_ARRAY: {
			return _convert_array<DA, PackedInt32Array>(p_variant.operator PackedInt32Array());
		}
		case Variant::PACKED_INT64_ARRAY: {
			return _convert_array<DA, PackedInt64Array>(p_variant.operator PackedInt64Array());
		}
		case Variant::PACKED_FLOAT32_ARRAY: {
			return _convert_array<DA, PackedFloat32Array>(p_variant.operator PackedFloat32Array());
		}
		case Variant::PACKED_FLOAT64_ARRAY: {
			return _convert_array<DA, PackedFloat64Array>(p_variant.operator PackedFloat64Array());
		}
		case Variant::PACKED_STRING_ARRAY: {
			return _convert_array<DA, PackedStringArray>(p_variant.operator PackedStringArray());
		}
		case Variant::PACKED_VECTOR2_ARRAY: {
			return _convert_array<DA, PackedVector2Array>(p_variant.operator PackedVector2Array());
		}
		case Variant::PACKED_VECTOR3_ARRAY: {
			return _convert_array<DA, PackedVector3Array>(p_variant.operator PackedVector3Array());
		}
		case Variant::PACKED_COLOR_ARRAY: {
			return _convert_array<DA, PackedColorArray>(p_variant.operator PackedColorArray());
		}
		default: {
			return DA();
		}
		}
	}



	Variant::operator Array() const {
		if (type == ARRAY) {
			return *reinterpret_cast<const Array*>(_data._mem);
		}
		else {
			return _convert_array_from_variant<Array>(*this);
		}
	}

	Variant::operator PackedByteArray() const {
		if (type == PACKED_BYTE_ARRAY) {
			return static_cast<PackedArrayRef<uint8_t> *>(_data.packed_array)->array;
		}
		else {
			return _convert_array_from_variant<PackedByteArray>(*this);
		}
	}

	Variant::operator PackedInt32Array() const {
		if (type == PACKED_INT32_ARRAY) {
			return static_cast<PackedArrayRef<int32_t> *>(_data.packed_array)->array;
		}
		else {
			return _convert_array_from_variant<PackedInt32Array>(*this);
		}
	}

	Variant::operator PackedInt64Array() const {
		if (type == PACKED_INT64_ARRAY) {
			return static_cast<PackedArrayRef<int64_t> *>(_data.packed_array)->array;
		}
		else {
			return _convert_array_from_variant<PackedInt64Array>(*this);
		}
	}

	Variant::operator PackedFloat32Array() const {
		if (type == PACKED_FLOAT32_ARRAY) {
			return static_cast<PackedArrayRef<float> *>(_data.packed_array)->array;
		}
		else {
			return _convert_array_from_variant<PackedFloat32Array>(*this);
		}
	}

	Variant::operator PackedFloat64Array() const {
		if (type == PACKED_FLOAT64_ARRAY) {
			return static_cast<PackedArrayRef<double> *>(_data.packed_array)->array;
		}
		else {
			return _convert_array_from_variant<PackedFloat64Array>(*this);
		}
	}

	Variant::operator PackedStringArray() const {
		if (type == PACKED_STRING_ARRAY) {
			return static_cast<PackedArrayRef<String> *>(_data.packed_array)->array;
		}
		else {
			return _convert_array_from_variant<PackedStringArray>(*this);
		}
	}

	Variant::operator PackedVector2Array() const {
		if (type == PACKED_VECTOR2_ARRAY) {
			return static_cast<PackedArrayRef<Vector2> *>(_data.packed_array)->array;
		}
		else {
			return _convert_array_from_variant<PackedVector2Array>(*this);
		}
	}

	Variant::operator PackedVector3Array() const {
		if (type == PACKED_VECTOR3_ARRAY) {
			return static_cast<PackedArrayRef<Vector3> *>(_data.packed_array)->array;
		}
		else {
			return _convert_array_from_variant<PackedVector3Array>(*this);
		}
	}

	Variant::operator PackedColorArray() const {
		if (type == PACKED_COLOR_ARRAY) {
			return static_cast<PackedArrayRef<Color> *>(_data.packed_array)->array;
		}
		else {
			return _convert_array_from_variant<PackedColorArray>(*this);
		}
	}


	void Variant::_clear_internal() {
		switch (type) {
		case STRING: {
			reinterpret_cast<String*>(_data._mem)->~String();
		} break;

			// Math types.
		/*case TRANSFORM2D: {
			if (_data._transform2d) {
				_data._transform2d->~Transform2D();
				Pools::_bucket_small.free((Pools::BucketSmall*)_data._transform2d);
				_data._transform2d = nullptr;
			}
		} break;
		case AABB: {
			if (_data._aabb) {
				_data._aabb->~AABB();
				Pools::_bucket_small.free((Pools::BucketSmall*)_data._aabb);
				_data._aabb = nullptr;
			}
		} break;
		case BASIS: {
			if (_data._basis) {
				_data._basis->~Basis();
				Pools::_bucket_medium.free((Pools::BucketMedium*)_data._basis);
				_data._basis = nullptr;
			}
		} break;
		case TRANSFORM3D: {
			if (_data._transform3d) {
				_data._transform3d->~Transform3D();
				Pools::_bucket_medium.free((Pools::BucketMedium*)_data._transform3d);
				_data._transform3d = nullptr;
			}
		} break;
		case PROJECTION: {
			if (_data._projection) {
				_data._projection->~Projection();
				Pools::_bucket_large.free((Pools::BucketLarge*)_data._projection);
				_data._projection = nullptr;
			}
		} break;*/

			// Miscellaneous types.
		case STRING_NAME: {
			reinterpret_cast<StringName*>(_data._mem)->~StringName();
		} break;
	/*	case GOBJECT_PATH: {
			reinterpret_cast<GObjectPath*>(_data._mem)->~GObjectPath();
		} break;*/
		case OBJECT: {
			if (_get_obj().id.is_ref_counted()) {
				// We are safe that there is a reference here.
				RefCounted* ref_counted = static_cast<RefCounted*>(_get_obj().obj);
				if (ref_counted->unreference()) {
					memdelete(ref_counted);
				}
			}
			_get_obj().obj = nullptr;
			_get_obj().id = ObjectID();
		} break;
		case RID: {
			// Not much need probably.
			// HACK: Can't seem to use destructor + scoping operator, so hack.
			typedef lain::RID RID_Class;
			reinterpret_cast<RID_Class*>(_data._mem)->~RID_Class();
		} break;
		case CALLABLE: {
			reinterpret_cast<Callable*>(_data._mem)->~Callable();
		} break;
		case SIGNAL: {
			reinterpret_cast<Signal*>(_data._mem)->~Signal();
		} break;
		/*case DICTIONARY: {
			reinterpret_cast<Dictionary*>(_data._mem)->~Dictionary();
		} break;*/
		case ARRAY: {
			reinterpret_cast<Array*>(_data._mem)->~Array();
		} break;

			// Arrays.
		case PACKED_BYTE_ARRAY: {
			PackedArrayRefBase::destroy(_data.packed_array);
		} break;
		case PACKED_INT32_ARRAY: {
			PackedArrayRefBase::destroy(_data.packed_array);
		} break;
		case PACKED_INT64_ARRAY: {
			PackedArrayRefBase::destroy(_data.packed_array);
		} break;
		case PACKED_FLOAT32_ARRAY: {
			PackedArrayRefBase::destroy(_data.packed_array);
		} break;
		case PACKED_FLOAT64_ARRAY: {
			PackedArrayRefBase::destroy(_data.packed_array);
		} break;
		case PACKED_STRING_ARRAY: {
			PackedArrayRefBase::destroy(_data.packed_array);
		} break;
		case PACKED_VECTOR2_ARRAY: {
			PackedArrayRefBase::destroy(_data.packed_array);
		} break;
		case PACKED_VECTOR3_ARRAY: {
			PackedArrayRefBase::destroy(_data.packed_array);
		} break;
		case PACKED_COLOR_ARRAY: {
			PackedArrayRefBase::destroy(_data.packed_array);
		} break;
		default: {
			// Not needed, there is no point. The following do not allocate memory:
			// VECTOR2, VECTOR3, RECT2, PLANE, QUATERNION, COLOR.
		}
		}
	}
	bool Variant::can_convert_strict(Variant::Type p_type_from, Variant::Type p_type_to) {
		if (p_type_from == p_type_to) {
			return true;
		}
		if (p_type_to == NIL) { //nil can convert to anything
			return true;
		}

		if (p_type_from == NIL) {
			return (p_type_to == OBJECT);
		}

		const Type* valid_types = nullptr;

		switch (p_type_to) {
		case BOOL: {
			static const Type valid[] = {
				INT,
				FLOAT,
				//STRING,
				NIL,
			};

			valid_types = valid;
		} break;
		case INT: {
			static const Type valid[] = {
				BOOL,
				FLOAT,
				//STRING,
				NIL,
			};

			valid_types = valid;

		} break;
		case FLOAT: {
			static const Type valid[] = {
				BOOL,
				INT,
				//STRING,
				NIL,
			};

			valid_types = valid;

		} break;
		case STRING: {
			static const Type valid[] = {
				GOBJECT_PATH,
				STRING_NAME,
				NIL
			};

			valid_types = valid;
		} break;
		case VECTOR2: {
			static const Type valid[] = {
				VECTOR2I,
				NIL,
			};

			valid_types = valid;

		} break;
		case VECTOR2I: {
			static const Type valid[] = {
				VECTOR2,
				NIL,
			};

			valid_types = valid;

		} break;
		case RECT2: {
			static const Type valid[] = {
				RECT2I,
				NIL,
			};

			valid_types = valid;

		} break;
		case RECT2I: {
			static const Type valid[] = {
				RECT2,
				NIL,
			};

			valid_types = valid;

		} break;
		case TRANSFORM2D: {
			static const Type valid[] = {
				TRANSFORM3D,
				NIL
			};

			valid_types = valid;
		} break;
		case VECTOR3: {
			static const Type valid[] = {
				VECTOR3I,
				NIL,
			};

			valid_types = valid;

		} break;
		case VECTOR3I: {
			static const Type valid[] = {
				VECTOR3,
				NIL,
			};

			valid_types = valid;

		} break;
		case VECTOR4: {
			static const Type valid[] = {
				VECTOR4I,
				NIL,
			};

			valid_types = valid;

		} break;
		case VECTOR4I: {
			static const Type valid[] = {
				VECTOR4,
				NIL,
			};

			valid_types = valid;

		} break;

		case QUATERNION: {
			static const Type valid[] = {
				BASIS,
				NIL
			};

			valid_types = valid;

		} break;
		case BASIS: {
			static const Type valid[] = {
				QUATERNION,
				NIL
			};

			valid_types = valid;

		} break;
		case TRANSFORM3D: {
			static const Type valid[] = {
				TRANSFORM2D,
				QUATERNION,
				BASIS,
				PROJECTION,
				NIL
			};

			valid_types = valid;

		} break;
		case PROJECTION: {
			static const Type valid[] = {
				TRANSFORM3D,
				NIL
			};

			valid_types = valid;

		} break;

		case COLOR: {
			static const Type valid[] = {
				STRING,
				INT,
				NIL,
			};

			valid_types = valid;

		} break;

		case RID: {
			static const Type valid[] = {
				OBJECT,
				NIL
			};

			valid_types = valid;
		} break;
		case OBJECT: {
			static const Type valid[] = {
				NIL
			};

			valid_types = valid;
		} break;
		case STRING_NAME: {
			static const Type valid[] = {
				STRING,
				NIL
			};

			valid_types = valid;
		} break;
		case GOBJECT_PATH: {
			static const Type valid[] = {
				STRING,
				NIL
			};

			valid_types = valid;
		} break;
		case ARRAY: {
			static const Type valid[] = {
				PACKED_BYTE_ARRAY,
				PACKED_INT32_ARRAY,
				PACKED_INT64_ARRAY,
				PACKED_FLOAT32_ARRAY,
				PACKED_FLOAT64_ARRAY,
				PACKED_STRING_ARRAY,
				PACKED_COLOR_ARRAY,
				PACKED_VECTOR2_ARRAY,
				PACKED_VECTOR3_ARRAY,
				NIL
			};

			valid_types = valid;
		} break;
			// arrays
		case PACKED_BYTE_ARRAY: {
			static const Type valid[] = {
				ARRAY,
				NIL
			};

			valid_types = valid;
		} break;
		case PACKED_INT32_ARRAY: {
			static const Type valid[] = {
				ARRAY,
				NIL
			};
			valid_types = valid;
		} break;
		case PACKED_INT64_ARRAY: {
			static const Type valid[] = {
				ARRAY,
				NIL
			};
			valid_types = valid;
		} break;
		case PACKED_FLOAT32_ARRAY: {
			static const Type valid[] = {
				ARRAY,
				NIL
			};

			valid_types = valid;
		} break;
		case PACKED_FLOAT64_ARRAY: {
			static const Type valid[] = {
				ARRAY,
				NIL
			};

			valid_types = valid;
		} break;
		case PACKED_STRING_ARRAY: {
			static const Type valid[] = {
				ARRAY,
				NIL
			};
			valid_types = valid;
		} break;
		case PACKED_VECTOR2_ARRAY: {
			static const Type valid[] = {
				ARRAY,
				NIL
			};
			valid_types = valid;

		} break;
		case PACKED_VECTOR3_ARRAY: {
			static const Type valid[] = {
				ARRAY,
				NIL
			};
			valid_types = valid;

		} break;
		case PACKED_COLOR_ARRAY: {
			static const Type valid[] = {
				ARRAY,
				NIL
			};

			valid_types = valid;

		} break;
		default: {
		}
		}

		if (valid_types) {
			int i = 0;
			while (valid_types[i] != NIL) {
				if (p_type_from == valid_types[i]) {
					return true;
				}
				i++;
			}
		}

		return false;
	}

	String Variant::stringify_variant_clean(const Variant& p_variant, int recursion_count) const {
		String s = p_variant.stringify(recursion_count);

		// Wrap strings in quotes to avoid ambiguity.
		switch (p_variant.get_type()) {
		case Variant::STRING: {
			s = s.c_escape().quote();
		} break;
		case Variant::STRING_NAME: {
			s = "&" + s.c_escape().quote();
		} break;
		case Variant::GOBJECT_PATH: {
			s = "^" + s.c_escape().quote();
		} break;
		default: {
		} break;
		}

		return s;
	}

	template <typename T>
	String Variant::stringify_vector(const T& vec, int recursion_count) const {
		String str("[");
		for (int i = 0; i < vec.size(); i++) {
			if (i > 0) {
				str += ", ";
			}

			str += stringify_variant_clean(vec[i], recursion_count);
		}
		str += "]";
		return str;
	}

}