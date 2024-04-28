#ifdef L_PLATFORM_WINDOWS
#ifdef L_DEBUG
#define _TEST_

#endif
#include <base.h>
#include <platform/os/os_windows.h>
using namespace lain;
#include "test/test_headers.h"
#include "core/main/main.h"
#ifndef _TEST_

int main() {

	// main function
	OSWindows os = OSWindows();
	char* argv[] = { "D:/LainEngine/game" }; //³¡¾°1
	Error err = Main::Initialize(1, argv);
	if (err != OK) {
		L_PERROR("initialize failed");
	}
	os.Run();

}
#endif // _TEST_

#ifdef _TEST_
int main() {

	// main function
	OSWindows os = OSWindows();
	char* argv[] = { "D:/LainEngine/game" }; //³¡¾°1
	Error err = Main::Initialize(1, argv);
	os.main_loop->initialize(); // scenetree
	test::test_process();
}
#endif



#endif
// in debug mode, each allocation is padded with 16 bytes
// GObeject contains reflection ptr as its' component.
