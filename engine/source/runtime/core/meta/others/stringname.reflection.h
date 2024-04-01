#pragma once
#ifndef STRINGNAMERREFLCTION_H
#define STRINGNAMERREFLCTION_H
#include "core/string/string_name.h"
#include "core/os/memory.h"
namespace lain {
	

	/// <summary>
	/// reflection and register
	/// </summary>
	namespace Reflection {
		namespace TypeFieldReflectionOperator {
			class TypeStringNameOperator {
			public:
				static const char* getClassName() { return "StringName"; }
				static void* constructorWithJson(const Json& json_context) {
					StringName* ret_instance = memnew(StringName);
					Serializer::read(json_context, *ret_instance);
					return ret_instance;
				}
				static Json writeByName(void* instance) {
					return Serializer::write(*(StringName*)instance);
				}
				// base class
				static int getStringNameBaseClassReflectionInstanceList(ReflectionInstance*& out_list, void* instance) {
					int count = 0;

					return count;
				}
				// fields
				static const char* getFieldName_m_name() { return "name"; }
				static const char* getFieldTypeName_m_name() { return "String"; }

				// field_value is String
				// piccolo这个系统太羸弱了，不支持重载
				// TODO: 把这些void*换成callable
				static void set_m_name(void* instance_ptr, void* field_value) {
					StringName& instance = *static_cast<StringName*> (instance_ptr);
					String& p_name = *static_cast<String*>(field_value);
					uint32_t hash = instance.hash();
					uint32_t idx = hash & STRING_TABLE_MASK;



					instance._data = instance._table[idx];

					while (instance._data) {
						if (instance._data->hash == hash && instance._data->get_name() == p_name) {
							break;
						}
						instance._data = instance._data->next;
					}

					if (instance._data && instance._data->refcount.ref()) {
						/// 这里删掉了static的部分
#ifdef DEBUG_ENABLED
						/*if (unlikely(debug_stringname)) {
							instance._data->debug_references++;
						}*/
#endif
						return;
					}

					instance._data = memnew(StringName::_Data);
					instance._data->name = p_name;
					instance._data->refcount.init();
					instance._data->static_count.set(0);
					instance._data->hash = hash;
					instance._data->idx = idx;
					instance._data->cname = nullptr;
					instance._data->next = instance._table[idx];
					instance._data->prev = nullptr;
#ifdef DEBUG_ENABLED
					//if (unlikely(debug_stringname)) {
					//	// Keep in memory, force static.
					//	instance._data->refcount.ref();
					//	instance._data->static_count.increment();
					//}
#endif

					if (instance._table[idx]) {
						instance._table[idx]->prev = instance._data;
					}
					instance._table[idx] = instance._data;
					return;
				}
				static void* get_m_name(void* instance) { return (void*)&static_cast<StringName*>(instance)->_data->name; }
				static bool isArray_m_name() { return false; }
			}; // class TypeStringNameOperator
		} // namespace Typefield
			void TypeWrapperRegister_StringName() {
				FieldFunctionTuple* field_function_tuple_m_name = memnew(FieldFunctionTuple(
					&TypeFieldReflectionOperator::TypeStringNameOperator::set_m_name,
					&TypeFieldReflectionOperator::TypeStringNameOperator::get_m_name,
					&TypeFieldReflectionOperator::TypeStringNameOperator::getClassName,
					&TypeFieldReflectionOperator::TypeStringNameOperator::getFieldName_m_name,
					&TypeFieldReflectionOperator::TypeStringNameOperator::getFieldName_m_name,
					&TypeFieldReflectionOperator::TypeStringNameOperator::isArray_m_name));
				REGISTER_FIELD_TO_MAP("StringName", field_function_tuple_m_name);
				



				ClassFunctionTuple* class_function_tuple_SubMeshRes = memnew(ClassFunctionTuple(
					&TypeFieldReflectionOperator::TypeStringNameOperator::getStringNameBaseClassReflectionInstanceList,
					&TypeFieldReflectionOperator::TypeStringNameOperator::constructorWithJson,
					&TypeFieldReflectionOperator::TypeStringNameOperator::writeByName));
				REGISTER_BASE_CLASS_TO_MAP("StringName", class_function_tuple_SubMeshRes);
			}

	namespace TypeWrappersRegister {
		void StringName()
		{
			TypeWrapperRegister_StringName();
		}
	}//namespace TypeWrappersRegister
	} //Reflection

}// namespace lain

#endif // !STRINGNAMERREFLCTION_H
