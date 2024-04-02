#include "mutex.h"
/// <summary>
/// allows to lock and unlock more than one times.
/// </summary>
namespace lain {

	static Mutex _global_mutex;



	template class MutexImpl<std::recursive_mutex>;
	template class MutexImpl<std::mutex>;
	template class MutexLock<MutexImpl<std::recursive_mutex>>;
	template class MutexLock<MutexImpl<std::mutex>>;
}

void _global_lock() {
	lain::_global_mutex.lock();
}

void _global_unlock() {
	lain::_global_mutex.unlock();
}
