#include "file_access_windows.h"
#include "core/config/project_settings.h"
#include "core/os/os.h"
#include "core/os/file_access.h"

#define S_ISREG(m) ((m)&_S_IFREG)
namespace lain {

	///WINDOWS

	String FileAccessWin::fix_path(const String& p_path) const {
		String r_path = FileAccess::fix_path(p_path);
		if (r_path.is_absolute_path() && !r_path.is_network_share_path() && r_path.length() > MAX_PATH) {
			r_path = "\\\\?\\" + r_path.replace("/", "\\");
		}
		return r_path;
	}
	HashSet<String> FileAccessWin::invalid_files;
	bool FileAccessWin::is_path_invalid(const String& p_path) {
		// Check for invalid operating system file.
		String fname = p_path;
		int dot = fname.find(".");
		if (dot != -1) {
			fname = fname.substr(0, dot);
		}
		fname = fname.to_lower();
		return invalid_files.has(fname);
	}
	Error FileAccessWin::open_internal(const String& p_path, int p_mode_flags) {
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
}