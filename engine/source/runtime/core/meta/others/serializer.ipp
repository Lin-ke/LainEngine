#pragma once
#ifndef SERIALIZER_IPP
#define SERIALIZER_IPP

#include "core/meta/serializer/serializer.h"
// 手写
namespace lain {
	/// <summary>
	/// StringName
	/// </summary>
	/// <param name="json_context"></param>
	/// <param name="instance"></param>
	/// <returns></returns>
	template<>
	void Serializer::read(const Json& json_context, StringName& instance)
	{
		memnew_placement(&instance, StringName(String(json_context.string_value())));
	}
	template<>
	Json Serializer::write(const StringName& instance)
	{
		
		return Json(CSTR(instance.operator lain::String()));
		
	}

	template<>
	void Serializer::read(const Json& json_context, GObjectPath& instance) {
		String str;
		Serializer::read(json_context, str);
		memnew_placement(&instance, GObjectPath(String(json_context.string_value())));
	}

	template<>
	Json Serializer::write(const GObjectPath& instance) {
		return Serializer::write(instance.operator String());

	}
}
#endif // !SERIALIZER_IPP
