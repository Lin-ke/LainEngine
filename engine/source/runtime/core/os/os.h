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
	virtual u64 GetTimeUsec() const = 0;
	virtual DateTime GetDateTime(bool p_utc) const = 0;
	// Absolute path to res:// ( if not reload)
	virtual String GetResourceDir() const;

	// OS specific path for user://
	virtual String GetUserDataDir() const;

	static L_INLINE  OS* GetSingleton() {
		return p_singleton;
	}
	

	
};



class OSWin :public OS {
	u64 ticks_start = 0;
	u64 ticks_per_second = 0;
	public:
	virtual	u64 GetTimeUsec() const;
	virtual	DateTime GetDateTime(bool p_utc) const;

	virtual void Initialize();
	virtual void Finialize() {}
	virtual void Run();
	OSWin() {
		Log::Init();
	}

};

}


#endif // !__OS_H__

// 子类习惯上加virtual使更清晰