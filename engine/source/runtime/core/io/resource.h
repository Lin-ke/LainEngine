#pragma once
#ifndef RESOURCE_H
#define RESOURCE_H
#include "core/meta/class_db.h"
#include "core/object/refcounted.h"
#include "core/meta/reflection/reflection_marcos.h"
#include "core/string/ustring.h"
#include "core/templates/self_list.h"
#include "core/os/rwlock.h"
#define RES_BASE_EXTENSION(m_ext)                                                                                   \
public:                                                                                                             \
	static void register_custom_data_to_otdb() { ClassDB::add_resource_base_extension(m_ext, get_class_static()); } \
	virtual String get_base_extension() const override { return m_ext; }                                            \
                                                                                                                    \
private:

namespace lain {
	class GObject;
	class Resource: public RefCounted {
		friend class ResourceCache;
		LCLASS(Resource, RefCounted);

	public:
		static void register_custom_data_to_otdb() { ClassDB::add_resource_base_extension("res", get_class_static()); }
		virtual String get_base_extension() const { return "res"; }
	private:
		String name;
		String path_cache;
		String scene_unique_id;
		//SelfList<Resource> remapped_list;
	public:
		void SetName(const String& p_name);
		String GetName() const;

		virtual void set_path(const String& p_path, bool p_take_over = false);
		String GetPath() const;
		
		virtual void set_pathCache(const String& p_path) { path_cache = p_path; }
		static String generate_scene_unique_id();

		String get_id_for_path(const String& p_path) const;
		void set_id_for_path(const String& p_path, const String& p_id);

		void set_scene_unique_id(const String& p_id);
		String get_scene_unique_id() const;
		virtual Error CopyFrom(const Ref<Resource>& p_resource);

		virtual RID GetRID() const; // some resources may offer conversion to RID

		virtual void emit_changed();
		/// <summary>
		/// @TODO
		/// </summary>
		virtual Ref<Resource> duplicate(bool p_subresources = false) const { return Ref<Resource>(); }
		virtual void reload_from_file();
		virtual void ResetState();
		virtual void _resource_path_changed();
		
		_FORCE_INLINE_ bool IsBuiltIn() const { return path_cache.is_empty() || path_cache.contains("::") || path_cache.begins_with("local://"); }
		~Resource();
	protected:
		void _set_path(const String& p_path) { set_path(p_path, false); }
		void _take_over_path(const String& p_path) { set_path(p_path, true); }
		virtual void reset_local_to_scene() {}
		virtual GObject* get_local_scene(){return nullptr;}
	private:

	};
	class ResourceCache {
		friend class Resource;
		friend class ResourceLoader; //need the lock
		static Mutex lock;
		static RWLock path_cache_lock;
		static HashMap<String, HashMap<String, String>> resource_path_cache; // Each tscn has a set of resource paths and IDs.

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