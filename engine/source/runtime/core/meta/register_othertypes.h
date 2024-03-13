#pragma once
#ifndef REGISTER_OTHER_TYPES_H
#define REGISTER_OTHER_TYPES_H

#include "runtime/core/string/string_name.h"
#include "serializer/serializer.h"
namespace lain {


	void register_core_types() {
		lain::StringName::setup();
	}

	void register_serializers() {

	}
	template<>
	StringName& Serializer::read(const Json& json_context, StringName& instance)
	{
		assert(json_context.is_bool());
		return instance = json_context.bool_value();
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
