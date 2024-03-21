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
	String DirAccessWindows::fix_path(const String& p_path) const {
		String r_path = DirAccess::fix_path(p_path); // 超过最大长度
		if (r_path.is_absolute_path() && !r_path.is_network_share_path() && r_path.length() > MAX_PATH) {
			r_path = "\\\\?\\" + r_path.replace("/", "\\");
		}
		return r_path;
	}
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

	Error DirAccessWindows::list_dir_begin() {
		_cisdir = false;
		_cishidden = false;

		list_dir_end();
		p->h = FindFirstFileExW((LPCWSTR)(String(current_dir + "\\*").utf16().get_data()), FindExInfoStandard, &p->fu, FindExSearchNameMatch, nullptr, 0); // 在目录中搜索具有与指定属性匹配的名称和属性的文件或子目录。

		if (p->h == INVALID_HANDLE_VALUE) {
			return ERR_CANT_OPEN;
		}

		return OK;
	}

	String DirAccessWindows::get_next() {
		if (p->h == INVALID_HANDLE_VALUE) {
			return "";
		}

		_cisdir = (p->fu.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		_cishidden = (p->fu.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);

		String name = String::utf16((const char16_t*)(p->fu.cFileName));
		// 写到P->fu中
		if (FindNextFileW(p->h, &p->fu) == 0) {
			FindClose(p->h);
			p->h = INVALID_HANDLE_VALUE;
		}

		return name;
	}
	bool DirAccessWindows::current_is_dir() const {
		return _cisdir;
	}

	bool DirAccessWindows::current_is_hidden() const {
		return _cishidden;
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
	// res:// ...
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
			FindClose(p->h); // 关闭搜索句柄
			p->h = INVALID_HANDLE_VALUE;
		}
	}
	int DirAccessWindows::get_drive_count() {
		return drive_count;
	}
	String DirAccess::get_drive_name(int p_idx) {
		Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
		return d->get_drive(p_idx);
	}
	String DirAccessWindows::get_drive(int p_drive) {
		if (p_drive < 0 || p_drive >= drive_count) {
			return "";
		}

		return String::chr(drives[p_drive]) + ":"; // e.g. D:
	}
	bool DirAccessWindows::dir_exists(String p_dir) {
		GLOBAL_LOCK_FUNCTION

			if (p_dir.is_relative_path()) {
				p_dir = get_current_dir().path_join(p_dir);
			}

		p_dir = fix_path(p_dir);

		DWORD fileAttr;
		fileAttr = GetFileAttributesW((LPCWSTR)(p_dir.utf16().get_data()));
		if (INVALID_FILE_ATTRIBUTES == fileAttr) {
			return false;
		}
		return (fileAttr & FILE_ATTRIBUTE_DIRECTORY);
	}
	Error DirAccessWindows::rename(String p_path, String p_new_path) {
		if (p_path.is_relative_path()) {
			p_path = get_current_dir().path_join(p_path);
		}

		p_path = fix_path(p_path);

		if (p_new_path.is_relative_path()) {
			p_new_path = get_current_dir().path_join(p_new_path);
		}

		p_new_path = fix_path(p_new_path);

		// If we're only changing file name case we need to do a little juggling
		// 生成临时文件，替换该文件，重命名
	
		const char16_t* old_path_16 = p_path.utf16().get_data();
		const char16_t* new_path_16 = p_new_path.utf16().get_data();

		if (p_path.to_lower() == p_new_path.to_lower()) {
			if (dir_exists(p_path)) {
				// The path is a dir; just rename
				return ::_wrename((LPCWSTR)(old_path_16), (LPCWSTR)(new_path_16)) == 0 ? OK : FAILED;
			}
			// The path is a file; juggle
			WCHAR tmpfile[MAX_PATH];
			// 生成临时文件名且创建文件，unique = 0则不停尝试
			if (!GetTempFileNameW((LPCWSTR)(fix_path(get_current_dir()).utf16().get_data()), nullptr, 0, tmpfile)) {
				return FAILED;
			}
			
			if (!::ReplaceFileW(tmpfile, (LPCWSTR)(p_path.utf16().get_data()), nullptr, 0, nullptr, nullptr)) {
				DeleteFileW(tmpfile);
				return FAILED;
			}

			return ::_wrename(tmpfile, (LPCWSTR)(new_path_16)) == 0 ? OK : FAILED;

		}
		else {
			if (file_exists(p_new_path)) {
				if (remove(p_new_path) != OK) {
					return FAILED;
				}
			}

			return ::_wrename((LPCWSTR)(p_path.utf16().get_data()), (LPCWSTR)(p_new_path.utf16().get_data())) == 0 ? OK : FAILED;
		}
	}

	Error DirAccessWindows::remove(String p_path) {
		if (p_path.is_relative_path()) {
			p_path = get_current_dir().path_join(p_path);
		}

		p_path = fix_path(p_path);

		DWORD fileAttr;

		fileAttr = GetFileAttributesW((LPCWSTR)(p_path.utf16().get_data()));
		if (INVALID_FILE_ATTRIBUTES == fileAttr) {
			return FAILED;
		}
		if ((fileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
			return ::_wrmdir((LPCWSTR)(p_path.utf16().get_data())) == 0 ? OK : FAILED;
		}
		else {
			return ::_wunlink((LPCWSTR)(p_path.utf16().get_data())) == 0 ? OK : FAILED;
		}
	}
	uint64_t DirAccessWindows::get_space_left() {
		uint64_t bytes = 0;
		if (!GetDiskFreeSpaceEx(nullptr, (PULARGE_INTEGER)&bytes, nullptr, nullptr)) {
			return 0;
		}

		// This is either 0 or a value in bytes.
		return bytes;
	}
	String DirAccessWindows::get_filesystem_type() const {
		String path = fix_path(const_cast<DirAccessWindows*>(this)->get_current_dir());

		int unit_end = path.find(":");
		ERR_FAIL_COND_V(unit_end == -1, String());
		String unit = path.substr(0, unit_end + 1) + "\\";

		if (path.is_network_share_path()) {
			return "Network Share";
		}

		WCHAR szVolumeName[100];
		WCHAR szFileSystemName[10];
		DWORD dwSerialNumber = 0;
		DWORD dwMaxFileNameLength = 0;
		DWORD dwFileSystemFlags = 0;

		if (::GetVolumeInformationW((LPCWSTR)(unit.utf16().get_data()),
			szVolumeName,
			sizeof(szVolumeName),
			&dwSerialNumber,
			&dwMaxFileNameLength,
			&dwFileSystemFlags,
			szFileSystemName,
			sizeof(szFileSystemName)) == TRUE) {
			return String::utf16((const char16_t*)szFileSystemName);
		}

		ERR_FAIL_V("");
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