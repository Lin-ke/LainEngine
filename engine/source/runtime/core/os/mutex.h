#ifndef MUTEX_H
#define MUTEX_H

#include "core/error/error_macros.h"
#include "core/typedefs.h"
#include "base.h"
#include <mutex>
namespace lain {

template <class MutexT>
class MutexLock;


// Impl设计模式，接口封装，对底层隐藏细节（具体是哪种锁）
// 鸭子类型
template <class StdMutexT>
class MutexImpl {
	friend class MutexLock<MutexImpl<StdMutexT>>;
	// alias : stdMutexType
	using StdMutexType = StdMutexT;

	mutable StdMutexT mutex;

public:
	_ALWAYS_INLINE_ void lock() const {
		mutex.lock();
	}

	_ALWAYS_INLINE_ void unlock() const {
		mutex.unlock();
	}

	_ALWAYS_INLINE_ bool try_lock() const {
		return mutex.try_lock();
	}
};

// A very special kind of mutex, used in scenarios where these
// requirements hold at the same time:
// - Must be used with a condition variable (only binary mutexes are suitable).
// - Must have recursive semnantics (or simulate, as this one does).
// The implementation keeps the lock count in TS. Therefore, only
// one object of each version of the template can exists; hence the Tag argument.
// Tags must be unique across the Godot codebase.
// Also, don't forget to declare the thread_local variable on each use.
// 像是std::recursive_mutex
template <int Tag>
class SafeBinaryMutex {
	friend class MutexLock<SafeBinaryMutex>;

	using StdMutexType = THREADING_NAMESPACE::mutex;

	mutable THREADING_NAMESPACE::mutex mutex;
	static thread_local uint32_t count; 

public:
	L_INLINE void lock() const {
		if (++count == 1) {
			mutex.lock();
		}
	}

	L_INLINE void unlock() const {
		DEV_ASSERT(count);
		if (--count == 0) {
			mutex.unlock();
		}
	}

	L_INLINE bool try_lock() const {
		if (count) {
			count++;
			return true;
		}
		else {
			if (mutex.try_lock()) {
				count++;
				return true;
			}
			else {
				return false;
			}
		}
	}

	~SafeBinaryMutex() {
		DEV_ASSERT(!count);
	}
};

// wrapper of lock
template <class MutexT>
class MutexLock {
	friend class ConditionVariable;

	std::unique_lock<typename MutexT::StdMutexType> lock; 

public:
	// explicit constructor
	_ALWAYS_INLINE_ explicit MutexLock(const MutexT& p_mutex) :
		lock(p_mutex.mutex) {};
};

// This specialization is needed so manual locking and MutexLock can be used
// at the same time on a SafeBinaryMutex.
// 模板特化以支持safe_binary_mutex.lock()

template <int Tag>
class MutexLock<SafeBinaryMutex<Tag>> {
	friend class ConditionVariable;

	std::unique_lock<std::mutex> lock;

public:
	_ALWAYS_INLINE_ explicit MutexLock(const SafeBinaryMutex<Tag>& p_mutex) :
		lock(p_mutex.mutex) {
		SafeBinaryMutex<Tag>::count++;
	};
	_ALWAYS_INLINE_ ~MutexLock() {
		SafeBinaryMutex<Tag>::count--;
	};
};

using Mutex = MutexImpl<std::recursive_mutex>; // Recursive, for general use
using BinaryMutex = MutexImpl<std::mutex>; // Non-recursive, handle with care

extern template class MutexImpl<std::recursive_mutex>;
extern template class MutexImpl<std::mutex>;
extern template class MutexLock<MutexImpl<std::recursive_mutex>>;
extern template class MutexLock<MutexImpl<std::mutex>>;
} // namespace lain

#endif // MUTEX_H
