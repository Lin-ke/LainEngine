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

class Semaphore {
	// ͨ���Ὣ��������mutex������Ϊ�ɱ䣨mutable�����Ա��� const ��Ա�����ж�����м����ͽ�������
	mutable THREADING_NAMESPACE::mutex mutex;
	mutable THREADING_NAMESPACE::condition_variable condition;
	mutable uint32_t count = 0; // Initialized as locked.
	// mutable:���ܱ������߳��޸�
#ifdef DEBUG_ENABLED
	mutable uint32_t awaiters = 0;
#endif
public:
	// ��lock_guard��unique_lock��סcount��Դ
	_ALWAYS_INLINE_ void post(uint32_t p_count = 1) const {
		std::lock_guard lock(mutex);
		count += p_count;
		// �������notify 1
		for (uint32_t i = 0; i < p_count; ++i) {
			condition.notify_one();
		}
	}

	// û����Դ���� ������ͬʱ��������ֻ��һ���߳̿��Խ����ٽ�����

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
}

#endif
