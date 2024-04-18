#pragma once
#ifndef __VARIANT_H__
#define __VARIANT_H__
#include "core/variant/variant_header.h"

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
	class ConfigFile;
	class VariantInternal;
	class RID;
	class Serializer;
	// variant的实现和lua是一样的，但是多一些类
	class Variant {
		friend class VariantInternal;
		friend class Serializer;
	public:
		enum Type
		{
			NIL,

			// atomic types
			BOOL,
			INT,
			FLOAT,
			STRING,

			// math types
			VECTOR2,
			VECTOR2I,
			RECT2,
			RECT2I,
			VECTOR3,
			VECTOR3I,
			TRANSFORM2D,
			VECTOR4,
			VECTOR4I,
			PLANE,
			QUATERNION,
			AABB,
			BASIS,
			TRANSFORM3D,
			PROJECTION,

			// misc types
			COLOR,
			STRING_NAME,
			GOBJECT_PATH,
			RID,
			OBJECT,
			CALLABLE,
			SIGNAL,
			DICTIONARY,
			ARRAY,

			// typed arrays
			PACKED_BYTE_ARRAY,
			PACKED_INT32_ARRAY,
			PACKED_INT64_ARRAY,
			PACKED_FLOAT32_ARRAY,
			PACKED_FLOAT64_ARRAY,
			PACKED_STRING_ARRAY,
			PACKED_VECTOR2_ARRAY,
			PACKED_VECTOR3_ARRAY,
			PACKED_COLOR_ARRAY,

			// reflect
			REFLECTIONINSTANCE,
			VARIANT_MAX,



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
			lain::AABB* _aabb;

			PackedArrayRefBase* packed_array;
			uint8_t _mem[sizeof(ObjData) > (sizeof(real_t) * 4) ? sizeof(ObjData) : (sizeof(real_t) * 4)]{ 0 };
		} _data alignas(8);
		const ObjData& _get_obj() const;
		ObjData& _get_obj();
		// constructor
		Variant(const Variant*);
		Variant(const Variant**);


	public:
		String stringify(int recursion_count) const;
		_FORCE_INLINE_ Type get_type() const {
			return type;
		}
		static const char* get_c_type_name(Variant::Type p_type);
		static String get_type_name(Variant::Type p_type) { return get_c_type_name(p_type); }
		L_INLINE String get_type_name() {
			return get_type_name(type);
		}
		void reference(const Variant& p_variant);
		void operator=(const Variant& p_variant); // only this is enough for all the other 

		bool operator==(const Variant& p_variant) const;

		typedef void (*ObjectConstruct)(const String& p_text, void* ud, Variant& r_value);
		static void construct_from_string(const String& p_string, Variant& r_value, ObjectConstruct p_obj_construct = nullptr, void* p_construct_ud = nullptr);

		ui32 recursive_hash(int recursion_count) const;
		bool hash_compare(const Variant& p_variant, int recursion_count = 0, bool semantic_comparison = true) const;

		uint32_t Variant::hash() const {
			return recursive_hash(0);
		}

		void zero();
		Variant duplicate(bool p_deep = false) const;
		Variant recursive_duplicate(bool p_deep, int recursion_count) const;
		void set_type(Type p_type) { type = p_type; };
		// 装箱
		// containers
		/// Variant transform
		Variant(const Array& p_array);
		Variant(const Vector<String>& p_string_array);
		Variant(const Vector<float>& p_float_array);
		Variant(const Vector<int64_t>& p_int64_array);
		Variant(const Vector<int32_t>& p_int32_array);
		Variant(const Vector<double>& p_double_array);
		Variant(const Vector<ui8>& p_ui8_array);

		Variant(const Dictionary& p_dictionary);

		// object
		Variant(const Object* p_obj);
		Variant(const Reflection::ReflectionInstance& p_instance);

		// basics
		Variant(int64_t p_int); // real one
		Variant(uint64_t p_int);
		Variant(float p_float);
		Variant(double p_double);
		Variant(bool p_bool);
		Variant(signed int p_int); // real one
		Variant(unsigned int p_int);
		// array


		//other class
		Variant(const Vector2& p_vector2);
		Variant(const Vector3& p_vector3);
		Variant(const String& p_string);
		Variant(const StringName& p_string);
		Variant(const GObjectPath& p_string);

		Variant(const char* const p_cstring);
		Variant(const Color& p_cstring);

		// copy construct
		Variant(const Variant& p_variant);


		Variant() { type = NIL; }

		// 拆箱
		operator bool() const;
		operator signed int() const;
		operator unsigned int() const; // this is the real one
		operator signed short() const;
		operator unsigned short() const;
		operator signed char() const;
		operator unsigned char() const;
		//operator long unsigned int() const;
		operator int64_t() const;
		operator uint64_t() const;
#ifdef NEED_LONG_INT
		operator signed long() const;
		operator unsigned long() const;
#endif

		operator ObjectID() const;

		operator char32_t() const;
		operator float() const;
		operator double() const;
		operator String() const;
		operator StringName() const;
		operator Vector2() const;
		operator Vector2i() const;
		operator Rect2() const;
		operator Rect2i() const;
		operator Vector3() const;
		operator Vector3i() const;
		operator Vector4() const;
		/*operator Vector4i() const;
		operator Plane() const;
		operator ::AABB() const;*/
		operator Quaternion() const;
		/*operator Basis() const;
		operator Transform2D() const;
		operator Transform3D() const;
		operator Projection() const;*/

		operator Color() const;
		operator GObjectPath() const;
		operator lain::RID() const;

		operator Object* () const;

		operator Callable() const;
		operator Signal() const;

		operator Dictionary() const;
		operator Array() const;

		operator PackedByteArray() const;
		operator PackedInt32Array() const;
		operator PackedInt64Array() const;
		operator PackedFloat32Array() const;
		operator PackedFloat64Array() const;
		operator PackedStringArray() const;
		operator PackedVector3Array() const;
		operator PackedVector2Array() const;
		operator PackedColorArray() const;

		/*operator Vector<::RID>() const;
		operator Vector<Plane>() const;
		operator Vector<Face3>() const;*/
		operator Vector<Variant>() const;
		operator Vector<StringName>() const;

		// some core type enums to convert to
		operator Side() const;
		operator Orientation() const;

		//operator IPAddress() const;

		Object* get_validated_object() const;

		~Variant() { clear(); }
		_FORCE_INLINE_ void clear() {
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
				true, //GOBJECT_PATH,
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
				false, //ReflecionInstance
			};

			if (unlikely(needs_deinit[type])) { // Make it fast for types that don't need deinit.
				_clear_internal();
			}
			type = NIL;
		}

		void _clear_internal();
		// convert function, type hint
		static bool can_convert_strict(Type from, Type to);
		bool is_ref_counted() const;
		_FORCE_INLINE_ bool is_num() const {
			return type == INT || type == FLOAT;
		}
		_FORCE_INLINE_ bool is_string() const {
			return type == STRING || type == STRING_NAME;
		}
		_FORCE_INLINE_ bool is_array() const {
			return type >= ARRAY;
		}
		bool is_shared() const;
		bool is_zero() const;
		bool is_one() const;
		bool is_null() const;
		
	/// <summary>
	/// variant operator
	/// </summary>
	public:
		enum Operator {
			//comparison
			OP_EQUAL,
			OP_NOT_EQUAL,
			OP_LESS,
			OP_LESS_EQUAL,
			OP_GREATER,
			OP_GREATER_EQUAL,
			//mathematic
			OP_ADD,
			OP_SUBTRACT,
			OP_MULTIPLY,
			OP_DIVIDE,
			OP_NEGATE,
			OP_POSITIVE,
			OP_MODULE,
			OP_POWER,
			//bitwise
			OP_SHIFT_LEFT,
			OP_SHIFT_RIGHT,
			OP_BIT_AND,
			OP_BIT_OR,
			OP_BIT_XOR,
			OP_BIT_NEGATE,
			//logic
			OP_AND,
			OP_OR,
			OP_XOR,
			OP_NOT,
			//containment
			OP_IN,
			OP_MAX

		};
		typedef void (*VariantEvaluatorFunction)(const Variant& p_left, const Variant& p_right, Variant* r_ret, bool& r_valid);
		static VariantEvaluatorFunction operator_evaluator_table[Variant::OP_MAX][Variant::VARIANT_MAX][Variant::VARIANT_MAX];

		static String get_operator_name(Operator p_op);
		static void evaluate(const Operator& p_op, const Variant& p_a, const Variant& p_b, Variant& r_ret, bool& r_valid);
		static _FORCE_INLINE_ Variant evaluate(const Operator& p_op, const Variant& p_a, const Variant& p_b) {
			bool valid = true;
			Variant res;
			evaluate(p_op, p_a, p_b, res, valid);
			return res;
		}


	};


	/// <summary>
	/// helper functions
	/// </summary>

	template <typename... VarArgs>
	String vformat(const String& p_text, const VarArgs... p_args) {
		Variant args[sizeof...(p_args) + 1] = { p_args..., Variant() }; // +1 makes sure zero sized arrays are also supported.
		Array args_array;
		args_array.resize(sizeof...(p_args));
		for (uint32_t i = 0; i < sizeof...(p_args); i++) {
			args_array[i] = args[i];
		}

		bool error = false;
		String fmt = p_text.sprintf(args_array, &error);

		ERR_FAIL_COND_V_MSG(error, String(), fmt);

		return fmt;
	}
	struct VariantHasher {
		static _FORCE_INLINE_ uint32_t hash(const Variant& p_variant) { return p_variant.hash(); }
	};
	struct StringLikeVariantComparator {
		static bool compare(const Variant& p_lhs, const Variant& p_rhs);
	};

	struct VariantComparator {
		static _FORCE_INLINE_ bool compare(const Variant& p_lhs, const Variant& p_rhs) { return p_lhs.hash_compare(p_rhs); }
	};

	// string helper function
	template <typename T>
	String stringify_vector(const T& vec, int recursion_count);
	String stringify_variant_clean(const Variant& p_variant, int recursion_count);
};

#endif // !__VARIANT_H__
