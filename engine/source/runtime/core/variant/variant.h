#pragma once
#ifndef __VARIANT_H__
#define __VARIANT_H__
#include <stdint.h>
#include "core/math/math_defs.h"
#include "core/object/object_id.h"
#include "core/resource/rid.h"
#include "core/string/ustring.h"
namespace lain {
	class Object;
	class Variant {
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
			VECTOR // is it possiable?

		};
		Type m_type;
		struct ObjData {
			ObjectID id;
			Object* obj = nullptr;
		};
		union {
			bool _bool;
			int64_t _int;
			double _float;
			void* _ptr; //generic pointer
			uint8_t _mem[sizeof(ObjData) > (sizeof(real_t) * 4) ? sizeof(ObjData) : (sizeof(real_t) * 4)]{ 0 };
		} m_data alignas(8);


		// constructor
		Variant(const Variant*) {}
		Variant(const Variant**) {}
	public:
		_FORCE_INLINE_ Type get_type() const {
			return m_type;
		}
		static String get_type_name(Variant::Type p_type);
		L_INLINE String get_type_name() {
			return get_type_name(m_type);
		}
		void operator=(const Variant& p_variant); // only this is enough for all the other types

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
