
#ifndef CONDITION_VARIABLE_H
#define CONDITION_VARIABLE_H

#include <condition_variable>
#include "base.h"
#include "mutex.h"
namespace lain {

// An object one or multiple threads can wait on a be notified by some other.
// Normally, you want to use a semaphore for such scenarios, but when the
// condition is something different than a count being greater than zero
// (which is the built-in logic in a semaphore) or you want to provide your
// own mutex to tie the wait-notify to some other behavior, you need to use this.
class ConditionVariable {
	mutable std::condition_variable condition;

public:
	template <class BinaryMutexT>
	L_INLINE void wait(const MutexLock<BinaryMutexT>& p_lock) const {
		condition.wait(const_cast<std::unique_lock<std::mutex> &>(p_lock.lock));
	}

	L_INLINE void notify_one() const {
		condition.notify_one();
	}

	L_INLINE void notify_all() const {
		condition.notify_all();
	}
};
}

#endif // CONDITION_VARIABLE_H

