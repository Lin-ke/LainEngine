#pragma once
#ifndef SERIALIZER_HELPER_H
#define SERIALIZER_HELPER_H
#include "core/meta/reflection/reflection.h"
#define FIELDOPERATOR(classname)	\
namespace Reflection {				\
		namespace TypeFieldReflectionOperator {		\
			class Type##classname##Operator {		\
		public:						\
		static const char* getClassName() { return #classname; }\
																\
		static void* constructorWithJson(const Json& json_context) {	\
		classname* ret_instance = memnew(classname);					\
		Serializer::read(json_context, *ret_instance);					\
		return ret_instance;											\
				}														\
		\
		static Json writeByName(void* instance) {\
			return Serializer::write(*(classname*)instance);\
				}\
			\
		}; \
		}\


namespace lain {
	//FIELDOPERATOR(StringName);
}

#endif
