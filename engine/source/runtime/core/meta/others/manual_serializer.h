#ifndef MANUAL_SERIALIZER_H
#define MANUAL_SERIALIZER_H
#include "core/meta/serializer/serializer.h"
#include "core/string/string_name.h"
#include "core/scene/object/gobject_path.h"
namespace lain {

	template<>
	StringName& Serializer::read(const Json& json_context, StringName& instance);

	template<>
	Json Serializer::write(const StringName& instance);

	template<>
	GObjectPath& Serializer::read(const Json& json_context, GObjectPath& instance);

	template<>
	Json Serializer::write(const GObjectPath& instance);
}
#endif // !MANUAL_SERIALIZER_H
