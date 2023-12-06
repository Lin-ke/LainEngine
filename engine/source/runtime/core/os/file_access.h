#pragma once
#ifndef __FILE_ACCESS_H__
#define __FILE_ACCESS_H__
#include "core/os/memory.h"
#include "core/string/ustring.h"
#include "core/object/refcounted.h"
#include "core/templates/hash_set.h"
namespace lain {
	// similar to fd 
	class FileAccess :public RefCounted {
	public:
		enum AccessType {
			ACCESS_RESOURCES,
			ACCESS_USERDATA,
			ACCESS_FILESYSTEM,
			ACCESS_MAX
		};
		enum ModeFlags {
			READ = 1,
			WRITE = 2,
			READ_WRITE = 3,
			WRITE_READ = 7,
		};
		/// <summary>
		/// create_functions
		/// </summary>
		typedef void (*FileCloseFailNotify)(const String&);
		typedef Ref<FileAccess>(*CreateFunc)();

		static CreateFunc create_func[ACCESS_MAX];
		
		void _set_access_type(AccessType p_access);

		AccessType _access_type = ACCESS_FILESYSTEM;
		// endian problem
		bool big_endian = false;
		bool real_is_double = false;
		AccessType get_access_type() const;

		// virtual::path
		virtual bool is_path_invalid(const String& p_path) = 0;
		virtual String fix_path(const String& p_path) const;

		virtual String get_path() const { return ""; } /// returns the path for the current open file
		virtual String get_path_absolute() const { return ""; } /// returns the absolute path for the current open file


		virtual void close() = 0;

		virtual bool file_exists(const String& p_name) = 0; ///< return true if a file exists
		virtual void flush() = 0;
		virtual String get_line() const;

		virtual float get_float() const;
		virtual double get_double() const;
		virtual real_t get_real() const;


		/// <summary>
		/// static methods
		/// </summary>
		/// <returns></returns>
		static void set_backup_save(bool p_enable) { backup_save = p_enable; };
		static bool is_backup_save_enabled() { return backup_save; };

		static Ref<FileAccess> create(AccessType p_access); /// Create a file access (for the current platform) this is the only portable way of accessing files.
		static Ref<FileAccess> create_for_path(const String& p_path);
		static Ref<FileAccess> open(const String& p_path, int p_mode_flags, Error* r_error = nullptr); /// Create a file access (for the current platform) this is the only portable way of accessing files.

		virtual Error open_internal(const String& p_path, int p_mode_flags) = 0; ///< open a file
	private:
		static bool backup_save;
		template<typename T>
		static Ref<FileAccess> _create_builtin() {
			return memnew(T);
		}
		template <class T>
		static void make_default(AccessType p_access) {
			create_func[p_access] = _create_builtin<T>;
		}

	};

}
#endif