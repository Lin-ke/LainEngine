#pragma once
#ifndef __OS_H__
#define __OS_H__
#include "runtime/base.h"
#include "memory.h"
#include "thread_safe.h"
#include "time_enums.h"
#include "core/config/project_settings.h"
#include "core/os/main_loop.h"

// abstruct of the OS
namespace lain {

class OS {
public:
	static OS* p_singleton;
	String m_execpath;
	
	virtual void Run() = 0;
	virtual void Finialize() = 0;
	virtual void Initialize() = 0;
	virtual void SetMainLoop(MainLoop* p_main_loop) = 0;

	// logger default

	virtual void Addlogger() {}
	OS() {
		p_singleton = this;
		
	}
	struct DateTime {
		int64_t year;
		Month month;
		uint8_t day;
		Weekday weekday;
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
		bool dst;
	};
	// need virtual
	virtual ~OS() {
		p_singleton = nullptr;
	}
	virtual ui64 GetTimeUsec() const = 0;
	virtual ui64 GetTicksUsec() const = 0;
	virtual double GetUnixTime() const { return 0; }
	virtual DateTime GetDateTime(bool p_utc = false) const = 0;
	// Absolute path to res:// ( if not reload)
	virtual String GetResourceDir() const;

	// OS specific path for user://
	virtual String GetUserDataDir() const;
	virtual String GetCachePath() const;

	virtual String GetExecutablePath() const { return m_execpath; }

	static L_INLINE  OS* GetSingleton() {
		return p_singleton;
	}
	virtual void DelayUsec(uint32_t p_usec) const = 0 ;
	virtual String GetDataPath() const { return "/"; }
	
	virtual String GetConfigPath() const { return "."; }
	virtual Error SetCwd(const String& path) { return ERR_CANT_OPEN; }
	// º”√‹
	virtual Error GetEntropy(uint8_t* r_buffer, size_t p_bytes) const = 0;
	
	String GetSafeDirName(const String& p_dir_name, bool p_allow_paths) const;
	int GetProcessorCount() const;
	int GetDefaultThreadPoolSize() const { return GetProcessorCount(); }


	void EnsureUserDataDir();
};


}

// It is customary to add virtual in subclasses to make it clearer ( no is OK)
#endif // !__OS_H__

