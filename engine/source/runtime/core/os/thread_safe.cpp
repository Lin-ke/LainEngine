#include "thread_safe.h"
namespace lain{

static thread_local bool current_thread_safe_for_nodes = false;

bool is_current_thread_safe_for_nodes() {
	return current_thread_safe_for_nodes;
}

void set_current_thread_safe_for_nodes(bool p_safe) {
	current_thread_safe_for_nodes = p_safe;
}


}
