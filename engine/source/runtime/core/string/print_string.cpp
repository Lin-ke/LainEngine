#include "print_string.h"
namespace lain {
	static PrintHandlerList* print_handler_list = nullptr;
	void print_error(String p_string) {
		L_ERROR(p_string.utf8().get_data());

		_global_lock();
		PrintHandlerList* l = print_handler_list;
		while (l) {
			l->printfunc(l->userdata, p_string, true, false);
			l = l->next;
		}

		_global_unlock();
	}
}