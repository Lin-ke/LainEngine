#include "dir_access_windows.h"
namespace lain {
	bool DirAccessWindows::file_exists(String p_file) {

			if (!p_file.is_absolute_path()) {
				p_file = get_current_dir().path_join(p_file);
			}

		p_file = fix_path(p_file);

		DWORD fileAttr;

		fileAttr = GetFileAttributesW((LPCWSTR)(p_file.utf16().get_data()));
		if (INVALID_FILE_ATTRIBUTES == fileAttr) {
			return false;
		}

		return !(fileAttr & FILE_ATTRIBUTE_DIRECTORY);
	}
	Error DirAccessWindows::make_dir(String p_dir) {

			p_dir = fix_path(p_dir);
		if (p_dir.is_relative_path()) {
			p_dir = current_dir.path_join(p_dir);
			p_dir = fix_path(p_dir);
		}

		p_dir = p_dir.simplify_path().replace("/", "\\");

		bool success;
		int err;

		success = CreateDirectoryW((LPCWSTR)(p_dir.utf16().get_data()), nullptr);
		err = GetLastError();

		if (success) {
			return OK;
		}

		if (err == ERROR_ALREADY_EXISTS || err == ERROR_ACCESS_DENIED) {
			return ERR_ALREADY_EXISTS;
		}

		return ERR_CANT_CREATE;
	}
	String DirAccessWindows::get_current_dir(bool p_include_drive) const {
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