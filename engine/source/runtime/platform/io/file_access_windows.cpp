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
#ifdef _MSC_VER
#define S_ISREG(m) ((m) & _S_IFREG)
#endif
namespace lain {
	void FileAccessWindows::_close() {
		if (!f) {
			return;
		}

		fclose(f);
		f = nullptr;

		if (!save_path.is_empty()) {
			bool rename_error = true;
			const Char16String& path_utf16 = path.utf16();
			const Char16String& save_path_utf16 = save_path.utf16();

			int attempts = 4;
			for (int i = 0; i < 1000; i++) {
				if (ReplaceFileW((LPCWSTR)(save_path_utf16.get_data()), (LPCWSTR)(path_utf16.get_data()), nullptr, REPLACEFILE_IGNORE_MERGE_ERRORS | REPLACEFILE_IGNORE_ACL_ERRORS, nullptr, nullptr)) {
					rename_error = false;
				}
				else {
					// Either the target exists and is locked (temporarily, hopefully)
					// or it doesn't exist; let's assume the latter before re-trying.
					rename_error = _wrename((LPCWSTR)(path_utf16.get_data()), (LPCWSTR)(save_path_utf16.get_data())) != 0;
				}

				if (!rename_error) {
					break;
				}

				OS::GetSingleton()->DelayUsec(1000);
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

		_close();

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

	
	// 查是否eof
	void FileAccessWindows::check_errors() const {
		ERR_FAIL_COND(!f);

		if (feof(f)) {
			last_error = ERR_FILE_EOF;
		}
	}
	ui8 FileAccessWindows::get_8() const {
		ERR_FAIL_COND_V(!f, 0);

		if (flags == READ_WRITE || flags == WRITE_READ) {
			if (prev_op == WRITE) {
				fflush(f);
			}
			prev_op = READ;
		}
		uint8_t b;
		if (fread(&b, 1, 1, f) == 0) {
			check_errors();
			b = '\0';
		}

		return b;
	}
	bool FileAccessWindows::is_open() const {
		return (f != nullptr);
	}
	String FileAccessWindows::get_path() const {
		return path_src;
	}

	String FileAccessWindows::get_path_absolute() const {
		return path;
	}
	// 在文件的位置定义指针
	void FileAccessWindows::seek(int64_t p_position) {
		ERR_FAIL_COND(!f);

		last_error = OK;
		if (_fseeki64(f, p_position, SEEK_SET)) {
			check_errors();
		}
		prev_op = 0;
	}

	void FileAccessWindows::seek_end(int64_t p_position) {
		ERR_FAIL_COND(!f);

		last_error = OK;
		if (_fseeki64(f, p_position, SEEK_END)) {
			check_errors();
		}
		prev_op = 0;
	}
	uint64_t FileAccessWindows::get_position() const {
		int64_t aux_position = _ftelli64(f);
		if (aux_position < 0) {
			check_errors();
		}
		return static_cast<ui64>(aux_position);
	}
	// 单位是字节
	uint64_t FileAccessWindows::get_length() const {
		ERR_FAIL_COND_V(!f, 0);

		uint64_t pos = get_position();
		_fseeki64(f, 0, SEEK_END);
		uint64_t size = get_position();
		_fseeki64(f, pos, SEEK_SET);

		return size;
	}

	bool FileAccessWindows::eof_reached() const {
		ERR_FAIL_COND(!f);
		if (feof(f)) {
			last_error = ERR_FILE_EOF;
			return true;
		}
		return false;
	}
	uint64_t FileAccessWindows::get_buffer(uint8_t* p_dst, uint64_t p_length) const {
		ERR_FAIL_COND_V(!p_dst && p_length > 0, -1);
		ERR_FAIL_COND_V(!f, -1);

		if (flags == READ_WRITE || flags == WRITE_READ) {
			if (prev_op == WRITE) {
				fflush(f);
			}
			prev_op = READ;
		}
		uint64_t read = fread(p_dst, 1, p_length, f);
		check_errors();
		return read;
	}
	Error FileAccessWindows::get_error() const {
		return last_error;
	}

	void FileAccessWindows::flush() {
		ERR_FAIL_COND(!f);

		fflush(f);
		if (prev_op == WRITE) {
			prev_op = 0;
		}
	}
	void FileAccessWindows::store_8(uint8_t p_dest) {
		ERR_FAIL_COND(!f);

		if (flags == READ_WRITE || flags == WRITE_READ) {
			if (prev_op == READ) {
				if (last_error != ERR_FILE_EOF) {
					fseek(f, 0, SEEK_CUR);
				}
			}
			prev_op = WRITE;
		}
		fwrite(&p_dest, 1, 1, f);
	}
	void FileAccessWindows::store_16(uint16_t p_dest) {
		ERR_FAIL_NULL(f);

		if (flags == READ_WRITE || flags == WRITE_READ) {
			if (prev_op == READ) {
				if (last_error != ERR_FILE_EOF) {
					fseek(f, 0, SEEK_CUR);
				}
			}
			prev_op = WRITE;
		}

		if (big_endian) {
			p_dest = BSWAP16(p_dest);
		}

		fwrite(&p_dest, 1, 2, f);
	}

	void FileAccessWindows::store_32(uint32_t p_dest) {
		ERR_FAIL_NULL(f);

		if (flags == READ_WRITE || flags == WRITE_READ) {
			if (prev_op == READ) {
				if (last_error != ERR_FILE_EOF) {
					fseek(f, 0, SEEK_CUR);
				}
			}
			prev_op = WRITE;
		}

		if (big_endian) {
			p_dest = BSWAP32(p_dest);
		}

		fwrite(&p_dest, 1, 4, f);
	}

	void FileAccessWindows::store_64(uint64_t p_dest) {
		ERR_FAIL_NULL(f);

		if (flags == READ_WRITE || flags == WRITE_READ) {
			if (prev_op == READ) {
				if (last_error != ERR_FILE_EOF) {
					fseek(f, 0, SEEK_CUR);
				}
			}
			prev_op = WRITE;
		}

		if (big_endian) {
			p_dest = BSWAP64(p_dest);
		}

		fwrite(&p_dest, 1, 8, f);
	}

	void FileAccessWindows::store_buffer(const uint8_t* p_src, uint64_t p_length) {
		ERR_FAIL_COND(!f);
		ERR_FAIL_COND(!p_src && p_length > 0);

		if (flags == READ_WRITE || flags == WRITE_READ) {
			if (prev_op == READ) {
				if (last_error != ERR_FILE_EOF) {
					fseek(f, 0, SEEK_CUR); //why?
				}
			}
			prev_op = WRITE;
		}
		ERR_FAIL_COND(fwrite(p_src, 1, p_length, f) != (size_t)p_length);
	}
	bool FileAccessWindows::file_exists(const String& p_name) {
		if (is_path_invalid(p_name)) {
			return false;
		}

		String filename = fix_path(p_name);
		FILE* g = _wfsopen((LPCWSTR)(filename.utf16().get_data()), L"rb", _SH_DENYNO);
		if (g == nullptr) {
			return false;
		}
		else {
			fclose(g);
			return true;
		}
	}

	uint64_t FileAccessWindows::_get_modified_time(const String& p_file) {
		if (is_path_invalid(p_file)) {
			return 0;
		}

		String file = fix_path(p_file);
		if (file.ends_with("/") && file != "/") {
			file = file.substr(0, file.length() - 1);
		}

		struct _stat st;
		int rv = _wstat((LPCWSTR)(file.utf16().get_data()), &st);

		if (rv == 0) {
			return st.st_mtime;
		}
		else {
			L_CORE_WARN("Failed to get modified time for: " + p_file + "");
			return 0;
		}
	}
	void FileAccessWindows::close() {
		_close();
	}

	uint16_t FileAccessWindows::get_16() const {
		ERR_FAIL_NULL_V(f, 0);

		if (flags == READ_WRITE || flags == WRITE_READ) {
			if (prev_op == WRITE) {
				fflush(f);
			}
			prev_op = READ;
		}

		uint16_t b = 0;
		if (fread(&b, 1, 2, f) != 2) {
			check_errors();
		}

		if (big_endian) {
			b = BSWAP16(b);
		}

		return b;
	}

	uint32_t FileAccessWindows::get_32() const {
		ERR_FAIL_NULL_V(f, 0);

		if (flags == READ_WRITE || flags == WRITE_READ) {
			if (prev_op == WRITE) {
				fflush(f);
			}
			prev_op = READ;
		}

		uint32_t b = 0;
		if (fread(&b, 1, 4, f) != 4) {
			check_errors();
		}

		if (big_endian) {
			b = BSWAP32(b);
		}

		return b;
	}

	uint64_t FileAccessWindows::get_64() const {
		ERR_FAIL_NULL_V(f, 0);

		if (flags == READ_WRITE || flags == WRITE_READ) {
			if (prev_op == WRITE) {
				fflush(f);
			}
			prev_op = READ;
		}

		uint64_t b = 0;
		if (fread(&b, 1, 8, f) != 8) {
			check_errors();
		}

		if (big_endian) {
			b = BSWAP64(b);
		}

		return b;
	}

}