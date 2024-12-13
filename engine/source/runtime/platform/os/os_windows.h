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
	virtual	ui64 GetTicksUsec() const;
	virtual double GetUnixTime() const;
	virtual	DateTime GetDateTime(bool p_utc) const;

	virtual void Initialize();
	virtual void Finialize() {}
	virtual void Run();
	virtual void SetMainLoop(MainLoop* p_main_loop) override;
	virtual MainLoop* GetMainLoop() const override;
	MainLoop* main_loop = nullptr;


	virtual String GetConfigPath() const override {
		static String _configpath = HasEnv("APPDATA") ? GetEnv("APPDATA") : ".";
		return _configpath;
	}
	virtual String GetDataPath() const override{
		return GetConfigPath();
	}
	virtual String GetCachePath() const override;
	// requires projectsettings.singleton
	virtual String GetUserDataDir() const override;
	virtual Error GetEntropy(uint8_t* r_buffer, size_t p_bytes) const override;
	virtual void SetEnvironment(const String&, const String&) const override;

	OSWindows() {
		Log::Initialize();
	}
	// env
	bool HasEnv(const String &name) const;
	String GetEnv(const String &name) const ;

	// sleep
	virtual void DelayUsec(uint32_t p_usec) const override;
	virtual Error SetCwd(const String& p_path) override;
};
}

#endif // !__OS_WINDOWS_H__
