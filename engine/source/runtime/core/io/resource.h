#pragma once
#ifndef RESOURCE_H
#define RESOURCE_H
#include "core/object/refcounted.h"
#include "core/object/object.h"
#include "core/meta/reflection/reflection.h"
#include "core/string/ustring.h"
#include "core/templates/self_list.h"

namespace lain {
	REFLECTION_TYPE(Resource)
	CLASS(Resource: public RefCounted, WhiteList) {
		REFLECTION_BODY(Resource);

		friend class ResourceCache;

		LCLASS(Resource, RefCounted)
	private:
		META(Enable);
		String name;
		META(Enable);
		String path_cache;
		META(Enable);
		String scene_id;
		//SelfList<Resource> remapped_list;
	public:
		void SetName(const String& p_name);
		String GetName() const;

		void SetPath(const String& p_path, bool p_take_over);
		String GetPath() const;
		
		void SetPathCache(const String& p_path) { path_cache = p_path; }
		void set_scene_unique_id(const String& p_id);
		String get_scene_unique_id() const;
		virtual Error CopyFrom(const Ref<Resource>& p_resource);


		virtual Ref<Resource> duplicate(bool p_subresources = false) const;
		virtual void reload_from_file();
		virtual void ResetState();
		virtual void _resource_path_changed();

	protected:
		void _set_path(const String& p_path) { SetPath(p_path, false); }
		void _take_over_path(const String& p_path) { SetPath(p_path, true); }
		virtual void reset_local_to_scene() {}

	};
	class ResourceCache {
		friend class Resource;
		friend class ResourceLoader; //need the lock
		static Mutex lock;
		static HashMap<String, Resource*> resources;

		friend void unregister_core_types();
		static void clear();
		friend void register_core_types();

	public:
		static bool has(const String& p_path);

		static Ref<Resource> get_ref(const String& p_path);
		static void get_cached_resources(List<Ref<Resource>>* p_resources);
		static int get_cached_resource_count();
	};
}

#endif