#pragma once
#ifndef __OS_H__
#define __OS_H__
#include "runtime/base.h"
#include "core/templates/singleton.h"
#include "memory.h"
#include "thread_safe.h"
#include "time_enums.h"

// abstruct of the OS
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
	virtual DateTime GetDateTime() const = 0;
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
		lain::Log::Init();
	}

};



#endif // !__OS_H__

// 子类习惯上加virtual使更清晰