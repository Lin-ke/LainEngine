#pragma once
#ifndef __OS_H__
#define __OS_H__
#include "runtime/base.h"
#include "core/templates/singleton.h"
#include "memory.h"
#include "thread_safe.h"
#include "core/mainloop/main.h"
// abstruct of the OS
// ��һ���������麯���ͳ�Ϊ������
class OS {
	// ���ַ�ʽ���뱣֤OS�ĵ����ǵ��̵߳ġ�
	// �ô�������ͨ��OS->ʵ�ֶ�̬��
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
	// ��̬��Ҫ���������������ܱ�����Ϊ�麯����
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
			//�˻�����
			// display.send_events;
			if (!Main::loop()) {
				break;
			}

			
		}
	}
};


#endif // !__OS_H__

