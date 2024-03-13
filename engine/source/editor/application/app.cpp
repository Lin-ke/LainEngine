#ifdef L_PLATFORM_WINDOWS
#ifdef L_DEBUG

#endif
#include <base.h>
#include <core/meta/meta_example.cpp>
#include <core/engine/engine.h>
#include <core/math/vector2.h>
#include <core/error/error_macros.h>
#include <core/os/memory.h>
#include <core/templates/cowdata.h>
#include <core/templates/vector.h>
#include <core/templates/safe_numeric.h>
#include <core/meta/meta_example.h>
#include <platform/io/os_windows.h>
#include "core/string/ustring.h"
using namespace lain;
int YminusA(float a, Vector3& obj);


int main(int argc, char** argv) {
	
	// main function
	OSWin os = OSWin();

	OSWin::GetSingleton()->Run();
	YminusA(12, Vector3{ 12,3,4 });

	
}


int YminusA(float a, Vector3& obj) {
	auto meta = Reflection::TypeMeta::newMetaFromName("Vector3");
	if (!meta.isValid()) {
		L_ERROR("meta is not valid");
	}
	auto field_accessor = meta.getFieldByName("y");
	if (field_accessor.isValid()) {
		L_INFO("valid");
		// set
		field_accessor.set((void*)&obj, (float *)&a);
		L_JSON(obj);
	}
	else {
		L_ERROR("Field is not valid");
	}
	return 0;


}

#endif
// in debug mode, each allocation is padded with 16 bytes
// GObeject contains reflection ptr as its' component.
