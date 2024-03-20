#include "dir_access_windows.h"
#include "file_access_windows.h"

#include "core/os/memory.h"
#include <stdio.h>
#include <wchar.h>
#include <minwinbase.h>
#include <winnt.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
namespace lain {
	struct DirAccessWindowsPrivate {
		HANDLE h; // handle for FindFirstFile.
		WIN32_FIND_DATA f;
		WIN32_FIND_DATAW fu; // Unicode version.
	};
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

	DirAccessWindows::DirAccessWindows() {
		p = memnew(DirAccessWindowsPrivate);
		p->h = INVALID_HANDLE_VALUE;
		current_dir = ".";

		DWORD mask = GetLogicalDrives();

		for (int i = 0; i < MAX_DRIVES; i++) {
			if (mask & (1 << i)) { //DRIVE EXISTS

				drives[drive_count] = 'A' + i;
				drive_count++;
			}
		}

		change_dir(".");
	}

	DirAccessWindows::~DirAccessWindows() {
		list_dir_end();

		memdelete(p);
	}
	void DirAccessWindows::list_dir_end() {
		if (p->h != INVALID_HANDLE_VALUE) {
			FindClose(p->h);
			p->h = INVALID_HANDLE_VALUE;
		}
	}

	Error DirAccessWindows::change_dir(String p_dir) {
		//GLOBAL_LOCK_FUNCTION

			p_dir = fix_path(p_dir);

		WCHAR real_current_dir_name[2048];
		GetCurrentDirectoryW(2048, real_current_dir_name);
		String prev_dir = String::utf16((const char16_t*)real_current_dir_name);

		SetCurrentDirectoryW((LPCWSTR)(current_dir.utf16().get_data()));
		bool worked = (SetCurrentDirectoryW((LPCWSTR)(p_dir.utf16().get_data())) != 0);

		String base = _get_root_path();
		if (!base.is_empty()) {
			GetCurrentDirectoryW(2048, real_current_dir_name);
			String new_dir = String::utf16((const char16_t*)real_current_dir_name).replace("\\", "/");
			if (!new_dir.begins_with(base)) {
				worked = false;
			}
		}

		if (worked) {
			GetCurrentDirectoryW(2048, real_current_dir_name);
			current_dir = String::utf16((const char16_t*)real_current_dir_name);
			current_dir = current_dir.replace("\\", "/");
		}

		SetCurrentDirectoryW((LPCWSTR)(prev_dir.utf16().get_data()));

		return worked ? OK : ERR_INVALID_PARAMETER;
	}
}