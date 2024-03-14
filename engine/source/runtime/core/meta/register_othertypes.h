#pragma once
#ifndef REGISTER_OTHER_TYPES_H
#define REGISTER_OTHER_TYPES_H

#include "runtime/core/string/string_name.h"
#include "serializer/serializer.h"
#include "core/os/memory.h"
namespace lain {


	void register_core_types() {
		lain::StringName::setup();
	}

	void register_serializers() {

	}

	template<>
	StringName& Serializer::read(const Json& json_context, StringName& instance)
	{
		assert(json_context.is_object());
		String p_name = "";
		Serializer::read<String>(json_context["$String"], p_name);
		bool m_static = false;
		Serializer::read<bool>(json_context["m_static"], m_static);

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
			// exists
			if (m_static) {
				instance._data->static_count.increment();
			}
#ifdef DEBUG_ENABLED
			if (unlikely(debug_stringname)) {
				instance._data->debug_references++;
			}
#endif
			return instance;
		}

		instance._data = memnew(StringName::_Data);
		instance._data->name = p_name;
		instance._data->refcount.init();
		instance._data->static_count.set(m_static ? 1 : 0);
		instance._data->hash = hash;
		instance._data->idx = idx;
		instance._data->cname = nullptr;
		instance._data->next = instance._table[idx];
		instance._data->prev = nullptr;
#ifdef DEBUG_ENABLED
		if (unlikely(debug_stringname)) {
			// Keep in memory, force static.
			instance._data->refcount.ref();
			instance._data->static_count.increment();
		}
#endif

		if (instance._table[idx]) {
			instance._table[idx]->prev = instance._data;
		}
		instance._table[idx] = instance._data;
		return instance;
		
	}
	template<>
	Json Serializer::write(const StringName& instance)
	{
		Json::object  ret_context;

		ret_context.insert_or_assign("name", Serializer::write(instance._data->name));
		ret_context.insert_or_assign("static", Serializer::write(instance._data->static_count.get() > 0));
		return  Json(ret_context);
	}

	namespace Reflection {
		namespace TypeFieldReflectionOparator {
			class TypeStringOperator {
			
			
			public:
				static const char* getClassName() { return "String"; }
				static void* constructorWithJson(const Json& json_context) {
					String* ret_instance = memnew(String);
					Serializer::read(json_context, *ret_instance);
					return ret_instance;
				}
				static Json writeByName(void* instance) {
					return Serializer::write(*(String*)instance);
				}
				// base class
				static int getStringBaseClassReflectionInstanceList(ReflectionInstance*& out_list, void* instance) {
					int count = 0;

					return count;
				}
				// fields
				static const char* getFieldName_data() { return "data"; }
				static const char* getFieldTypeName_data() { return "String"; }
				static void set_data(void* instance, String* field_value) { *static_cast<String*>(instance) = *field_value; }
				static void set_data(void* instance, char* field_value) { (*static_cast<String*>(instance)).copy_from(field_value); }
				// data就是他自己
				static void* get_data(void* instance) { return instance; }
				static bool isArray_data() { return false; }
			};
		}

	}
	namespace Reflection {
		namespace TypeFieldReflectionOparator {
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
				static int getStringBaseClassReflectionInstanceList(ReflectionInstance*& out_list, void* instance) {
					int count = 0;

					return count;
				}
				// fields
				static const char* getFieldName_name() { return "name"; }
				// field_value is String
				static void set_name(void* instance_ptr, String* field_value, bool m_static = false) { 
					StringName& instance = *static_cast<StringName*> (instance_ptr );
					String& p_name = *field_value;
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
						// exists
						if (m_static) {
							instance._data->static_count.increment();
						}
#ifdef DEBUG_ENABLED
						if (unlikely(debug_stringname)) {
							instance._data->debug_references++;
						}
#endif
						return ;
					}

					instance._data = memnew(StringName::_Data);
					instance._data->name = p_name;
					instance._data->refcount.init();
					instance._data->static_count.set(m_static ? 1 : 0);
					instance._data->hash = hash;
					instance._data->idx = idx;
					instance._data->cname = nullptr;
					instance._data->next = instance._table[idx];
					instance._data->prev = nullptr;
#ifdef DEBUG_ENABLED
					if (unlikely(debug_stringname)) {
						// Keep in memory, force static.
						instance._data->refcount.ref();
						instance._data->static_count.increment();
					}
#endif

					if (instance._table[idx]) {
						instance._table[idx]->prev = instance._data;
					}
					instance._table[idx] = instance._data;
					return ;
				}
				static void* get_name(void* instance) { return (void*) & static_cast<StringName*>(instance)->_data->name; }
				static bool isArray_name() { return false; }
			};
		}

	}

	void register_other_types() {

		register_core_types();
		register_serializers();
	}


}

#endif // !REGISTER_OTHER_TYPES_H
