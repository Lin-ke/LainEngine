#pragma once
#ifndef __FILE_ACCESS_WINDOWS_H__
#define __FILE_ACCESS_WINDOWS_H__
#include "core/string/ustring.h"
#include "core/object/refcounted.h"
#include "core/templates/hash_set.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"

namespace lain {

class FileAccessWin :public FileAccess {
public:
	String path;
	String path_src;
	String save_path;
	FILE* f = nullptr;
	int flags = 0;
	void check_errors() const;
	mutable int prev_op = 0;
	mutable Error last_error = OK;


	virtual Error open_internal(const String& p_path, int p_mode_flags);
	virtual bool is_path_invalid(const String& p_path);

	static HashSet<String> invalid_files;
	virtual String fix_path(const String& p_path) const;
};
}

#endif // !__FILE_ACCESS_WINDOWS_H__

