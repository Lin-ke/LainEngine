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
	// ����ͨ������ post �������� count ��ֵ��֪ͨ�������п��õ����ݡ������߿���ͨ���ȴ���wait���������ȴ��������ͷ���Դ��ÿ�ε��� post �������������� count ��ֵ���Ӷ��ṩ�������߸������Դ��
	// ͨ��ʹ���ź��������ǿ�������ͬʱ���ʹ�����Դ�������������߳���������������-������ģʽ�У������ߺ���������Ҫ����ط��ʻ��������Ա��⾺�����������ݲ�һ���Ե����⡣
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
		std::lock_guard lock(mutex); // ��mutex��������ʱ����
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
