#pragma once
#ifndef __DIR_ACCESS_WINDOWS_H__
#define __DIR_ACCESS_WINDOWS_H__
#include "core/io/dir_access.h"
namespace lain {
	struct DirAccessWindowsPrivate;

class DirAccessWindows :public DirAccess {

	enum {
		MAX_DRIVES = 26
	};
	// Windows对dir的引用
	DirAccessWindowsPrivate* p = nullptr;

	/* Windows stuff */
	// drive：驱动盘

	char drives[MAX_DRIVES] = { 0 }; // a-z:
	int drive_count = 0;

	String current_dir;

	bool _cisdir = false;
	bool _cishidden = false;
protected:
	virtual String fix_path(const String& p_path) const override;

public:
	virtual Error list_dir_begin() override; ///< This starts dir listing
	virtual String get_next() override;
	virtual bool current_is_dir() const override;
	virtual bool current_is_hidden() const override;
	virtual void list_dir_end() override; ///<

	virtual int get_drive_count() override;
	virtual String get_drive(int p_drive) override;

	virtual Error change_dir(String p_dir) override; ///< can be relative or absolute, return false on success
	virtual String get_current_dir(bool p_include_drive = true) const override; ///< return current dir location

	virtual bool file_exists(String p_file) override;
	virtual bool dir_exists(String p_dir) override;

	virtual Error make_dir(String p_dir) override;

	virtual Error rename(String p_path, String p_new_path) override;
	virtual Error remove(String p_path) override;

	virtual bool is_link(String p_file) override { return false; };
	virtual String read_link(String p_file) override { return p_file; };
	virtual Error create_link(String p_source, String p_target) override { return FAILED; };

	uint64_t get_space_left() override;

	virtual String get_filesystem_type() const override;
	//virtual bool is_case_sensitive(const String& p_path) const override;

	DirAccessWindows();
	~DirAccessWindows();
};
}

#endif // !__DIR_ACCESS_WINDOWS_H__
