#include "resource_importer.h"
#include "core/meta/serializer/serializer.h"
#include "_generated/serializer/all_serializer.h"
using namespace lain;
Error ResourceFormatImporter::_get_path_and_type(const String& p_path, ImportPathAndType& r_path_and_type, bool* r_valid) const {
    Error err;
	Ref<FileAccess> f = FileAccess::open(p_path + ".import", FileAccess::READ, &err);
    if (f.is_null()) {
		if (r_valid) {
			*r_valid = false;
		}
		return err;
	}
    std::string json_err;
    json11::Json::parse(f->get_as_text().utf8().get_data(), json_err);
    
    return err;

}   