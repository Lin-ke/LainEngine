#include "thread.h"
#include "core/templates/safe_numeric.h"
#include "core/string/ustring.h"
namespace lain {
	SafeNumeric<uint64_t> Thread::id_counter(1); // The first value after .increment() is 2, hence by default the main thread ID should be 1.
	thread_local Thread::ID Thread::caller_id = Thread::UNASSIGNED_ID;
	// 也是一种鸭子的方法
	void Thread::callback(ID p_caller_id, const Settings& p_settings, Thread::Callback p_callback, void* p_userdata) {
		Thread::caller_id = p_caller_id;
		if (platform_functions.set_priority) {
			platform_functions.set_priority(p_settings.priority);
		}
		if (platform_functions.init) {
			platform_functions.init();
		}
		if (platform_functions.wrapper) {
			platform_functions.wrapper(p_callback, p_userdata);
		}
		else {
			p_callback(p_userdata);
		}
		if (platform_functions.term) {
			platform_functions.term();
		}

	}
	Thread::ID Thread::start(Thread::Callback p_callback, void* p_user, const Settings& p_settings) {
		ERR_FAIL_COND_V_MSG(id != UNASSIGNED_ID, UNASSIGNED_ID, "A Thread object has been re-started without wait_to_finish() having been called on it.");
		id = id_counter.increment();
		// std::thread(&function, parameters, ...)
		thread = THREADING_NAMESPACE::thread(&Thread::callback, id, p_settings, p_callback, p_user);
		return id;
	}
	bool Thread::is_started() const {
		return id != UNASSIGNED_ID;
	}
	Error Thread::set_name(const String& p_name) {
		if (platform_functions.set_name) {
			return platform_functions.set_name(p_name);
		}

		return ERR_UNAVAILABLE;
	}
	void Thread::_set_platform_functions(const PlatformFunctions& p_functions) {
		platform_functions = p_functions;
	}
	void Thread::wait_to_finish() {
		ERR_FAIL_COND_MSG(id == UNASSIGNED_ID, "Attempt of waiting to finish on a thread that was never started.");
		ERR_FAIL_COND_MSG(id == get_caller_id(), "Threads can't wait to finish on themselves, another thread must wait.");
		thread.join();
		thread = THREADING_NAMESPACE::thread();
		id = UNASSIGNED_ID;
	}

	Thread::Thread() {
	}

	Thread::~Thread() {
		if (id != UNASSIGNED_ID) {
#ifdef L_DEBUG
			L_CORE_WARN(
				"A Thread object is being destroyed without its completion having been realized.\n"
				"Please call wait_to_finish() on it to ensure correct cleanup.");
#endif
			thread.detach();
		}
	}
}