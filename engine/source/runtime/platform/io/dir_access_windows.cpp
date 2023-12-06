#include "dir_access_windows.h"
namespace lain {
	String DirAccessWin::get_current_dir(bool p_include_drive) const {
		String base = _get_root_path();
		if (!base.is_empty()) {
			String bd = current_dir.replace("\\", "/").replace_first(base, "");
			if (bd.begins_with("/")) {
				return _get_root_string() + bd.substr(1, bd.length());
			}
			else {
				return _get_root_string() + bd;
			}
		}

		if (p_include_drive) {
			return current_dir;
		}
		else {
			if (_get_root_string().is_empty()) {
				int pos = current_dir.find(":");
				if (pos != -1) {
					return current_dir.substr(pos + 1);
				}
			}
			return current_dir;
		}
	}
}