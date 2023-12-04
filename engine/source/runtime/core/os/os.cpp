#include "os.h"
#include "core/mainloop/main.h"
#include "function/display/window_system.h"
#include <timeapi.h>
OS* OS::p_singleton = nullptr;
void OSWin::Run() {
	// init
	lain::Main::Init();
	while (true) {
		//ÈË»ú½»»¥
		// display.send_events;
		bool exit = lain::WindowSystem::GetSingleton()->ShouldClose();

		
		if (!lain::Main::Loop()||exit) {
			break;
		}
		lain::WindowSystem::GetSingleton()->PollEvents();

	}
	L_PRINT("Exiting. Have a nice day.");

}
void OSWin::Initialize() {

	QueryPerformanceFrequency((LARGE_INTEGER*)&ticks_per_second);
	QueryPerformanceCounter((LARGE_INTEGER*)&ticks_start);
	L_PRINT(ticks_per_second, ticks_start);
	timeBeginPeriod(1);
}


uint64_t OSWin::GetTimeUsec() const {
	uint64_t ticks;

	// This is the number of clock ticks since start
	QueryPerformanceCounter((LARGE_INTEGER*)&ticks);
	// Subtract the ticks at game start to get
	// the ticks since the game started
	ticks -= ticks_start;

	// Divide by frequency to get the time in seconds
	// original calculation shown below is subject to overflow
	// with high ticks_per_second and a number of days since the last reboot.
	// time = ticks * 1000000L / ticks_per_second;

	// we can prevent this by either using 128 bit math
	// or separating into a calculation for seconds, and the fraction
	uint64_t seconds = ticks / ticks_per_second;

	// compiler will optimize these two into one divide
	uint64_t leftover = ticks % ticks_per_second;

	// remainder
	uint64_t time = (leftover * 1000000L) / ticks_per_second;

	// seconds
	time += seconds * 1000000L;

	return time;
}