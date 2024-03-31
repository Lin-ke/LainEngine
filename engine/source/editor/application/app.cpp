#ifdef L_PLATFORM_WINDOWS
#ifdef L_DEBUG

#endif
#include <base.h>
#include <platform/os/os_windows.h>
using namespace lain;
#include "test/test_headers.h"
#include "core/mainloop/main.h"
int main(int argc, char** argv) {
	
	// main function
	OSWindows os = OSWindows();

	Error err = Main::Initialize(0, nullptr);
	if (err != OK) {
		// 
	}
	os.Run();
	/*test::test_Vector3_reflect();
	test::test_StringName_reflect();
	test::test_accessor();
	test::test_assign_using_serializer();*/
	test::test_fileio();
}



#endif
// in debug mode, each allocation is padded with 16 bytes
// GObeject contains reflection ptr as its' component.
