#pragma once
#include "runtime\core\math\matrix4.h"

namespace Lain{
    class Matrix4x4_;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeMatrix4x4_Operator{
    public:
        static const char* getClassName(){ return "Matrix4x4_";}
        static void* constructorWithJson(const Json& json_context){
            Matrix4x4_* ret_instance= new Matrix4x4_;
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(Matrix4x4_*)instance);
        }
        // base class
        static int getMatrix4x4_BaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 0;
            
            return count;
        }
        // fields
        static const char* getFieldName_v0(){ return "v0";}
        static const char* getFieldTypeName_v0(){ return "float";}
        static void set_v0(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v0 = *static_cast<float*>(field_value);}
        static void* get_v0(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v0));}
        static bool isArray_v0(){ return false; }
        static const char* getFieldName_v1(){ return "v1";}
        static const char* getFieldTypeName_v1(){ return "float";}
        static void set_v1(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v1 = *static_cast<float*>(field_value);}
        static void* get_v1(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v1));}
        static bool isArray_v1(){ return false; }
        static const char* getFieldName_v2(){ return "v2";}
        static const char* getFieldTypeName_v2(){ return "float";}
        static void set_v2(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v2 = *static_cast<float*>(field_value);}
        static void* get_v2(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v2));}
        static bool isArray_v2(){ return false; }
        static const char* getFieldName_v3(){ return "v3";}
        static const char* getFieldTypeName_v3(){ return "float";}
        static void set_v3(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v3 = *static_cast<float*>(field_value);}
        static void* get_v3(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v3));}
        static bool isArray_v3(){ return false; }
        static const char* getFieldName_v4(){ return "v4";}
        static const char* getFieldTypeName_v4(){ return "float";}
        static void set_v4(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v4 = *static_cast<float*>(field_value);}
        static void* get_v4(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v4));}
        static bool isArray_v4(){ return false; }
        static const char* getFieldName_v5(){ return "v5";}
        static const char* getFieldTypeName_v5(){ return "float";}
        static void set_v5(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v5 = *static_cast<float*>(field_value);}
        static void* get_v5(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v5));}
        static bool isArray_v5(){ return false; }
        static const char* getFieldName_v6(){ return "v6";}
        static const char* getFieldTypeName_v6(){ return "float";}
        static void set_v6(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v6 = *static_cast<float*>(field_value);}
        static void* get_v6(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v6));}
        static bool isArray_v6(){ return false; }
        static const char* getFieldName_v7(){ return "v7";}
        static const char* getFieldTypeName_v7(){ return "float";}
        static void set_v7(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v7 = *static_cast<float*>(field_value);}
        static void* get_v7(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v7));}
        static bool isArray_v7(){ return false; }
        static const char* getFieldName_v8(){ return "v8";}
        static const char* getFieldTypeName_v8(){ return "float";}
        static void set_v8(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v8 = *static_cast<float*>(field_value);}
        static void* get_v8(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v8));}
        static bool isArray_v8(){ return false; }
        static const char* getFieldName_v9(){ return "v9";}
        static const char* getFieldTypeName_v9(){ return "float";}
        static void set_v9(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v9 = *static_cast<float*>(field_value);}
        static void* get_v9(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v9));}
        static bool isArray_v9(){ return false; }
        static const char* getFieldName_v10(){ return "v10";}
        static const char* getFieldTypeName_v10(){ return "float";}
        static void set_v10(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v10 = *static_cast<float*>(field_value);}
        static void* get_v10(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v10));}
        static bool isArray_v10(){ return false; }
        static const char* getFieldName_v11(){ return "v11";}
        static const char* getFieldTypeName_v11(){ return "float";}
        static void set_v11(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v11 = *static_cast<float*>(field_value);}
        static void* get_v11(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v11));}
        static bool isArray_v11(){ return false; }
        static const char* getFieldName_v12(){ return "v12";}
        static const char* getFieldTypeName_v12(){ return "float";}
        static void set_v12(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v12 = *static_cast<float*>(field_value);}
        static void* get_v12(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v12));}
        static bool isArray_v12(){ return false; }
        static const char* getFieldName_v13(){ return "v13";}
        static const char* getFieldTypeName_v13(){ return "float";}
        static void set_v13(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v13 = *static_cast<float*>(field_value);}
        static void* get_v13(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v13));}
        static bool isArray_v13(){ return false; }
        static const char* getFieldName_v14(){ return "v14";}
        static const char* getFieldTypeName_v14(){ return "float";}
        static void set_v14(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v14 = *static_cast<float*>(field_value);}
        static void* get_v14(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v14));}
        static bool isArray_v14(){ return false; }
        static const char* getFieldName_v15(){ return "v15";}
        static const char* getFieldTypeName_v15(){ return "float";}
        static void set_v15(void* instance, void* field_value){ static_cast<Matrix4x4_*>(instance)->v15 = *static_cast<float*>(field_value);}
        static void* get_v15(void* instance){ return static_cast<void*>(&(static_cast<Matrix4x4_*>(instance)->v15));}
        static bool isArray_v15(){ return false; }

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_Matrix4x4_(){
        FieldFunctionTuple* field_function_tuple_v0=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v0,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v0,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v0,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v0,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v0);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v0);
        FieldFunctionTuple* field_function_tuple_v1=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v1,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v1,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v1,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v1,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v1);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v1);
        FieldFunctionTuple* field_function_tuple_v2=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v2,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v2,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v2,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v2,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v2);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v2);
        FieldFunctionTuple* field_function_tuple_v3=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v3,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v3,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v3,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v3,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v3);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v3);
        FieldFunctionTuple* field_function_tuple_v4=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v4,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v4,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v4,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v4,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v4);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v4);
        FieldFunctionTuple* field_function_tuple_v5=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v5,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v5,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v5,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v5,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v5);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v5);
        FieldFunctionTuple* field_function_tuple_v6=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v6,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v6,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v6,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v6,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v6);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v6);
        FieldFunctionTuple* field_function_tuple_v7=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v7,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v7,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v7,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v7,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v7);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v7);
        FieldFunctionTuple* field_function_tuple_v8=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v8,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v8,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v8,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v8,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v8);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v8);
        FieldFunctionTuple* field_function_tuple_v9=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v9,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v9,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v9,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v9,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v9);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v9);
        FieldFunctionTuple* field_function_tuple_v10=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v10,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v10,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v10,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v10,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v10);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v10);
        FieldFunctionTuple* field_function_tuple_v11=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v11,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v11,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v11,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v11,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v11);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v11);
        FieldFunctionTuple* field_function_tuple_v12=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v12,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v12,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v12,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v12,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v12);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v12);
        FieldFunctionTuple* field_function_tuple_v13=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v13,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v13,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v13,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v13,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v13);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v13);
        FieldFunctionTuple* field_function_tuple_v14=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v14,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v14,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v14,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v14,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v14);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v14);
        FieldFunctionTuple* field_function_tuple_v15=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::set_v15,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::get_v15,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getClassName,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldName_v15,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getFieldTypeName_v15,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::isArray_v15);
        REGISTER_FIELD_TO_MAP("Matrix4x4_", field_function_tuple_v15);

        
        
        
        ClassFunctionTuple* class_function_tuple_Matrix4x4_=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::getMatrix4x4_BaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeMatrix4x4_Operator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("Matrix4x4_", class_function_tuple_Matrix4x4_);
    }
namespace TypeWrappersRegister{
    void Matrix4()
    {
        TypeWrapperRegister_Matrix4x4_();
    }
}//namespace TypeWrappersRegister

}//namespace Reflection
}//namespace Piccolo

