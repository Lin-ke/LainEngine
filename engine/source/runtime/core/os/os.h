#pragma once
#ifndef __OS_H__
#define __OS_H__
#include "runtime/base.h"
#include "memory.h"
#include "thread_safe.h"
#include "time_enums.h"
#include "core/config/project_settings.h"

// abstruct of the OS
namespace lain {

class OS {
public:
	static OS* p_singleton;
	String m_execpath;
	
	virtual void Run() = 0;
	virtual void Finialize() = 0;
	virtual void Initialize() = 0;
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
	virtual DateTime GetDateTime(bool p_utc) const = 0;
	// Absolute path to res:// ( if not reload)
	virtual String GetResourceDir() const;

	// OS specific path for user://
	virtual String GetUserDataDir() const;
	virtual String GetCachePath() const;

	virtual String GetExecutablePath() const { return m_execpath; }

	static L_INLINE  OS* GetSingleton() {
		return p_singleton;
	}
	virtual void DelayUsec(ui32 p_usec) const = 0 ;
	virtual String GetDataPath() { return "/"; }
	
	virtual String GetConfigPath() const { return "."; }
	
	String GetSafeDirName(const String& p_dir_name, bool p_allow_paths) const;
	int GetProcessorCount() const;
	int GetDefaultThreadPoolSize() const { return GetProcessorCount(); }

};


}

// It is customary to add virtual in subclasses to make it clearer ( no is OK)
#endif // !__OS_H__

