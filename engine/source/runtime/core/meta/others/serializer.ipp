#pragma once
#ifndef SERIALIZER_IPP
#define SERIALIZER_IPP

#include "core/meta/serializer/serializer.h"

namespace lain {
	/// <summary>
	/// StringName
	/// </summary>
	/// <param name="json_context"></param>
	/// <param name="instance"></param>
	/// <returns></returns>
	template<>
	StringName& Serializer::read(const Json& json_context, StringName& instance)
	{
		assert(json_context.is_object());
		String p_name = "";
		Serializer::read<String>(json_context["name"], p_name);
		bool m_static = false;
		Serializer::read<bool>(json_context["static"], m_static);

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
}
#endif // !SERIALIZER_IPP
