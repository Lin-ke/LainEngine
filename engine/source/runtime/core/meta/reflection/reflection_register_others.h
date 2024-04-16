#pragma once
#ifndef REGISTER_OTHER_TYPES_H
#define REGISTER_OTHER_TYPES_H
#define Seriali(class) Reflection::TypeWrappersRegister::ClassFunction_Manual_##class##();
#define Reflect(class) Reflection::TypeWrappersRegister::##class##();

#include "core/meta/reflection/reflection.h"
#include "core/meta/serializer/serializer.h"
namespace lain {
	// serializer随处可以定义，所以使用这个

	SERIALIZER(Dictionary); 
	SERIALIZER(StringName);
	SERIALIZER(GObjectPath);
	SERIALIZER(String);

	MANUAL_REFLECTION_TYPE_SAFETY(StringName)
	static void stringname_set_name(void* p_stringname, void* p_str) {
		*reinterpret_cast<StringName*>(p_stringname) = *reinterpret_cast<StringName*>(p_str);
	}
	static void* stringname_get_name(void* instance) { return (void*)&static_cast<StringName*>(instance)->_data->name; }
	static void stringname_set_static(void* p_stringname, void* p_staic) {
		// todo 
	}
	static void* stringname_set_static(void* instance) { return nullptr; }
	REFLECT_FIELD_DEF_SAFETY(StringName, data, String, stringname_set_name, stringname_get_name, false);
	MANUAL_REFLECT_FIELD_SAFETY(StringName, data) 
	// Name, setfunction, getfunction (name), is array bool



	L_INLINE void register_other_meta() {
		Reflect(StringName);
		Seriali(Dictionary);
		Seriali(StringName);
		Seriali(GObjectPath);
		Seriali(String);


		// dictionary

	}


}
#undef Seriali
#undef Reflect
#endif // !REGISTER_OTHER_TYPES_H
