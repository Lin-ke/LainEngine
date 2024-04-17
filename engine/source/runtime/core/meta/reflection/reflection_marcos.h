#pragma once
#ifndef REFELCTION_MACROS_H
#define REFELCTION_MACROS_H

#define REFLECTABLE_TYPE(class_name)\
    friend class Serializer;



#if defined(__REFLECTION_PARSER__)
#define META(...) __attribute__((annotate(#__VA_ARGS__)))
#define CLASS(class_name, ...) class __attribute__((annotate(#__VA_ARGS__))) class_name
#define STRUCT(struct_name, ...) struct __attribute__((annotate(#__VA_ARGS__))) struct_name
//#define CLASS(class_name,...) class __attribute__((annotate(#__VA_ARGS__))) class_name:public Reflection::object
#else
#define META(...)
#define CLASS(class_name, ...) class class_name
#define STRUCT(struct_name, ...) struct struct_name
//#define CLASS(class_name,...) class class_name:public Reflection::object
#endif // __REFLECTION_PARSER__

#define REFLECTION_BODY(class_name) \
    friend class Reflection::TypeFieldReflectionOperator::Type##class_name##Operator; \
    friend class Serializer;                

#define MANUAL_REFLECTION_BODY(class_name) \
    friend class Serializer;                \

    // public: virtual std::string getTypeName() override {return #class_name;}

#define REFLECTION_TYPE(class_name) \
    namespace Reflection \
    { \
        namespace TypeFieldReflectionOperator \
        { \
            class Type##class_name##Operator; \
        } \
    };

