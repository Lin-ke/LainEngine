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

class Semaphore {
	// count semaphore
	mutable THREADING_NAMESPACE::mutex mutex;
	mutable THREADING_NAMESPACE::condition_variable condition;
	mutable uint32_t count = 0; // Initialized as locked.
#ifdef DEBUG_ENABLED
	mutable uint32_t awaiters = 0;
#endif
	// 可以通过调用 post 函数增加 count 的值来通知消费者有可用的数据。消费者可以通过等待（wait）操作来等待生产者释放资源。每次调用 post 函数，都会增加 count 的值，从而提供给消费者更多的资源。
	// 通过使用信号量，我们可以限制同时访问共享资源（缓冲区）的线程数量。在生产者-消费者模式中，生产者和消费者需要交替地访问缓冲区，以避免竞争条件和数据不一致性的问题。
	_ALWAYS_INLINE_ void post(uint32_t p_count = 1) const {
		std::lock_guard lock(mutex);
		count += p_count;
		for (uint32_t i = 0; i < p_count; ++i) {
			condition.notify_one();
		}
	}

	_ALWAYS_INLINE_ void wait() const {
		THREADING_NAMESPACE::unique_lock lock(mutex);
#ifdef DEBUG_ENABLED
		++awaiters;
#endif
		while (!count) { // Handle spurious wake-ups.
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
#endif
