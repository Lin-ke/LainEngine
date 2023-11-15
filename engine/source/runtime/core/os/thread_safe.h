#pragma once
#ifndef __THREAD_SAFE_H__
#define __THREAD_SAFE_H__
#include "core/os/mutex.h"
#define _THREAD_SAFE_CLASS_ mutable Mutex m_thread_safe;
#define _THREAD_SAFE_CLASS_ mutable Mutex _thread_safe_;
#define _THREAD_SAFE_LOCK_ _thread_safe_.lock();
#define _THREAD_SAFE_UNLOCK_ _thread_safe_.unlock();
#endif // !__THREAD_SAFE_H__
