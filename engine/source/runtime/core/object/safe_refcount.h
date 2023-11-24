#pragma once
#ifndef __SAFE_REFCOUNT_H__
#define __SAFE_REFCOUNT_H__
#include "base.h"
// 引用计数
//#include <memory>
//template <typename T>
//using Ref = std::shared_ptr<T>;

#ifdef DEV_ENABLED
#include "core/error/error_macros.h"
#endif


class SafeRefCount {
	s_u32 count;

#ifdef DEV_ENABLED

	_ALWAYS_INLINE_ void _check_unref_sanity() {
		// This won't catch every misuse, but it's better than nothing.
		CRASH_COND_MSG(count.get() == 0,
			"Trying to unreference a SafeRefCount which is already zero is wrong and a symptom of it being misused.\n"
			"Upon a SafeRefCount reaching zero any object whose lifetime is tied to it, as well as the ref count itself, must be destroyed.\n"
			"Moreover, to guarantee that, no multiple threads should be racing to do the final unreferencing to zero.");
	}
#endif

public:
	_ALWAYS_INLINE_ bool ref() { // true on success
		return count.conditional_increment() != 0;
	}

	_ALWAYS_INLINE_ uint32_t refval() { // none-zero on success
		return count.conditional_increment();
	}

	_ALWAYS_INLINE_ bool unref() { // true if must be disposed of
#ifdef DEV_ENABLED
		_check_unref_sanity();
#endif
		return count.decrement() == 0;
	}

	_ALWAYS_INLINE_ uint32_t unrefval() { // 0 if must be disposed of
#ifdef DEV_ENABLED
		_check_unref_sanity();
#endif
		return count.decrement();
	}

	_ALWAYS_INLINE_ uint32_t get() const {
		return count.get();
	}

	_ALWAYS_INLINE_ void init(uint32_t p_value = 1) {
		count.set(p_value);
	}
};

#endif // SAFE_REFCOUNT_H
