#include "thread.h"
#include "core/templates/safe_numeric.h"
#include "core/string/ustring.h"
namespace lain {
	SafeNumeric<uint64_t> Thread::id_counter(1); // The first value after .increment() is 2, hence by default the main thread ID should be 1.
	thread_local Thread::ID Thread::caller_id = Thread::UNASSIGNED_ID;

}