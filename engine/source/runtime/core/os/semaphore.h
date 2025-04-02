#pragma once
#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#include <cstdint>
#include "core/typedefs.h"

#ifdef DEBUG_ENABLED
#include "core/error/error_macros.h"
#endif
#include <condition_variable>
#include <mutex>
namespace lain {
// Semaphore 是用条件变量实现的
class Semaphore {
	// 通常会将互斥锁（mutex）声明为可变（mutable），以便在 const 成员函数中对其进行加锁和解锁操作
	mutable THREADING_NAMESPACE::mutex mutex;
	mutable THREADING_NAMESPACE::condition_variable condition;
	mutable uint32_t count = 0; // Initialized as locked.
	// mutable:可能被其他线程修改
#ifdef DEBUG_ENABLED
	mutable uint32_t awaiters = 0;
#endif
public:
	// 用lock_guard和unique_lock锁住count资源
	_ALWAYS_INLINE_ void post(uint32_t p_count = 1) const {
		std::lock_guard lock(mutex);
		count += p_count;
		// 否则就是notify 1
		for (uint32_t i = 0; i < p_count; ++i) {
			condition.notify_one();
		}
	}

	// 没有资源，等 （但是同时加着锁，只有一个线程可以进入临界区）

	_ALWAYS_INLINE_ void wait() const {
		THREADING_NAMESPACE::unique_lock lock(mutex);
#ifdef DEBUG_ENABLED
		++awaiters;
#endif
		while (!count) { // 虚假唤醒！
			condition.wait(lock);
		}
		--count;
#ifdef DEBUG_ENABLED
		--awaiters;
#endif
	}
	_ALWAYS_INLINE_ bool try_wait() const {
		std::lock_guard lock(mutex); // 锁mutex，在析构时解锁
		if (count) {
			count--;
			return true;
		}
		else {
			return false;
		}
	}
};
}

#endif
