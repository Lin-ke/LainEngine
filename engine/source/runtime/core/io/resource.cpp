
#include "resource.h"
#include "core/typedefs.h"
#include "core/templates/local_vector.h"
#include "resource_loader.h"
#include "core/meta/reflection/reflection.h"
namespace lain {


	// resource
	void Resource::SetName(const String& p_name) {
		name = p_name;
	}
	String Resource::GetName() const {
		return name;
	}
	// 设置path_cache为path，
	void Resource::SetPath(const String& p_path, bool p_take_over) {
		if (path_cache == p_path) {
			return;
		}

		if (p_path.is_empty()) {
			p_take_over = false; // Can't take over an empty path
		}

		ResourceCache::lock.lock();

		if (!path_cache.is_empty()) {
			ResourceCache::resources.erase(path_cache);
		}

		path_cache = "";

		Ref<Resource> existing = ResourceCache::get_ref(p_path);

		if (existing.is_valid()) {
			if (p_take_over) {
				existing->path_cache = String();
				ResourceCache::resources.erase(p_path);
			}
			else {
				ResourceCache::lock.unlock();
				ERR_FAIL_MSG("Another resource is loaded from path '" + p_path + "' (possible cyclic resource inclusion).");
			}
		}

		path_cache = p_path;
		// 放进去
		if (!path_cache.is_empty()) {
			ResourceCache::resources[path_cache] = this;
		}
		ResourceCache::lock.unlock();

		_resource_path_changed();
	}

	void Resource::_resource_path_changed() {
	}
	String Resource::GetPath() const { return path_cache; }

	void Resource::reload_from_file() {
		String path = GetPath();
		if (!path.is_resource_file()) {
			return;
		}

		Ref<Resource> s = ResourceLoader::load(ResourceLoader::path_remap(path), get_class(), ResourceFormatLoader::CACHE_MODE_IGNORE);

		if (!s.is_valid()) {
			return;
		}

		CopyFrom(s);
	}

	Error Resource::CopyFrom(const Ref<Resource>& p_resource) {
		ERR_FAIL_COND_V(p_resource.is_null(), ERR_INVALID_PARAMETER);
		if (get_class() != p_resource->get_class()) {
			return ERR_INVALID_PARAMETER;
		}
		ResetState(); // May want to reset state.
		// 是的反射就是这么用的
		// 有了variant，PropertyInfo的实现简单直接
		{
			// using piccolo's reflection
			using Reflection::TypeMeta;
			TypeMeta meta = TypeMeta::newMetaFromName(p_resource->get_class().utf8().get_data());
			Reflection::FieldAccessor* fields;
			int fields_count = meta.getFieldsList(fields);
			Reflection::FieldAccessor field_accessor;

			for (int i = 0; i < fields_count; i++) {
				Reflection::FieldAccessor facc = fields[i];
				if (facc.getFieldName() == "resource_path") {
					continue;
				}
				facc.set(this, facc.get(*p_resource)); // 这么写，真没问题吗。。
				// 然而这里并不知道填什么

			}

		}
		// Variant标类型，StringName标名字，hint加点信息
		//List<PropertyInfo> pi;
		//p_resource->get_property_list(&pi);

		//for (const PropertyInfo& E : pi) {
		//	if (!(E.usage & PROPERTY_USAGE_STORAGE)) {
		//		continue;
		//	}
		//	if (E.name == "resource_path") {
		//		continue; //do not change path
		//	}

		//	set(E.name, p_resource->get(E.name));
		//}
		return OK;
	}
	RID Resource::GetRID() const { return RID(); }
	void Resource::ResetState(){}
	// resourceCache static:
	void ResourceCache::clear() {
		if (!resources.is_empty()) {
			L_PWARNING(resources.size(), " resources still in use at exit.");
		}
		resources.clear();
	}

	// public:
	// static:
	// 这个单例比较适合lock的使用
	bool ResourceCache::has(const String& p_path) {
		lock.lock();
		Resource** res = resources.getptr(p_path);

		if (res && (*res)->get_reference_count() == 0) {
			// This resource is in the process of being deleted, ignore its existence.
			(*res)->path_cache = String();
			resources.erase(p_path);
			res = nullptr;
		}

		lock.unlock();

		if (!res) {
			return false;
		}

		return true;
	}

	Ref<Resource> ResourceCache::get_ref(const String& p_path) {
		Ref<Resource> ref;
		lock.lock();

		Resource** res = resources.getptr(p_path);

		if (res) {
			ref = Ref<Resource>(*res);
		}

		if (res && !ref.is_valid()) {
			// This resource is in the process of being deleted, ignore its existence
			(*res)->path_cache = String();
			resources.erase(p_path);
			res = nullptr;
		}

		lock.unlock();

		return ref;
	}
	void ResourceCache::get_cached_resources(List<Ref<Resource>>* p_resources) {
		lock.lock();

		LocalVector<String> to_remove;

		for (KeyValue<String, Resource*>& E : resources) {
			Ref<Resource> ref = Ref<Resource>(E.value);

			if (!ref.is_valid()) {
				// This resource is in the process of being deleted, ignore its existence
				E.value->path_cache = String();
				to_remove.push_back(E.key);
				continue;
			}

			p_resources->push_back(ref);
		}

		for (const String& E : to_remove) {
			resources.erase(E);
		}

		lock.unlock();
	}


	int ResourceCache::get_cached_resource_count() {
		lock.lock();
		int rc = resources.size();
		lock.unlock();

		return rc;
	}

	HashMap<String, Resource*> ResourceCache::resources;
	Mutex ResourceCache::lock;
}