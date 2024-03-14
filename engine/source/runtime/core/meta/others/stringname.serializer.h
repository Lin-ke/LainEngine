#pragma once
#ifndef STRINGNAME_SERIALIZER_H
#define STRINGNAME_SERIALIZER_H

#include "core/meta/serializer/serializer.h"
#include "core/string/string_name.h"

namespace lain {

	template<>
	StringName& Serializer::read(const Json& json_context, StringName& instance);

	template<>
	Json Serializer::write(const StringName& instance);
}
#endif // !STRINGNAME_SERIALIZER_H
