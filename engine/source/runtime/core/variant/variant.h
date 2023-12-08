#pragma once
#ifndef __VARIANT_H__
#define __VARIANT_H__
#include <stdint.h>
#include "core/math/math_defs.h"
#include "core/object/object_id.h"
#include "core/resource/rid.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include "core/math/color.h"
#include "core/object/safe_refcount.h"
namespace lain {
	typedef Vector<uint8_t> PackedByteArray;
	typedef Vector<int32_t> PackedInt32Array;
	typedef Vector<int64_t> PackedInt64Array;
	typedef Vector<float> PackedFloat32Array;
	typedef Vector<double> PackedFloat64Array;
	typedef Vector<String> PackedStringArray;
	typedef Vector<Vector2> PackedVector2Array;
	typedef Vector<Vector3> PackedVector3Array;
	typedef Vector<Color> PackedColorArray;
	class Object;
	class ConfigParser;
	class Variant {
	public:
		enum Type
		{
			NIL,
			// atomic types
			// Packing and unpacking
			BOOL,
			INT,
			FLOAT,
			STRING,

			// using pointer
			VECTOR2,
			VECTOR2I,
			RECT2, 
			RECT2I,
			VECTOR3,
			VECTOR3I,

			VECTOR4,
			VECTOR4I,
			PLANE,
			AABB,
			QUATERNION,

			TRANSFORM2D,
			TRANSFORM3D,
			PROJECTION,

			// engine
			OBJECT,
			CALLABLE,
			RID,

			
			// arrays
			VECTOR,
			PACKED_STRING_ARRAY
		};
	private:

		Type type = NIL;

		/// <summary>
		/// array helpers
		/// </summary>
		struct PackedArrayRefBase {
			SafeRefCount refcount;
			_FORCE_INLINE_ PackedArrayRefBase* reference() {
				if (this->refcount.ref()) {
					return this;
				}
				else {
					return nullptr;
				}
			}
			static _FORCE_INLINE_ PackedArrayRefBase* reference_from(PackedArrayRefBase* p_base, PackedArrayRefBase* p_from) {
				if (p_base == p_from) {
					return p_base; //same thing, do nothing
				}

				if (p_from->reference()) {
					if (p_base->refcount.unref()) {
						memdelete(p_base);
					}
					return p_from;
				}
				else {
					return p_base; //keep, could not reference new
				}
			}
			static _FORCE_INLINE_ void destroy(PackedArrayRefBase* p_array) {
				if (p_array->refcount.unref()) {
					memdelete(p_array);
				}
			}
			_FORCE_INLINE_ virtual ~PackedArrayRefBase() {} //needs virtual destructor, but make inline
		};
		template <class T>
		struct PackedArrayRef : public PackedArrayRefBase {
			Vector<T> array;
			static _FORCE_INLINE_ PackedArrayRef<T>* create() {
				return memnew(PackedArrayRef<T>);
			}
			static _FORCE_INLINE_ PackedArrayRef<T>* create(const Vector<T>& p_from) {
				return memnew(PackedArrayRef<T>(p_from));
			}

			static _FORCE_INLINE_ const Vector<T>& get_array(PackedArrayRefBase* p_base) {
				return static_cast<PackedArrayRef<T> *>(p_base)->array;
			}
			static _FORCE_INLINE_ Vector<T>* get_array_ptr(const PackedArrayRefBase* p_base) {
				return &const_cast<PackedArrayRef<T> *>(static_cast<const PackedArrayRef<T> *>(p_base))->array;
			}

			_FORCE_INLINE_ PackedArrayRef(const Vector<T>& p_from) {
				array = p_from;
				refcount.init();
			}
			_FORCE_INLINE_ PackedArrayRef() {
				refcount.init();
			}
		};

		struct ObjData {
			ObjectID id;
			Object* obj = nullptr;
		};
		union {
			bool _bool;
			int64_t _int;
			double _float;
			void* _ptr; //generic pointer
			PackedArrayRefBase* packed_array;
			uint8_t _mem[sizeof(ObjData) > (sizeof(real_t) * 4) ? sizeof(ObjData) : (sizeof(real_t) * 4)]{ 0 };
		} _data alignas(8);


		// constructor
		Variant(const Variant*);
		Variant(const Variant**);
	public:
		_FORCE_INLINE_ Type get_type() const {
			return type;
		}
		static String get_type_name(Variant::Type p_type);
		L_INLINE String get_type_name() {
			return get_type_name(type);
		}
		void operator=(const Variant& p_variant); // only this is enough for all the other types
		typedef void (*ObjectConstruct)(const String& p_text, void* ud, Variant& r_value);
		static void construct_from_string(const String& p_string, Variant& r_value, ObjectConstruct p_obj_construct = nullptr, void* p_construct_ud = nullptr);
		u32 recursive_hash(int recursion_count) const;
		uint32_t Variant::hash() const {
			return recursive_hash(0);
		}


		///constructors
		Variant(const Vector<String>& p_string_array);
		Variant(const String& p_string);
		Variant(const StringName& p_string);
		Variant(const char* const p_cstring);
		Variant(int64_t p_int); // real one
		Variant(uint64_t p_int);
		Variant(float p_float);
		Variant(double p_double);
		Variant() { type = NIL; }
	};

	template <typename... VarArgs>
	String vformat(const String& p_text, const VarArgs... p_args) {
		return "";
	}
	//String vformat(const String& p_text, const VarArgs... p_args) {
	//	Variant args[sizeof...(p_args) + 1] = { p_args..., Variant() }; // +1 makes sure zero sized arrays are also supported.
	//	Array args_array;
	//	args_array.resize(sizeof...(p_args));
	//	for (uint32_t i = 0; i < sizeof...(p_args); i++) {
	//		args_array[i] = args[i];
	//	}

	//	bool error = false;
	//	String fmt = p_text.sprintf(args_array, &error);

	//	ERR_FAIL_COND_V_MSG(error, String(), fmt);

	//	return fmt;
	//}
};
#endif // !__VARIANT_H__
