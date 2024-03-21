#pragma once
#ifndef __DIR_ACCESS_H__
#define __DIR_ACCESS_H__
#include "base.h"
#include "core/string/ustring.h"
#include "core/object/refcounted.h"
#include <filesystem>
namespace lain {
	class DirAccess: public RefCounted {
		//  CreateFunc ����Ϊһ������ָ�����ͣ��ú�������һ�� Ref<DirAccess> ����

		typedef Ref<DirAccess>(*CreateFunc)();

	public:
		enum AccessType {
			ACCESS_RESOURCES,
			ACCESS_USERDATA,
			ACCESS_FILESYSTEM,
			ACCESS_MAX
		};
	private:
		AccessType _access_type = ACCESS_FILESYSTEM;
		static CreateFunc create_func[ACCESS_MAX]; ///< set this to instance a filesystem object
		static Ref<DirAccess> _open(const String& p_path);

		Error _copy_dir(Ref<DirAccess>& p_target_da, const String& p_to, int p_chmod_flags, bool p_copy_links);
		Vector<String> _get_contents(bool p_directories);

		thread_local static Error last_dir_open_error;
		bool include_navigational = false;
		bool include_hidden = false; // If true, hidden files are included when navigating the directory.
	protected:

		String _get_root_path() const;
		virtual String _get_root_string() const;

		AccessType get_access_type() const { return _access_type; }
		virtual String fix_path(const String& p_path) const;

		template <class T>
		static Ref<DirAccess> _create_builtin() {
			return memnew(T);
		}
	public:
		virtual Error list_dir_begin() = 0; ///< This starts dir listing
		virtual String get_next() = 0;
		virtual bool current_is_dir() const = 0;
		virtual bool current_is_hidden() const = 0;

		virtual void list_dir_end() = 0; ///<

		virtual int get_drive_count() = 0;
		virtual String get_drive(int p_drive) = 0;
		virtual bool drives_are_shortcuts() { return false; }

		virtual Error change_dir(String p_dir) = 0; ///< can be relative or absolute, return false on success
		virtual String get_current_dir(bool p_include_drive = true) const = 0; ///< return current dir location
		virtual Error make_dir(String p_dir) = 0;
		virtual Error make_dir_recursive(const String& p_dir);
		virtual Error erase_contents_recursive(); //super dangerous, use with care!

		virtual bool file_exists(String p_file) = 0;
		virtual bool dir_exists(String p_dir) = 0;
		virtual bool is_readable(String p_dir) { return true; };
		virtual bool is_writable(String p_dir) { return true; };
		static bool exists(const String& p_dir);
		virtual uint64_t get_space_left() = 0;

		Error copy_dir(const String& p_from, String p_to, int p_chmod_flags = -1, bool p_copy_links = false);
		virtual Error copy(const String& p_from, const String& p_to, int p_chmod_flags = -1);
		virtual Error rename(String p_from, String p_to) = 0;
		virtual Error remove(String p_name) = 0;

		virtual bool is_link(String p_file) = 0;
		virtual String read_link(String p_file) = 0;
		virtual Error create_link(String p_source, String p_target) = 0;

		// Meant for editor code when we want to quickly remove a file without custom
		// handling (e.g. removing a cache file).
		static void remove_file_or_error(const String& p_path) {
			Ref<DirAccess> da = create(ACCESS_FILESYSTEM);
			if (da->file_exists(p_path)) {
				if (da->remove(p_path) != OK) {
					ERR_FAIL_MSG("Cannot remove file or directory: " + p_path);
				}
			}
			else {
				ERR_FAIL_MSG("Cannot remove non-existent file or directory: " + p_path);
			}
		}

		virtual String get_filesystem_type() const = 0;
		static String get_full_path(const String& p_path, AccessType p_access);
		static Ref<DirAccess> create_for_path(const String& p_path);

		static Ref<DirAccess> create(AccessType p_access);
		static Error get_open_error();

		template <class T>
		static void make_default(AccessType p_access) {
			create_func[p_access] = _create_builtin<T>;
		}

		static Ref<DirAccess> open(const String& p_path, Error* r_error = nullptr);

		static int _get_drive_count();
		static String get_drive_name(int p_idx);

		static Error make_dir_absolute(const String& p_dir);
		static Error make_dir_recursive_absolute(const String& p_dir);
		static bool dir_exists_absolute(const String& p_dir);

		static Error copy_absolute(const String& p_from, const String& p_to, int p_chmod_flags = -1);
		static Error rename_absolute(const String& p_from, const String& p_to);
		static Error remove_absolute(const String& p_path);

		Vector<String> get_files();
		static Vector<String> get_files_at(const String& p_path);
		Vector<String> get_directories();
		static Vector<String> get_directories_at(const String& p_path);
		String _get_next();

		void set_include_navigational(bool p_enable);
		bool get_include_navigational() const;
		void set_include_hidden(bool p_enable);
		bool get_include_hidden() const;

		//virtual bool is_case_sensitive(const String& p_path) const;

		DirAccess() {}
		virtual ~DirAccess() {}

	};
}

#endif // !__DIR_ACCESS_H__