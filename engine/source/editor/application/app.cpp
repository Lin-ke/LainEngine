#ifdef L_PLATFORM_WINDOWS
#ifdef L_DEBUG

#endif
#include <base.h>
#include <platform/os/os_windows.h>
using namespace lain;
#include "test/test_headers.h"
#include "core/main/main.h"

int main() {
	
	// main function
	OSWindows os = OSWindows();
	char* argv[] = {"D:/LainEngine/game"}; //³¡¾°1
	Error err = Main::Initialize(1, argv);
	if (err != OK) {
		// LPRINT
	}
	//test::test_scene();
	test::test_image_io();

	os.Run();
	/*test::test_Vector3_reflect();
	test::test_StringName_reflect();
	test::test_accessor();
	test::test_assign_using_serializer();*/
	//test::test_fileio();
}



#endif
// in debug mode, each allocation is padded with 16 bytes
// GObeject contains reflection ptr as its' component.
