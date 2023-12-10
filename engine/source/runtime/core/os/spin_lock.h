#pragma once
#ifndef SPIN_LOCK_H
#define SPIN_LOCK_H

#include "core/typedefs.h"

#include <atomic>

class SpinLock {
	mutable std::atomic_flag locked = ATOMIC_FLAG_INIT;

public:
	_ALWAYS_INLINE_ void lock() const {
		while (locked.test_and_set(std::memory_order_acquire)) {
			// Continue.
		}
	}
	_ALWAYS_INLINE_ void unlock() const {
		locked.clear(std::memory_order_release);
	}
};

#endif // SPIN_LOCK_H