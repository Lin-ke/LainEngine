#pragma once
#ifndef __OS_WINDOWS_H__
#define __OS_WINDOWS_H__
#include "core/os/os.h"
namespace lain {

class OSWindows :public OS {
	ui64 ticks_start = 0;
	ui64 ticks_per_second = 0;
public:
	virtual	ui64 GetTimeUsec() const;
	virtual	DateTime GetDateTime(bool p_utc) const;

	virtual void Initialize();
	virtual void Finialize() {}
	virtual void Run();
	

	virtual String GetConfigPath() const {
		static String _configpath = HasEnv("APPDATA") ? GetEnv("APPDATA") : ".";
		return _configpath;
	}
	virtual String GetDataPath() const {
		return GetConfigPath();
	}
	virtual String GetCachePath() const;
	virtual String GetUserDataDir() const;
	OSWindows() {
		Log::Initialize();
	}
	// env
	bool HasEnv(const String &name) const;
	String GetEnv(const String &name) const ;

	// sleep
	virtual void DelayUsec(ui32 p_usec) const override;
	virtual Error SetCwd(const String& p_path) override;
};
}

#endif // !__OS_WINDOWS_H__
