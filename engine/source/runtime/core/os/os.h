#pragma once
#ifndef __OS_H__
#define __OS_H__
#include "runtime/base.h"
#include "core/templates/singleton.h"
#include "memory.h"
#include "thread_safe.h"
#include "core/mainloop/main.h"
// abstruct of the OS
// 有一个或多个纯虚函数就成为抽象类
class OS {
	// 这种方式必须保证OS的调用是单线程的。
	// 好处：可以通过OS->实现多态。
	static OS* p_singleton;
	
public:
	virtual void Run() = 0;
	virtual void Finialize() = 0;
	virtual void Initialize() = 0;
	// logger default

	virtual void Addlogger() = 0;
	OS() {
		p_singleton = this;
		lain::Log::Init();
	}
	// 多态需要虚析构函数（并总被解释为虚函数）
	virtual ~OS() {
		p_singleton = nullptr;
	}
	
	L_INLINE OS* GetSingleton() {
		return p_singleton;
	}

};
class OSWin :public OS {
	virtual void Run() {
		while (true) {
			//人机交互
			// display.send_events;
			if (!Main::loop()) {
				break;
			}

			
		}
	}
};


#endif // !__OS_H__

