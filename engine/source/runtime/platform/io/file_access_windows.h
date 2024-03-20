#pragma once
#ifndef __FILE_ACCESS_WINDOWS_H__
#define __FILE_ACCESS_WINDOWS_H__
#include "core/string/ustring.h"
#include "core/object/refcounted.h"
#include "core/templates/hash_set.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"

namespace lain {

class FileAccessWindows :public FileAccess {
private:
	FILE* f = nullptr;
	int flags = 0;
	void check_errors() const;
	mutable int prev_op = 0;
	mutable Error last_error = OK;
	String path;	  // 绝对路径
	String path_src;  // 资源路径
	String save_path; // 保存路径

	void _close();

	static bool is_path_invalid(const String& p_path);
	static HashSet<String> invalid_files; // 保留文件名
public:
	virtual String fix_path(const String& p_path) const override;
	virtual Error open_internal(const String& p_path, int p_mode_flags) override; ///< open a file
	virtual bool is_open() const override; ///< true when file is open

	virtual String get_path() const override; /// returns the path for the current open file
	virtual String get_path_absolute() const override; /// returns the absolute path for the current open file

	virtual void seek(int64_t p_position) override; ///< seek to a given position
	virtual void seek_end(int64_t p_position = 0) override; ///< seek from the end of file
	virtual uint64_t get_position() const override; ///< get position in the file
	virtual uint64_t get_length() const override; ///< get size of the file

	virtual bool eof_reached() const override; ///< reading passed EOF

	virtual uint8_t get_8() const override; ///< get a byte
	virtual uint16_t get_16() const override;
	virtual uint32_t get_32() const override;
	virtual uint64_t get_64() const override;
	virtual uint64_t get_buffer(uint8_t* p_dst, uint64_t p_length) const override;

	virtual Error get_error() const override; ///< get last error

	virtual void flush() override;

	virtual void store_8(uint8_t p_dest) override; ///< store a byte
	virtual void store_16(uint16_t p_dest) override;
	virtual void store_32(uint32_t p_dest) override;
	virtual void store_64(uint64_t p_dest) override;
	virtual void store_buffer(const uint8_t* p_src, uint64_t p_length) override; ///< store an array of bytes

	virtual bool file_exists(const String& p_name) override; ///< return true if a file exists

	uint64_t _get_modified_time(const String& p_file) override;

	virtual void close() override;

	static void initialize();
	static void finalize();

	FileAccessWindows() {}
	virtual ~FileAccessWindows() { _close(); };
};
}

#endif // !__FILE_ACCESS_WINDOWS_H__

