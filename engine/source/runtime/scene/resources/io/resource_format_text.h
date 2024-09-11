#pragma once
#ifndef RESOURCE_FORMAT_TEXT_H
#define RESOURCE_FORMAT_TEXT_H
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/io/resource_uid.h"
#include "core/scene/packed_scene.h"
#include "core/templates/pair.h"

namespace lain {
	// 场景树
class ResourceFormatLoaderText : public ResourceFormatLoader {
public:
	static ResourceFormatLoaderText* singleton;
	virtual Ref<Resource> load(const String& p_path, const String& p_original_path = "", Error* r_error = nullptr, bool p_use_sub_threads = false, float* r_progress = nullptr, CacheMode p_cache_mode = CACHE_MODE_REUSE) override;
	virtual void get_recognized_extensions(List<String>* p_extensions) const override;
	virtual void get_recognized_resources(List<String>* p_resouce_class) const override;

	//virtual void get_recognized_extensions_for_type(const String& p_type, List<String>* p_extensions) const override;
	//virtual bool handles_type(const String& p_type) const override;
	//virtual void get_classes_used(const String& p_path, HashSet<StringName>* r_classes) override;

	//virtual String get_resource_type(const String& p_path) const override;
	//virtual String get_resource_script_class(const String& p_path) const override;
	//virtual ResourceUID::ID get_resource_uid(const String& p_path) const override;
	//virtual void get_dependencies(const String& p_path, List<String>* p_dependencies, bool p_add_types = false) override;
	//virtual Error rename_dependencies(const String& p_path, const HashMap<String, String>& p_map) override;

	//static Error convert_file_to_binary(const String& p_src_path, const String& p_dst_path);

	ResourceFormatLoaderText() { singleton = this; }
};

class ResourceSaverText {
	String local_path;

	Ref<PackedScene> packed_scene;

	bool takeover_paths = false;
	bool relative_paths = false;
	bool bundle_resources = false;
	bool skip_editor = false;

	struct NonPersistentKey { //for resource properties generated on the fly
		Ref<Resource> base;
		StringName property;
		bool operator<(const NonPersistentKey& p_key) const { return base == p_key.base ? property < p_key.property : base < p_key.base; }
	};

	RBMap<NonPersistentKey, Variant> non_persistent_map;

	HashSet<Ref<Resource>> resource_set;
	List<Ref<Resource>> saved_resources;
	HashMap<Ref<Resource>, Pair<int, String>> external_resources;
	HashMap<Ref<Resource>, String> internal_resources;

	struct ResourceSort {
		Ref<Resource> resource;
		String id;
		bool operator<(const ResourceSort& p_right) const {
			return id.naturalnocasecmp_to(p_right.id) < 0;
		}
	};

	void _find_resources(const Variant& p_variant, bool p_main = false);

	static String _write_resources(void* ud, const Ref<Resource>& p_resource);
	String _write_resource(const Ref<Resource>& res);

public:
	Error save(const String& p_path, const Ref<Resource>& p_resource, uint32_t p_flags = 0);
};


class ResourceFormatSaverText;

// 实际执行操作的类
class ResourceLoaderText {
	friend class ResourceFormatLoaderText;
	friend class ResourceFormatSaverText;
	struct ExtResource {
		Ref<ResourceLoader::LoadToken> load_token;
		String path;
		String type;
	}; // ext resource
	Ref<FileAccess> f;
	String local_path;
	Ref<Resource> resource;
	Error error = OK;
	bool is_scene = false;
	bool use_sub_threads = false;
	float* progress = nullptr; // ?
	String res_path;
	String error_text;
	ResourceUID::ID res_uid = ResourceUID::INVALID_ID;
	// 外部资源加载
	HashMap<String, ExtResource> ext_resources;
	HashMap<String, Ref<Resource>> int_resources;

	int resources_total = 0;
	int resource_current = 0;
	ResourceFormatLoader::CacheMode cache_mode = ResourceFormatLoader::CACHE_MODE_REUSE;
	ResourceFormatLoader::CacheMode cache_mode_for_external = ResourceFormatLoader::CACHE_MODE_REUSE;
	// 如果missing
	
public:
	Error load();
	Ref<Resource> get_resource() { return resource; }
	Error set_uid(Ref<FileAccess> p_f, ResourceUID::ID p_uid);
	void open(Ref<FileAccess> p_f, bool p_skip_first_tag = false);
	ResourceLoaderText() {}
	
};

class ResourceFormatSaverText : public ResourceFormatSaver {
public:
	static ResourceFormatSaverText* singleton;
	virtual Error save(const Ref<Resource>& p_resource, const String& p_path, uint32_t p_flags = 0) override;
	virtual Error set_uid(const String& p_path, ResourceUID::ID p_uid) override;
	//virtual bool recognize(const Ref<Resource>& p_resource) const override;
	virtual void get_recognized_extensions(const Ref<Resource>& p_resource, List<String>* p_extensions) const override;
	virtual void get_recognized_extensions(List<String>* p_extensions) const override;
	virtual void get_recognized_resources(List<String>* p_resource_class) const override;


	ResourceFormatSaverText();
};

}

#endif // RESOURCE_FORMAT_TEXT_H
