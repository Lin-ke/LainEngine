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
	void __print_line(String p_string) {

		L_CORE_INFO("%s\n", p_string.utf8().get_data());

		_global_lock();
		PrintHandlerList* l = print_handler_list;
		while (l) {
			l->printfunc(l->userdata, p_string, false, false);
			l = l->next;
		}

		_global_unlock();
	}

	void __print_line_rich(String p_string) {
		// Convert a subset of BBCode tags to ANSI escape codes for correct display in the terminal.
		// Support of those ANSI escape codes varies across terminal emulators,
		// especially for italic and strikethrough.
		String p_string_ansi = p_string;

		p_string_ansi = p_string_ansi.replace("[b]", "\u001b[1m");
		p_string_ansi = p_string_ansi.replace("[/b]", "\u001b[22m");
		p_string_ansi = p_string_ansi.replace("[i]", "\u001b[3m");
		p_string_ansi = p_string_ansi.replace("[/i]", "\u001b[23m");
		p_string_ansi = p_string_ansi.replace("[u]", "\u001b[4m");
		p_string_ansi = p_string_ansi.replace("[/u]", "\u001b[24m");
		p_string_ansi = p_string_ansi.replace("[s]", "\u001b[9m");
		p_string_ansi = p_string_ansi.replace("[/s]", "\u001b[29m");

		p_string_ansi = p_string_ansi.replace("[indent]", "    ");
		p_string_ansi = p_string_ansi.replace("[/indent]", "");
		p_string_ansi = p_string_ansi.replace("[code]", "\u001b[2m");
		p_string_ansi = p_string_ansi.replace("[/code]", "\u001b[22m");
		p_string_ansi = p_string_ansi.replace("[url]", "");
		p_string_ansi = p_string_ansi.replace("[/url]", "");
		p_string_ansi = p_string_ansi.replace("[center]", "\n\t\t\t");
		p_string_ansi = p_string_ansi.replace("[/center]", "");
		p_string_ansi = p_string_ansi.replace("[right]", "\n\t\t\t\t\t\t");
		p_string_ansi = p_string_ansi.replace("[/right]", "");

		if (p_string_ansi.contains("[color")) {
			p_string_ansi = p_string_ansi.replace("[color=black]", "\u001b[30m");
			p_string_ansi = p_string_ansi.replace("[color=red]", "\u001b[91m");
			p_string_ansi = p_string_ansi.replace("[color=green]", "\u001b[92m");
			p_string_ansi = p_string_ansi.replace("[color=lime]", "\u001b[92m");
			p_string_ansi = p_string_ansi.replace("[color=yellow]", "\u001b[93m");
			p_string_ansi = p_string_ansi.replace("[color=blue]", "\u001b[94m");
			p_string_ansi = p_string_ansi.replace("[color=magenta]", "\u001b[95m");
			p_string_ansi = p_string_ansi.replace("[color=pink]", "\u001b[38;5;218m");
			p_string_ansi = p_string_ansi.replace("[color=purple]", "\u001b[38;5;98m");
			p_string_ansi = p_string_ansi.replace("[color=cyan]", "\u001b[96m");
			p_string_ansi = p_string_ansi.replace("[color=white]", "\u001b[97m");
			p_string_ansi = p_string_ansi.replace("[color=orange]", "\u001b[38;5;208m");
			p_string_ansi = p_string_ansi.replace("[color=gray]", "\u001b[90m");
			p_string_ansi = p_string_ansi.replace("[/color]", "\u001b[39m");
		}
		if (p_string_ansi.contains("[bgcolor")) {
			p_string_ansi = p_string_ansi.replace("[bgcolor=black]", "\u001b[40m");
			p_string_ansi = p_string_ansi.replace("[bgcolor=red]", "\u001b[101m");
			p_string_ansi = p_string_ansi.replace("[bgcolor=green]", "\u001b[102m");
			p_string_ansi = p_string_ansi.replace("[bgcolor=lime]", "\u001b[102m");
			p_string_ansi = p_string_ansi.replace("[bgcolor=yellow]", "\u001b[103m");
			p_string_ansi = p_string_ansi.replace("[bgcolor=blue]", "\u001b[104m");
			p_string_ansi = p_string_ansi.replace("[bgcolor=magenta]", "\u001b[105m");
			p_string_ansi = p_string_ansi.replace("[bgcolor=pink]", "\u001b[48;5;218m");
			p_string_ansi = p_string_ansi.replace("[bgcolor=purple]", "\u001b[48;5;98m");
			p_string_ansi = p_string_ansi.replace("[bgcolor=cyan]", "\u001b[106m");
			p_string_ansi = p_string_ansi.replace("[bgcolor=white]", "\u001b[107m");
			p_string_ansi = p_string_ansi.replace("[bgcolor=orange]", "\u001b[48;5;208m");
			p_string_ansi = p_string_ansi.replace("[bgcolor=gray]", "\u001b[100m");
			p_string_ansi = p_string_ansi.replace("[/bgcolor]", "\u001b[49m");
		}
		if (p_string_ansi.contains("[fgcolor")) {
			p_string_ansi = p_string_ansi.replace("[fgcolor=black]", "\u001b[30;40m");
			p_string_ansi = p_string_ansi.replace("[fgcolor=red]", "\u001b[91;101m");
			p_string_ansi = p_string_ansi.replace("[fgcolor=green]", "\u001b[92;102m");
			p_string_ansi = p_string_ansi.replace("[fgcolor=lime]", "\u001b[92;102m");
			p_string_ansi = p_string_ansi.replace("[fgcolor=yellow]", "\u001b[93;103m");
			p_string_ansi = p_string_ansi.replace("[fgcolor=blue]", "\u001b[94;104m");
			p_string_ansi = p_string_ansi.replace("[fgcolor=magenta]", "\u001b[95;105m");
			p_string_ansi = p_string_ansi.replace("[fgcolor=pink]", "\u001b[38;5;218;48;5;218m");
			p_string_ansi = p_string_ansi.replace("[fgcolor=purple]", "\u001b[38;5;98;48;5;98m");
			p_string_ansi = p_string_ansi.replace("[fgcolor=cyan]", "\u001b[96;106m");
			p_string_ansi = p_string_ansi.replace("[fgcolor=white]", "\u001b[97;107m");
			p_string_ansi = p_string_ansi.replace("[fgcolor=orange]", "\u001b[38;5;208;48;5;208m");
			p_string_ansi = p_string_ansi.replace("[fgcolor=gray]", "\u001b[90;100m");
			p_string_ansi = p_string_ansi.replace("[/fgcolor]", "\u001b[39;49m");
		}

		L_CORE_INFO("%s\n", p_string_ansi.utf8().get_data());

		_global_lock();
		PrintHandlerList* l = print_handler_list;
		while (l) {
			l->printfunc(l->userdata, p_string, false, true);
			l = l->next;
		}

		_global_unlock();
	}
}