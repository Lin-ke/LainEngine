#pragma once
#ifndef __DIR_ACCESS_H__
#define __DIR_ACCESS_H__
#include "base.h"
#include "core/string/ustring.h"
#include "core/object/refcounted.h"
#include <filesystem>
namespace lain {
	class DirAccess: public RefCounted {
		//  CreateFunc 定义为一个函数指针类型，该函数返回一个 Ref<DirAccess> 对象

		typedef Ref<DirAccess>(*CreateFunc)();

	public:
		enum AccessType {
			ACCESS_RESOURCES,
			ACCESS_USERDATA,
			ACCESS_FILESYSTEM,
			ACCESS_MAX
		};
		AccessType m_access_type = ACCESS_FILESYSTEM;
		virtual String _get_root_string() const{
				switch (m_access_type) {
				case ACCESS_RESOURCES:
					return "res://";
				case ACCESS_USERDATA:
					return "user://";
				default:
					return "";
				}
		}

		virtual bool file_exists(String p_file) = 0;
		virtual bool dir_exists(String p_dir) = 0;
		virtual bool is_readable(String p_dir) { return true; };
		virtual bool is_writable(String p_dir) { return true; };
		String _get_root_path() const;
		virtual String fix_path(String p_path) const;
		virtual Error change_dir(String p_dir) = 0; ///< can be relative or absolute, return false on success
		virtual String get_current_dir(bool p_drive = false) const = 0;

		static bool exists(String p_dir);
		static DirAccess::CreateFunc DirAccess::create_func[ACCESS_MAX]; 
		// factory
		static Ref<DirAccess> create(AccessType p_access); 
		static Ref<DirAccess> create_for_path(const String& p_path);

	};
}

#endif // !__DIR_ACCESS_H__