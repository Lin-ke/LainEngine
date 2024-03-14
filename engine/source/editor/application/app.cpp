#ifdef L_PLATFORM_WINDOWS
#ifdef L_DEBUG

#endif
#include <base.h>
#include <platform/io/os_windows.h>
#include "test/test_reflect.h"

using namespace lain;


int main(int argc, char** argv) {
	
	// main function
	OSWin os = OSWin();

	OSWin::GetSingleton()->Run();
	test::test_Vector3_reflect();
	test::test_StringName_reflect();

	
}



#endif
// in debug mode, each allocation is padded with 16 bytes
// GObeject contains reflection ptr as its' component.
