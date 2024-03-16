#include "file_access_windows.h"
#include "core/config/project_settings.h"
#include <share.h> // _SH_DENYNO
#include <shlwapi.h>
#include "core/os/os.h"
#define WIN32_LEAN_AND_MEAN
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tchar.h>
#include <wchar.h>
#include <windows.h>
#define S_ISREG(m) ((m)&_S_IFREG)
namespace lain {

	///WINDOWS

	String FileAccessWindows::fix_path(const String& p_path) const {
		String r_path = FileAccess::fix_path(p_path);
		if (r_path.is_absolute_path() && !r_path.is_network_share_path() && r_path.length() > MAX_PATH) {
			r_path = "\\\\?\\" + r_path.replace("/", "\\");
		}
		return r_path;
	}
	HashSet<String> FileAccessWindows::invalid_files;
	bool FileAccessWindows::is_path_invalid(const String& p_path) {
		// Check for invalid operating system file.
		String fname = p_path;
		int dot = fname.find(".");
		if (dot != -1) {
			fname = fname.substr(0, dot);
		}
		fname = fname.to_lower();
		return invalid_files.has(fname);
	}
	Error FileAccessWindows::open_internal(const String& p_path, int p_mode_flags) {
		if (is_path_invalid(p_path)) {
#ifdef L_DEBUG
			if (p_mode_flags != READ) {
				WARN_PRINT("The path :" + p_path + " is a reserved Windows system pipe, so it can't be used for creating files.");
			}
#endif
			return ERR_INVALID_PARAMETER;
		}

		//_close();

		path_src = p_path;
		path = fix_path(p_path);

		const WCHAR* mode_string;

		if (p_mode_flags == READ) {
			mode_string = L"rb";
		}
		else if (p_mode_flags == WRITE) {
			mode_string = L"wb";
		}
		else if (p_mode_flags == READ_WRITE) {
			mode_string = L"rb+";
		}
		else if (p_mode_flags == WRITE_READ) {
			mode_string = L"wb+";
		}
		else {
			return ERR_INVALID_PARAMETER;
		}

		/* Pretty much every implementation that uses fopen as primary
		   backend supports utf8 encoding. */

		struct _stat st;
		if (_wstat((LPCWSTR)(path.utf16().get_data()), &st) == 0) {
			if (!S_ISREG(st.st_mode)) {
				return ERR_FILE_CANT_OPEN;
			}
		}


		if (is_backup_save_enabled() && p_mode_flags == WRITE) {
			save_path = path;
			// Create a temporary file in the same directory as the target file.
			WCHAR tmpFileName[MAX_PATH];
			if (GetTempFileNameW((LPCWSTR)(path.get_base_dir().utf16().get_data()), (LPCWSTR)(path.get_file().utf16().get_data()), 0, tmpFileName) == 0) {
				last_error = ERR_FILE_CANT_OPEN;
				return last_error;
			}
			path = tmpFileName;
		}

		f = _wfsopen((LPCWSTR)(path.utf16().get_data()), mode_string, is_backup_save_enabled() ? _SH_SECURE : _SH_DENYNO);

		if (f == nullptr) {
			switch (errno) {
			case ENOENT: {
				last_error = ERR_FILE_NOT_FOUND;
			} break;
			default: {
				last_error = ERR_FILE_CANT_OPEN;
			} break;
			}
			return last_error;
		}
		else {
			last_error = OK;
			flags = p_mode_flags;
			return OK;
		}
	}
	// 避免使用保留文件名
	void FileAccessWindows::initialize() {
		static const char* reserved_files[]{
			"con", "prn", "aux", "nul", "com0", "com1", "com2", "com3", "com4", "com5", "com6", "com7", "com8", "com9", "lpt0", "lpt1", "lpt2", "lpt3", "lpt4", "lpt5", "lpt6", "lpt7", "lpt8", "lpt9", nullptr
		};
		int reserved_file_index = 0;
		while (reserved_files[reserved_file_index] != nullptr) {
			invalid_files.insert(reserved_files[reserved_file_index]);
			reserved_file_index++;
		}
	}
	void FileAccessWindows::finalize() {
		invalid_files.clear();
	}
	FileAccessWindows::~FileAccessWindows() {
		_close();
	}

	void FileAccessWindows::_close() {
		if (!f) {
			return;
		}

		fclose(f);
		f = nullptr;

		if (!save_path.is_empty()) {
			bool rename_error = true;
			int attempts = 4;
			while (rename_error && attempts) {
				// This workaround of trying multiple times is added to deal with paranoid Windows
				// antiviruses that love reading just written files even if they are not executable, thus
				// locking the file and preventing renaming from happening. XD

#ifdef UWP_ENABLED
			// UWP has no PathFileExists, so we check attributes instead
				DWORD fileAttr;

				fileAttr = GetFileAttributesW((LPCWSTR)(save_path.utf16().get_data()));
				if (INVALID_FILE_ATTRIBUTES == fileAttr) {
#else
				if (!PathFileExistsW((LPCWSTR)(save_path.utf16().get_data()))) {
#endif
					// Creating new file
					rename_error = _wrename((LPCWSTR)(path.utf16().get_data()), (LPCWSTR)(save_path.utf16().get_data())) != 0;
				}
				else {
					// Atomic replace for existing file
					rename_error = !ReplaceFileW((LPCWSTR)(save_path.utf16().get_data()), (LPCWSTR)(path.utf16().get_data()), nullptr, 2 | 4, nullptr, nullptr);
				}
				if (rename_error) {
					attempts--;
					OS::GetSingleton()->DelayUsec(100000); // wait 100msec and try again
				}
				}

			if (rename_error) {
				if (close_fail_notify) {
					close_fail_notify(save_path);
				}
			}

			save_path = "";

			ERR_FAIL_COND_MSG(rename_error, "Safe save failed. This may be a permissions problem, but also may happen because you are running a paranoid antivirus. If this is the case, please switch to Windows Defender or disable the 'safe save' option in editor settings. This makes it work, but increases the risk of file corruption in a crash.");
			}
		}
	ui32 FileAccessWindows::get_8() const {

	}
}