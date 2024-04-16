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
		auto&& p = json_context.dump();

		memnew_placement(&instance, StringName(String(json_context["name"].string_value())));
		return instance;
	}
	template<>
	Json Serializer::write(const StringName& instance)
	{
		Json::object  ret_context;
		ret_context.insert_or_assign("name", Serializer::write(instance._data->name));
		ret_context.insert_or_assign("static", Serializer::write(instance._data->static_count.get() > 0));
		return ret_context;
		
	}

	template<>
	GObjectPath& Serializer::read(const Json& json_context, GObjectPath& instance) {
		String str;
		Serializer::read(json_context, str);
		instance = str;
		return instance;
	}

	template<>
	Json Serializer::write(const GObjectPath& instance) {
		return Serializer::write(instance.operator String());

	}
}
#endif // !SERIALIZER_IPP
