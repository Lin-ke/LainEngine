#include "reflection_register_others.h"

#include "core/meta/reflection/reflection_marcos.h"
#include "core/variant/dictionary.h"
#include "core/string/string_name.h"
#include "core/scene/object/gobject_path.h"
#include "core/meta/serializer/serializer.h"
namespace lain{

SERIALIZER(Dictionary); 
SERIALIZER(StringName);
SERIALIZER(GObjectPath);
SERIALIZER(String);

void register_other_meta()
{
   {
		SERIAL_REG(StringName);
		SERIAL_REG(Dictionary);
		SERIAL_REG(StringName);
		SERIAL_REG(GObjectPath);
		SERIAL_REG(String);
	}

}
}
