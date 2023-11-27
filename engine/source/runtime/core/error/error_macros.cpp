
#include "error_macros.h"
#include "core/os/mutex.h"
#include "base.h"


static ErrorHandlerList* error_handler_list = nullptr;

void add_error_handler(ErrorHandlerList* p_handler) {
	// If p_handler is already in error_handler_list
	// we'd better remove it first then we can add it.
	// This prevent cyclic redundancy.
	remove_error_handler(p_handler);

	_global_lock();

	p_handler->next = error_handler_list;
	error_handler_list = p_handler;

	_global_unlock();
}

void remove_error_handler(const ErrorHandlerList* p_handler) {
	_global_lock();

	ErrorHandlerList* prev = nullptr;
	ErrorHandlerList* l = error_handler_list;

	while (l) {
		if (l == p_handler) {
			if (prev) {
				prev->next = l->next;
			}
			else {
				error_handler_list = l->next;
			}
			break;
		}
		prev = l;
		l = l->next;
	}

	_global_unlock();
}

// Errors without messages.
void _err_print_error(const char* p_function, const char* p_file, int p_line, const char* p_error, bool p_editor_notify, ErrorHandlerType p_type) {
	_err_print_error(p_function, p_file, p_line, p_error, "", p_editor_notify, p_type);
}

void _err_print_error(const char* p_function, const char* p_file, int p_line, const String& p_error, bool p_editor_notify, ErrorHandlerType p_type) {
	_err_print_error(p_function, p_file, p_line, p_error.utf8().get_data(), "", p_editor_notify, p_type);
}

// Main error printing function.

void _err_print_error(const char* p_function, const char* p_file, int p_line, const String& p_error, bool p_editor_notify, ErrorHandlerType p_type) {
	_err_print_error(p_function, p_file, p_line, p_error.utf8().get_data(), "", p_editor_notify, p_type);
}
void _err_print_error(const char* p_function, const char* p_file, int p_line, const String& p_error, const String& p_message, bool p_editor_notify, ErrorHandlerType p_type) {
	_err_print_error(p_function, p_file, p_line, p_error.utf8().get_data(), p_message.utf8().get_data(), p_editor_notify, p_type);
}

void _err_print_error(const char* p_function, const char* p_file, int p_line, const char* p_error, const char* p_message, bool p_editor_notify, ErrorHandlerType p_type) {
	
	L_PERROR("Func:", p_function, "File:", p_file, "L:", p_line, "ERR:", p_error, "MSG:", p_message, "NTF:", p_editor_notify, "TP:",p_type);

	_global_lock();
	ErrorHandlerList* l = error_handler_list;
	while (l) {
		l->errfunc(l->userdata, p_function, p_file, p_line, p_error, p_message, p_editor_notify, p_type);
		l = l->next;
	}

	_global_unlock();
}

// Errors with message. (All combinations of p_error and p_message as String or char*.)
//void _err_print_error(const char* p_function, const char* p_file, int p_line, const String& p_error, const char* p_message, bool p_editor_notify, ErrorHandlerType p_type) {
//	_err_print_error(p_function, p_file, p_line, p_error, p_message, p_editor_notify, p_type);
//}

//void _err_print_error(const char* p_function, const char* p_file, int p_line, const char* p_error, const String& p_message, bool p_editor_notify, ErrorHandlerType p_type) {
//	_err_print_error(p_function, p_file, p_line, p_error, p_message, p_editor_notify, p_type);
//}
//
//void _err_print_error(const char* p_function, const char* p_file, int p_line, const String& p_error, const String& p_message, bool p_editor_notify, ErrorHandlerType p_type) {
//	_err_print_error(p_function, p_file, p_line, p_error, p_message, p_editor_notify, p_type);
//}

// Index errors. (All combinations of p_message as String or char*.)
void _err_print_index_error(const char* p_function, const char* p_file, int p_line, int64_t p_index, int64_t p_size, const char* p_index_str, const char* p_size_str, const char* p_message, bool p_editor_notify, bool p_fatal) {
	String fstr(p_fatal ? "FATAL: " : "");
	String err(fstr + "Index " + p_index_str + " = " + lain::itos(p_index) + " is out of bounds (" + p_size_str + " = " + lain::itos(p_size) + ").");
	_err_print_error(p_function, p_file, p_line, err.utf8().get_data(), p_message, p_editor_notify, ERR_HANDLER_ERROR);
}

//void _err_print_index_error(const char* p_function, const char* p_file, int p_line, int64_t p_index, int64_t p_size, const char* p_index_str, const char* p_size_str, const String& p_message, bool p_editor_notify, bool p_fatal) {
//	_err_print_index_error(p_function, p_file, p_line, p_index, p_size, p_index_str, p_size_str, p_message, p_editor_notify, p_fatal);
//}

void _err_flush_stdout() {
	fflush(stdout);
}
