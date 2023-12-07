#pragma once
#ifndef __DIR_ACCESS_WINDOWS_H__
#define __DIR_ACCESS_WINDOWS_H__
#include "core/os/dir_access.h"
namespace lain {

class DirAccessWin :public DirAccess {
public:
	String current_dir;

	virtual String fix_path(String p_path) const {
		String r_path = DirAccess::fix_path(p_path);
		if (r_path.is_absolute_path() && !r_path.is_network_share_path() && r_path.length() > MAX_PATH) {
			r_path = "\\\\?\\" + r_path.replace("/", "\\");
		}
		return r_path;
	}
	virtual Error change_dir(String p_dir) {
		std::filesystem::path real_current_path = std::filesystem::current_path();
		String prev_dir = String::utf16((const char16_t*)real_current_path.generic_u16string().c_str());

	}
	Error make_dir(String p_path);

	String get_current_dir(bool p_include_drive = true) const;
	

	bool file_exists(String p_file);
};
}

#endif // !__DIR_ACCESS_WINDOWS_H__
