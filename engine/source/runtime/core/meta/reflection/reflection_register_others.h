#pragma once
#ifndef REGISTER_OTHER_TYPES_H
#define REGISTER_OTHER_TYPES_H
#define SERIAL_REG(class) Reflection::TypeWrappersRegister::ClassFunction_Manual_##class##();
#define REFLECT_REG(class) Reflection::TypeWrappersRegister::##class##();


namespace lain {
	// 这里手动写一些reflection的注册
	// 就类似于godot 的 variant 巴拉巴拉


	// MANUAL_REFLECTION_TYPE_SAFETY(StringName)
	// static void stringname_set_name(void* p_stringname, void* p_str) {
	// 	*reinterpret_cast<StringName*>(p_stringname) = *reinterpret_cast<StringName*>(p_str);
	// }
	// static void* stringname_get_name(void* instance) {
	// 	return 
	//  }
	// static void stringname_set_static(void* p_stringname, void* p_staic) {
	// 	// todo 
	// }
	// static void* stringname_set_static(void* instance) { return nullptr; }
	// REFLECT_FIELD_DEF_SAFETY(StringName, data, String, stringname_set_name, stringname_get_name, false);
	// MANUAL_REFLECT_FIELD_SAFETY(StringName, data) 
	// Name, setfunction, getfunction (name), is array bool

	


	void register_other_meta();

}
#undef Seriali
#undef Reflect
#endif // !REGISTER_OTHER_TYPES_H
