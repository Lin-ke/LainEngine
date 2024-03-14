#pragma once
#ifndef REGISTER_OTHER_TYPES_H
#define REGISTER_OTHER_TYPES_H

#include "core/meta/others/stringname.reflection.h"
namespace lain {


	L_INLINE void register_other_meta() {
		Reflection::TypeWrappersRegister::StringName();
	}

	//namespace Reflection {
	//	namespace TypeFieldReflectionOparator {
	//		class TypeStringOperator {
	//		
	//		
	//		public:
	//			static const char* getClassName() { return "String"; }
	//			static void* constructorWithJson(const Json& json_context) {
	//				String* ret_instance = memnew(String);
	//				Serializer::read(json_context, *ret_instance);
	//				return ret_instance;
	//			}
	//			static Json writeByName(void* instance) {
	//				return Serializer::write(*(String*)instance);
	//			}
	//			// base class
	//			static int getStringBaseClassReflectionInstanceList(ReflectionInstance*& out_list, void* instance) {
	//				int count = 0;

	//				return count;
	//			}
	//			// fields
	//			static const char* getFieldName_data() { return "data"; }
	//			static const char* getFieldTypeName_data() { return "String"; }
	//			static void set_data(void* instance, String* field_value) { *static_cast<String*>(instance) = *field_value; }
	//			static void set_data(void* instance, char* field_value) { (*static_cast<String*>(instance)).copy_from(field_value); }
	//			// data就是他自己
	//			static void* get_data(void* instance) { return instance; }
	//			static bool isArray_data() { return false; }
	//		};
	//	}

	//}


}

#endif // !REGISTER_OTHER_TYPES_H
