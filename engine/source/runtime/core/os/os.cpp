#include "os.h"
#include "core/io/file_access.h"
#include "core/io/dir_access.h"

namespace lain {
	OS* OS::p_singleton = nullptr;
	 String OS::GetResourceDir() const {
		return ProjectSettings::GetSingleton()->GetResourcePath();
	}
	 String OS::GetUserDataDir() const {
		 return ".";
	 }
	 String OS::GetCachePath() const {
		 return ".";
	 }
	 // Helper function to ensure that a dir name/path will be valid on the OS
	 String OS::GetSafeDirName(const String& p_dir_name, bool p_allow_paths) const {
		 String safe_dir_name = p_dir_name;
		 Vector<String> invalid_chars = String(": * ? \" < > |").split(" ");
		 if (p_allow_paths) {
			 // Dir separators are allowed, but disallow ".." to avoid going up the filesystem
			 invalid_chars.push_back("..");
			 safe_dir_name = safe_dir_name.replace("\\", "/").strip_edges();
		 }
		 else {
			 invalid_chars.push_back("/");
			 invalid_chars.push_back("\\");
			 safe_dir_name = safe_dir_name.strip_edges();

			 // These directory names are invalid.
			 if (safe_dir_name == ".") {
				 safe_dir_name = "dot";
			 }
			 else if (safe_dir_name == "..") {
				 safe_dir_name = "twodots";
			 }
		 }

		 for (int i = 0; i < invalid_chars.size(); i++) {
			 safe_dir_name = safe_dir_name.replace(invalid_chars[i], "-");
		 }
		 return safe_dir_name;
	 }
	 // threadÓëcpuÊýÁ¿
	 int OS::GetProcessorCount() const { return THREADING_NAMESPACE::thread::hardware_concurrency(); }
	 void OS::EnsureUserDataDir() {
		 String dd = GetUserDataDir();
		 if (DirAccess::exists(dd)) {
			 return;
		 }

		 Ref<DirAccess> da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
		 Error err = da->make_dir_recursive(dd);
		 ERR_FAIL_COND_MSG(err != OK, "Error attempting to create data dir: " + dd + ".");
	 }
}