#define MANUAL_REFLECTION_TYPE(class_name) \
    namespace Reflection \
    { \
        namespace TypeFieldReflectionOperator \
        {       \
            namespace Type##class_name##Operator    \
                { \
                 const char* getClassName() \
                     { return #class_name; }    \
                }}}

#define MANUAL_REFLECTION_TYPE_SAFETY(class_name) \
    namespace Reflection \
    { \
        namespace TypeFieldReflectionOperator \
        {       \
            class Type##class_name##Operator    \
                { \
                    public: \
                 static const char* getClassName() \
                     { return #class_name; }    \

#define MANUAL_REFLECTION_NAMESPACE(class_name)\
        namespace Reflection \
    {   \
        namespace TypeFieldReflectionOperator \
        {   



#define REGISTER_FIELD_TO_MAP(name, value) TypeMetaRegisterinterface::registerToFieldMap(name, value);
#define REGISTER_Method_TO_MAP(name, value) TypeMetaRegisterinterface::registerToMethodMap(name, value);
#define REGISTER_BASE_CLASS_TO_MAP(name, value) TypeMetaRegisterinterface::registerToClassMap(name, value);
#define REGISTER_SERIAL_TO_MAP(name, value) TypeMetaRegisterinterface::registerToSerialMap(name, value);
#define REGISTER_ARRAY_TO_MAP(name, value) TypeMetaRegisterinterface::registerToArrayMap(name, value);
#define UNREGISTER_ALL TypeMetaRegisterinterface::unregisterAll();

#define Lain_REFLECTION_NEW(name, ...) Reflection::ReflectionPtr(#name, new name(__VA_ARGS__));
#define Lain_REFLECTION_DELETE(value) \
    if (value) \
    { \
        delete value.operator->(); \
        value.getPtrReference() = nullptr; \
    }
#define Lain_REFLECTION_DEEP_COPY(type, dst_ptr, src_ptr) \
    *static_cast<type*>(dst_ptr) = *static_cast<type*>(src_ptr.getPtr());
#define SERIALIZER(CLASS)       \
    namespace Reflection {      \
    namespace TypeFieldReflectionOperator {\
    class Type##CLASS##SerilizerWrapper {           \
    public : static const char* getClassName() { return #CLASS; } \
                                                           \
        static void* constructorWithJson(const Json& json_context) { \
            CLASS* ret_instance = memnew(CLASS); \
            Serializer::read(json_context, *ret_instance); \
            return ret_instance;                            \
         }                                                 \
                                                           \
        static Json writeByName(void* instance) {       \
                                                         \
            return Serializer::write(*(CLASS*)instance); \
    }                                                   \
    static void json_read_warpper(const Json& json, void* instance) {       \
                                                     \
           Serializer::read(json, *reinterpret_cast<CLASS*>(instance));      \
    }                                                   \
static size_t get_size_of(){ return sizeof(CLASS);}            \
static void* memnew_class(void* target = nullptr) { if(target != nullptr) { \
    memnew_placement(target, {{class_name}}); return target}\
    return memnew(CLASS); }             \
static void* memnew_class_arr(int size_of_arr) { return memnew_arr(CLASS, size_of_arr); }\
};                                          }   \
                                                    \
  namespace TypeWrappersRegister{                                                      \
 void ClassFunction_Manual_##CLASS##(){       \
   ClassFunctionTuple* class_function_tuple_##CLASS##=memnew(ClassFunctionTuple(\
        nullptr,                                                                \
        &TypeFieldReflectionOperator::Type##CLASS##SerilizerWrapper::constructorWithJson,              \
        &TypeFieldReflectionOperator::Type##CLASS##SerilizerWrapper::writeByName,           \
        &TypeFieldReflectionOperator::Type##CLASS##SerilizerWrapper::json_read_warpper,     \
& TypeFieldReflectionOperator::Type##CLASS##SerilizerWrapper::memnew_class,             \
& TypeFieldReflectionOperator::Type##CLASS##SerilizerWrapper::memnew_class_arr,         \
& TypeFieldReflectionOperator::Type##CLASS##SerilizerWrapper::get_size_of));         \
    REGISTER_BASE_CLASS_TO_MAP(#CLASS, class_function_tuple_##CLASS##);             \
    }           \
    }               \
} // namespace Reflection

#define SERIALIZER_FUNCS(CLASS)     \
public : static const char* getClassName() { return #CLASS; } \
                                                           \
        static void* constructorWithJson(const Json& json_context) { \
            CLASS* ret_instance = memnew(CLASS); \
            Serializer::read(json_context, *ret_instance); \
            return ret_instance;                            \
         }                                                 \
                                                           \
        static Json writeByName(void* instance) {       \
        \
            return Serializer::write(*(CLASS*)instance); \
    }                                                   \
    static void json_read_warpper(const Json& json, void* instance) {       \
                                                     \
           Serializer::read(json, *reinterpret_cast<CLASS*>(instance));      \
    }                                                   \

// manual的情况下，用namespace而不是类

#define REFLECT_FIELD_DEF(CLASS, FName, TYPE, SetF, GetF, IsArray) \
	namespace Reflection{			\
	namespace TypeFieldReflectionOperator{								\
		namespace Type##CLASS##Operator{\
			const char* get_fieldname_##FName(){return #FName;}\
			const char* get_field_typename_##FName(){return #TYPE;}\
			bool isArray_##FName(){return IsArray; }	\
		}\
	static void TypeWrappersRegister##CLASS####FName##() {		\
	FieldFunctionTuple* field_function_tuple_##FName= memnew(FieldFunctionTuple(		\
		&SetF,	\
		&GetF,	\
		&TypeFieldReflectionOperator::Type##CLASS##Operator::getClassName,		\
		&TypeFieldReflectionOperator::Type##CLASS##Operator::get_fieldname_##FName,\
		&TypeFieldReflectionOperator::Type##CLASS##Operator::get_field_typename_##FName,\
		&TypeFieldReflectionOperator::Type##CLASS##Operator::isArray_##FName));\
	}}\
}

// safety的情况下，用类

#define REFLECT_FIELD_DEF_SAFETY(CLASS, FName, TYPE, SetF, GetF, IsArray) \
			static const char* get_fieldname_##FName(){return #FName;}\
			static const char* get_field_typename_##FName(){return #TYPE;}\
			static bool isArray_##FName(){return IsArray; }	\
	static void TypeWrappersRegister##CLASS####FName##() {		\
	FieldFunctionTuple* field_function_tuple_##FName= memnew(FieldFunctionTuple(		\
		&SetF,	\
		&GetF,	\
		&TypeFieldReflectionOperator::Type##CLASS##Operator::getClassName,		\
		&TypeFieldReflectionOperator::Type##CLASS##Operator::get_fieldname_##FName,\
		&TypeFieldReflectionOperator::Type##CLASS##Operator::get_field_typename_##FName,\
		&TypeFieldReflectionOperator::Type##CLASS##Operator::isArray_##FName));\
	}

// ct 从前向后顶， vals匹配
// 

#define VALS(N1, N2, N3, N4, N5, N, ...) N
#define CATGENCOUNT(N) CATGENCOUNT_IMPL(N)
#define CATGENCOUNT_IMPL(N) GENERATE_MANUAL_REFLECT_FIELD_CODE##N
#define CT(...) VALS(__VA_ARGS__, 5, 4, 3, 2, 1)

#define GENERATE_MANUAL_REFLECT_FIELD_CODE(CLASS, ... ) CATGENCOUNT(CT(__VA_ARGS__))(CLASS,__VA_ARGS__)
#define GENERATE_MANUAL_REFLECT_FIELD_CODE1(CLASS, FName) Reflection::TypeWrappersRegister##CLASS####FName##();
#define GENERATE_MANUAL_REFLECT_FIELD_CODE2(CLASS, FName, ...) Reflection::TypeWrappersRegister##CLASS####FName##(); GENERATE_MANUAL_REFLECT_FIELD_CODE1(CLASS,__VA_ARGS__)
#define GENERATE_MANUAL_REFLECT_FIELD_CODE3(CLASS, FName, ...) Reflection::TypeWrappersRegister##CLASS####FName##(); GENERATE_MANUAL_REFLECT_FIELD_CODE2(CLASS,__VA_ARGS__)
#define GENERATE_MANUAL_REFLECT_FIELD_CODE4(CLASS, FName, ...) Reflection::TypeWrappersRegister##CLASS####FName##(); GENERATE_MANUAL_REFLECT_FIELD_CODE3(CLASS,__VA_ARGS__)
#define GENERATE_MANUAL_REFLECT_FIELD_CODE5(CLASS, FName, ...) Reflection::TypeWrappersRegister##CLASS####FName##(); GENERATE_MANUAL_REFLECT_FIELD_CODE4(CLASS,__VA_ARGS__)

#define MANUAL_REFLECT_FIELD(CLASS, ...) \
namespace Reflection{   \
namespace TypeWrappersRegister{		\
	void CLASS() { \
		do{		\
			GENERATE_MANUAL_REFLECT_FIELD_CODE(CLASS, __VA_ARGS__)  \
		} while(0);\
	} }}

#define CATGENCOUNT_SAFETY(N) CATGENCOUNT_IMPL_SAFETY(N)
#define CATGENCOUNT_IMPL_SAFETY(N) GENERATE_MANUAL_REFLECT_FIELD_CODE_SAFETY##N
#define CLASS_GEN_CODE_LINE(CLASS, FName) Reflection::TypeFieldReflectionOperator::Type##CLASS##Operator::TypeWrappersRegister##CLASS####FName##();
#define GENERATE_MANUAL_REFLECT_FIELD_CODE_SAFETY(CLASS, ... ) CATGENCOUNT_SAFETY(CT(__VA_ARGS__))(CLASS,__VA_ARGS__)
#define GENERATE_MANUAL_REFLECT_FIELD_CODE_SAFETY1(CLASS, FName) CLASS_GEN_CODE_LINE(CLASS,FName)
#define GENERATE_MANUAL_REFLECT_FIELD_CODE_SAFETY2(CLASS, FName, ...) CLASS_GEN_CODE_LINE(CLASS,FName) GENERATE_MANUAL_REFLECT_FIELD_CODE_SAFETY1(CLASS,__VA_ARGS__)
#define GENERATE_MANUAL_REFLECT_FIELD_CODE_SAFETY3(CLASS, FName, ...) CLASS_GEN_CODE_LINE(CLASS,FName) GENERATE_MANUAL_REFLECT_FIELD_CODE_SAFETY2(CLASS,__VA_ARGS__)
#define GENERATE_MANUAL_REFLECT_FIELD_CODE_SAFETY4(CLASS, FName, ...) CLASS_GEN_CODE_LINE(CLASS,FName)  GENERATE_MANUAL_REFLECT_FIELD_CODE_SAFETY3(CLASS,__VA_ARGS__)
#define GENERATE_MANUAL_REFLECT_FIELD_CODE_SAFETY5(CLASS, FName, ...) CLASS_GEN_CODE_LINE(CLASS,FName) GENERATE_MANUAL_REFLECT_FIELD_CODE_SAFETY4(CLASS,__VA_ARGS__)
#define MANUAL_REFLECT_FIELD_SAFETY(CLASS, ...) \
}; } \
namespace TypeWrappersRegister{     \
        void CLASS() {                  \
            \
                GENERATE_MANUAL_REFLECT_FIELD_CODE_SAFETY(CLASS, __VA_ARGS__)  \
        }   \
    }   \
};

// TypeWrappersRegister
#define TypeMetaDef(class_name, ptr) \
    lain::Reflection::ReflectionInstance(lain::Reflection::TypeMeta::newMetaFromName(#class_name), \
                                            (class_name*)ptr)

#define TypeMetaDefPtr(class_name, ptr) \
    memnew(lain::Reflection::ReflectionInstance(lain::Reflection::TypeMeta::newMetaFromName(#class_name), \
                                                (class_name*)ptr))
#endif