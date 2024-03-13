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
				_data->debug_references++;
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
			_data->refcount.ref();
			_data->static_count.increment();
		}
#endif

		if (instance._table[idx]) {
			instance._table[idx]->prev = instance._data;
		}
		instance._table[idx] = instance._data;
		return instance;
		
	}
	template<>
	String& Serializer::read(const Json& json_context, String& instance)
	{
		assert(json_context.is_string());
		instance += json_context.string_value().c_str();
		return instance;
	}
	template<>
	Json Serializer::write(const StringName& instance)
	{
		return Json(instance);
	}

	void register_other_types() {

		register_core_types();
		register_serializers();
	}


}

#endif // !REGISTER_OTHER_TYPES_H
