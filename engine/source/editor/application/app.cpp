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
int YminusA(int a, Vector2& obj);
void TryERR_FAIL_INDEX();

int main(int argc, char** argv) {
	std::cout << VERSION_BRANCH << std::endl;
	// main function
	OSWin os = OSWin();

	OSWin::GetSingleton()->Run();
	
}

void TryERR_FAIL_INDEX() {
	ERR_FAIL_INDEX(-1, 3);
}
int YminusA(int a, Vector2& obj) {
	auto meta = Reflection::TypeMeta::newMetaFromName("Vector2");
	if (!meta.isValid()) {
		L_ERROR("meta is not valid");
	}
	auto field_accessor = meta.getFieldByName("x");
	float b = 16;
	if (field_accessor.isValid()) {
		L_INFO("valid");
		// set
		field_accessor.set((void*)&obj, (float *)&b);
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
