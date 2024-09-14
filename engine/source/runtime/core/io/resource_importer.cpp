#include "resource_importer.h"
#include "core/meta/serializer/serializer.h"
#include "_generated/serializer/all_serializer.h"
#include "json.h"
using namespace lain;
ResourceFormatImporter* ResourceFormatImporter::singleton = nullptr;
Error ResourceFormatImporter::_get_path_and_type(const String& p_path, ImportPathAndType& r_path_and_type, bool* r_valid) const {
    Error err;
	Ref<FileAccess> f = FileAccess::open(p_path + ".import", FileAccess::READ, &err);
    if (f.is_null()) {
		if (r_valid) {
			*r_valid = false;
		}
		return err;
	}
	String json_err = "";
    Json json = Json::parse(f->get_as_text(), json_err);
    ERR_FAIL_COND_V_MSG(!json_err.is_empty(), ERR_PARSE_ERROR, json_err);
	Serializer::read(json, r_path_and_type);
    return OK;
}

bool lain::ResourceFormatImporter::SortImporterByName::operator()(const Ref<ResourceImporter>& p_a, const Ref<ResourceImporter>& p_b) const {
	return p_a->get_importer_name() < p_b->get_importer_name();
}

Ref<Resource> ResourceFormatImporter::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) {
	ImportPathAndType pat;
	Error err = _get_path_and_type(p_path, pat);

	if (err != OK) {
		if (r_error) {
			*r_error = err;
		}

		return Ref<Resource>();
	}

	Ref<Resource> res = ResourceLoader::_load(pat.path, p_path, pat.type, p_cache_mode, r_error, p_use_sub_threads, r_progress);

#ifdef TOOLS_ENABLED
	if (res.is_valid()) {
		res->set_import_last_modified_time(res->get_last_modified_time()); //pass this, if used
		res->set_import_path(pat.path);
	}
#endif

	return res;
}

void ResourceFormatImporter::get_recognized_extensions(List<String> *p_extensions) const {
	HashSet<String> found;

	for (int i = 0; i < importers.size(); i++) {
		List<String> local_exts;
		importers[i]->get_recognized_extensions(&local_exts);
		for (const String &F : local_exts) {
			if (!found.has(F)) {
				p_extensions->push_back(F);
				found.insert(F);
			}
		}
	}
}

void lain::ResourceFormatImporter::get_recognized_resources(List<String>* p_extensions) const {
	HashSet<String> found;
	for (int i = 0; i < importers.size(); i++) {
		List<String> local_exts;
		importers[i]->get_recognized_resources(&local_exts);
		for (const String &F : local_exts) {
			if (!found.has(F)) {
				p_extensions->push_back(F);
				found.insert(F);
			}
		}
	}
}

bool ResourceFormatImporter::exists(const String &p_path) const {
	return FileAccess::exists(p_path + ".import");
}

void ResourceFormatImporter::_add_importer_to_map(const Ref<ResourceImporter>& p_importer, bool is_ext) {
	List<String> list;
	if (!is_ext) {
		p_importer->get_recognized_resources(&list);
	} else {
		p_importer->get_recognized_extensions(&list);
	}
	for (const String &F : list) {
		Vector<int>& idxs = is_ext ? ext_to_loader_idx[F] : type_to_loader_idx[F];
		idxs.push_back(importers.size() - 1);
	}
}
void ResourceFormatImporter::_remove_importer_from_map(const Ref<ResourceImporter>& p_importer, bool is_ext) {
    List<String> list;
	if(!is_ext)
		p_importer->get_recognized_resources(&list);
	else{
		p_importer->get_recognized_extensions(&list);
	}
	for (const String &F : list) {
		Vector<int>& idxs = is_ext? ext_to_loader_idx[F] : type_to_loader_idx[F];
		for (int i = 0; i < idxs.size(); i++) {
			if (importers[idxs[i]] == p_importer) {
				idxs.erase(i);
				break;
			}
		}
		if (idxs.size() == 0) {
			type_to_loader_idx.erase(F);
		}
	}
}


void ResourceFormatImporter::add_importer(const Ref<ResourceImporter> &p_importer, bool p_first_priority) {
	ERR_FAIL_COND(p_importer.is_null());
	int idx = 0;
	if (p_first_priority) {
		importers.insert(0, p_importer);
	} else {
		importers.push_back(p_importer);
		idx = importers.size() - 1;
	}
	_add_importer_to_map(p_importer, false);
	_add_importer_to_map(p_importer, true);
}

void lain::ResourceFormatImporter::remove_importer(const Ref<ResourceImporter>& p_importer) {
	ERR_FAIL_COND(p_importer.is_null());
	_remove_importer_from_map(p_importer, false);
	_remove_importer_from_map(p_importer, true);
	 { importers.erase(p_importer); }
}

Ref<ResourceImporter> ResourceFormatImporter::get_importer_by_name(const String &p_name) const {
	for (int i = 0; i < importers.size(); i++) {
		if (importers[i]->get_importer_name() == p_name) {
			return importers[i];
		}
	}
	return Ref<ResourceImporter>();
}
void ResourceFormatImporter::get_importers_for_extension(const String &p_extension, List<Ref<ResourceImporter>> *r_importers) {
	for (int i : ext_to_loader_idx[p_extension]) {
		r_importers->push_back(importers[i]);
	}
}

void ResourceFormatImporter::get_importers(List<Ref<ResourceImporter>> *r_importers) {
	for (int i = 0; i < importers.size(); i++) {
		r_importers->push_back(importers[i]);
	}
}
Variant ResourceFormatImporter::get_resource_metadata(const String &p_path) const {
	ImportPathAndType pat;
	Error err = _get_path_and_type(p_path, pat);

	if (err != OK) {
		return Variant();
	}

	return pat.metadata;
}

Ref<ResourceImporter> ResourceFormatImporter::get_importer_by_extension(const String &p_extension) const {
	Ref<ResourceImporter> importer;
	float priority = 0;

	for (int i = 0; i < importers.size(); i++) {
		List<String> local_exts;
		importers[i]->get_recognized_extensions(&local_exts);
		for (const String &F : local_exts) {
			if (p_extension.to_lower() == F && importers[i]->get_priority() > priority) {
				importer = importers[i];
				priority = importers[i]->get_priority();
			}
		}
	}

	return importer;
}
