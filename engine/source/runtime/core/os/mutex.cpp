
#include "mutex.h"

static Mutex _global_mutex;
// È«¾ÖËø

void _global_lock() {
	_global_mutex.lock();
}

void _global_unlock() {
	_global_mutex.unlock();
}

template class MutexImpl<std::recursive_mutex>;
template class MutexImpl<std::mutex>;
template class MutexLock<MutexImpl<std::recursive_mutex>>;
template class MutexLock<MutexImpl<std::mutex>>;
