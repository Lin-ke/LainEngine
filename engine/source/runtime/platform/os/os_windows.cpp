#include "os_windows.h"
#include <timeapi.h>
#include "core/mainloop/main.h"
#include "function/display/window_system.h"
#include "core/config/project_settings.h"
#include "platform/io/file_access_windows.h"
#include "platform/io/dir_access_windows.h"
#include <bcrypt.h>

namespace lain {
	bool OSWindows::HasEnv(const String& p_var) const {
#ifdef MINGW_ENABLED
		return _wgetenv((LPCWSTR)(p_var.utf16().get_data())) != nullptr;
#else
		WCHAR* env;
		size_t len;
		_wdupenv_s(&env, &len, (LPCWSTR)(p_var.utf16().get_data()));
		const bool has_env = env != nullptr;
		free(env);
		return has_env;
#endif
	}
	String OSWindows::GetEnv(const String& p_var) const {
		WCHAR wval[0x7fff]; // MSDN says 32767 char is the maximum
		int wlen = GetEnvironmentVariableW((LPCWSTR)(p_var.utf16().get_data()), wval, 0x7fff);
		if (wlen > 0) {
			return String::utf16((const char16_t*)wval);
		}
		return "";
	}

	String OSWindows::GetCachePath() const {
		static String cache_path_cache;
		if (cache_path_cache.is_empty()) {
			if (HasEnv("LOCALAPPDATA")) {
				cache_path_cache = GetEnv("LOCALAPPDATA").replace("\\", "/");
			}
			if (cache_path_cache.is_empty() && HasEnv("TEMP")) {
				cache_path_cache = GetEnv("TEMP").replace("\\", "/");
			}
			if (cache_path_cache.is_empty()) {
				cache_path_cache = GetConfigPath();
			}
		}
		return cache_path_cache;
	}
	String OSWindows::GetUserDataDir() const {
		String appname = GetSafeDirName(GLOBAL_GET("application/config/name"), true);
		if (!appname.is_empty()) {
			return GetDataPath().path_join("Lain").path_join("app_userdata").path_join(appname).replace("\\", "/");
		}
		return GetDataPath().path_join("Lain").path_join("app_userdata").path_join("[unnamed project]");
	}


	void OSWindows::Run() {
		// init
		while (true) {
			//人机交互
			// display.send_events;
			bool exit = WindowSystem::GetSingleton()->ShouldClose();


			if (!Main::Loop() || exit) {
				break;
			}
			WindowSystem::GetSingleton()->PollEvents();

		}
		L_PRINT("Exiting. Have a nice day.");

	}
	// 填必要的环境变量、文件操作
	void OSWindows::Initialize() {
		
		// 在这里初始化，将FileAccess类的create_function函数数组放入make_default(memnew)<T> 工厂
		// 这样以后在fileaccess里执行create()得到的就是对应的子类
		// 而Ref<> = 

		FileAccess::make_default<FileAccessWindows>(FileAccess::ACCESS_RESOURCES);
		FileAccess::make_default<FileAccessWindows>(FileAccess::ACCESS_USERDATA);
		FileAccess::make_default<FileAccessWindows>(FileAccess::ACCESS_FILESYSTEM);
		DirAccess::make_default<DirAccessWindows>(DirAccess::ACCESS_RESOURCES);
		DirAccess::make_default<DirAccessWindows>(DirAccess::ACCESS_USERDATA);
		DirAccess::make_default<DirAccessWindows>(DirAccess::ACCESS_FILESYSTEM);
		QueryPerformanceFrequency((LARGE_INTEGER*)&ticks_per_second);
		QueryPerformanceCounter((LARGE_INTEGER*)&ticks_start);
		//L_PRINT(ticks_per_second, ticks_start);
		timeBeginPeriod(1);
		
	}


	uint64_t OSWindows::GetTimeUsec() const {
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
	OS::DateTime OSWindows::GetDateTime(bool p_utc) const {
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
	void OSWindows::DelayUsec(ui32 p_usec) const{
		if (p_usec < 1000) {
				Sleep(1);
		}
		else {
			Sleep(p_usec / 1000);
		}
	}

	Error OSWindows::SetCwd(const String& p_cwd) {
		if (_wchdir((LPCWSTR)(p_cwd.utf16().get_data())) != 0) {
			return ERR_CANT_OPEN;
		}

		return OK;
	}
	Error OSWindows::GetEntropy(uint8_t* r_buffer, int p_bytes) const  {
		NTSTATUS status = BCryptGenRandom(nullptr, r_buffer, p_bytes, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
		ERR_FAIL_COND_V(status, FAILED);
		return OK;
	}
}