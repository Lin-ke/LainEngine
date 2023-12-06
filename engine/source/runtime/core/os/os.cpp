#include "os.h"
#include "core/mainloop/main.h"
#include "function/display/window_system.h"
#include <timeapi.h>
namespace lain {
OS* OS::p_singleton = nullptr;
 String OS::GetResourceDir() const {
	return ProjectSettings::GetSingleton()->GetResourcePath();
}
 String OS::GetUserDataDir() const {
	 return ".";
 }

void OSWin::Run() {
	// init
	Main::Init();
	while (true) {
		//ÈË»ú½»»¥
		// display.send_events;
		bool exit = WindowSystem::GetSingleton()->ShouldClose();

		
		if (!Main::Loop()||exit) {
			break;
		}
		WindowSystem::GetSingleton()->PollEvents();

	}
	L_PRINT("Exiting. Have a nice day.");

}
void OSWin::Initialize() {

	QueryPerformanceFrequency((LARGE_INTEGER*)&ticks_per_second);
	QueryPerformanceCounter((LARGE_INTEGER*)&ticks_start);
	//L_PRINT(ticks_per_second, ticks_start);
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
OS::DateTime OSWin::GetDateTime(bool p_utc) const {
	SYSTEMTIME systemtime;
	if (p_utc) {
		GetSystemTime(&systemtime);
	}
	else {
		GetLocalTime(&systemtime);
	}

	//Get DST information from Windows, but only if p_utc is false.
	TIME_ZONE_INFORMATION info;
	bool is_daylight = false;
	if (!p_utc && GetTimeZoneInformation(&info) == TIME_ZONE_ID_DAYLIGHT) {
		is_daylight = true;
	}

	DateTime dt;
	dt.year = systemtime.wYear;
	dt.month = Month(systemtime.wMonth);
	dt.day = systemtime.wDay;
	dt.weekday = Weekday(systemtime.wDayOfWeek);
	dt.hour = systemtime.wHour;
	dt.minute = systemtime.wMinute;
	dt.second = systemtime.wSecond;
	dt.dst = is_daylight;
	return dt;
}
}
