#include "signal.h"
#include "core/string/string_name.h"
#include "core/object/objectdb.h"
using namespace lain;
_FORCE_INLINE_ bool Signal::is_null() const
{
	return object.is_null() && name == StringName();
}
Object *Signal::get_object() const
{
	return ObjectDB::get_instance(object);
}

bool Signal::operator==(const Signal &p_signal) const
{
	return object == p_signal.object && name == p_signal.name;
}

bool Signal::operator!=(const Signal &p_signal) const
{
	return object != p_signal.object || name != p_signal.name;
}

bool Signal::operator<(const Signal &p_signal) const
{
	if (object == p_signal.object)
	{
		return name < p_signal.name;
	}
	else
	{
		return object < p_signal.object;
	}
}

Signal::Signal(const Object *p_object, const StringName &p_name)
{
	ERR_FAIL_NULL_MSG(p_object, "Object argument to Signal constructor must be non-null.");

	object = p_object->get_instance_id();
	name = p_name;
}

Signal::Signal(ObjectID p_object, const StringName &p_name)
{
	object = p_object;
	name = p_name;
}
