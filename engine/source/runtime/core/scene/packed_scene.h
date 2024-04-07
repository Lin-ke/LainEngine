#pragma once

#ifndef PACKED_SCENE_H
#define PACKED_SCENE_H

#include "core/io/resource.h"
#include "core/scene/object/gobject.h"
namespace lain {

class SceneState : public RefCounted {
	LCLASS(SceneState, RefCounted);

	Vector<StringName> names;
	Vector<Variant> variants;
	Vector<GObjectPath> gobject_paths;
	Vector<GObjectPath> editable_instances;
	mutable HashMap<GObjectPath, int> gobject_path_cache;
	mutable HashMap<int, int> base_scene_gobject_remap;

	int base_scene_idx = -1;

	enum {
		NO_PARENT_SAVED = 0x7FFFFFFF,
		NAME_INDEX_BITS = 18,
		NAME_MASK = (1 << NAME_INDEX_BITS) - 1,
	};

	struct GobjectData {
		int parent = 0;
		int owner = 0;
		int type = 0;
		int name = 0;
		int instance = 0;
		int index = 0;

		struct Property {
			int name = 0;
			int value = 0;
		};

		Vector<Property> properties;
		Vector<int> groups;
	};

	struct DeferredGObjectPathProperties {
		GObject* base = nullptr;
		StringName property;
		Variant value;
	};

	Vector<GObjectData> gobjects;

	struct ConnectionData {
		int from = 0;
		int to = 0;
		int signal = 0;
		int method = 0;
		int flags = 0;
		int unbinds = 0;
		Vector<int> binds;
	};

	Vector<ConnectionData> connections;

	Error _parse_gobject(GObject* p_owner, GObject* p_gobject, int p_parent_idx, HashMap<StringName, int>& name_map, HashMap<Variant, int, VariantHasher, VariantComparator>& variant_map, HashMap<GObject*, int>& gobject_map, HashMap<GObject*, int>& gobjectpath_map);
	Error _parse_connections(GObject* p_owner, GObject* p_gobject, HashMap<StringName, int>& name_map, HashMap<Variant, int, VariantHasher, VariantComparator>& variant_map, HashMap<GObject*, int>& gobject_map, HashMap<GObject*, int>& gobjectpath_map);

	String path;

	uint64_t last_modified_time = 0;

	static bool disable_placeholders;

	Vector<String> _get_gobject_groups(int p_idx) const;

	int _find_base_scene_gobject_remap_key(int p_idx) const;

#ifdef TOOLS_ENABLED
public:
	typedef void (*InstantiationWarningNotify)(const String& p_warning);

private:
	static InstantiationWarningNotify instantiation_warn_notify;
#endif

protected:
	static void _bind_methods();

public:
	enum {
		FLAG_ID_IS_PATH = (1 << 30),
		TYPE_INSTANTIATED = 0x7FFFFFFF,
		FLAG_INSTANCE_IS_PLACEHOLDER = (1 << 30),
		FLAG_PATH_PROPERTY_IS_gobject = (1 << 30),
		FLAG_PROP_NAME_MASK = FLAG_PATH_PROPERTY_IS_gobject - 1,
		FLAG_MASK = (1 << 24) - 1,
	};

	enum GenEditState {
		GEN_EDIT_STATE_DISABLED,
		GEN_EDIT_STATE_INSTANCE,
		GEN_EDIT_STATE_MAIN,
		GEN_EDIT_STATE_MAIN_INHERITED,
	};

	struct PackState {
		Ref<SceneState> state;
		int gobject = -1;
	};

	static void set_disable_placeholders(bool p_disable);
	static Ref<Resource> get_remap_resource(const Ref<Resource>& p_resource, HashMap<Ref<Resource>, Ref<Resource>>& remap_cache, const Ref<Resource>& p_fallback, GObject* p_for_scene);

	int find_gobject_by_path(const GObjectPath& p_gobject) const;
	Variant get_property_value(int p_gobject, const StringName& p_property, bool& r_found, bool& r_gobject_deferred) const;
	bool is_gobject_in_group(int p_gobject, const StringName& p_group) const;
	bool is_connection(int p_gobject, const StringName& p_signal, int p_to_gobject, const StringName& p_to_method) const;

	void set_bundled_scene(const Dictionary& p_dictionary);
	Dictionary get_bundled_scene() const;

	Error pack(GObject* p_scene);

	void set_path(const String& p_path);
	String get_path() const;

	void clear();
	Error copy_from(const Ref<SceneState>& p_scene_state);

	bool can_instantiate() const;
	GObject* instantiate(GenEditState p_edit_state) const;

	Array setup_resources_in_array(Array& array_to_scan, const SceneState::GObjectData& n, HashMap<Ref<Resource>, Ref<Resource>>& resources_local_to_sub_scene, GObject* gobject, const StringName sname, HashMap<Ref<Resource>, Ref<Resource>>& resources_local_to_scene, int i, GObject** ret_gobjects, SceneState::GenEditState p_edit_state) const;
	Variant make_local_resource(Variant& value, const SceneState::GObjectData& p_gobject_data, HashMap<Ref<Resource>, Ref<Resource>>& p_resources_local_to_sub_scene, GObject* p_gobject, const StringName p_sname, HashMap<Ref<Resource>, Ref<Resource>>& p_resources_local_to_scene, int p_i, GObject** p_ret_gobjects, SceneState::GenEditState p_edit_state) const;
	bool has_local_resource(const Array& p_array) const;

	Ref<SceneState> get_base_scene_state() const;

	void update_instance_resource(String p_path, Ref<PackedScene> p_packed_scene);

	//unbuild API

	int get_gobject_count() const;
	StringName get_gobject_type(int p_idx) const;
	StringName get_gobject_name(int p_idx) const;
	GObjectPath get_gobject_path(int p_idx, bool p_for_parent = false) const;
	GObjectPath get_gobject_owner_path(int p_idx) const;
	Ref<PackedScene> get_gobject_instance(int p_idx) const;
	String get_gobject_instance_placeholder(int p_idx) const;
	bool is_gobject_instance_placeholder(int p_idx) const;
	Vector<StringName> get_gobject_groups(int p_idx) const;
	int get_gobject_index(int p_idx) const;

	int get_gobject_property_count(int p_idx) const;
	StringName get_gobject_property_name(int p_idx, int p_prop) const;
	Variant get_gobject_property_value(int p_idx, int p_prop) const;
	Vector<String> get_gobject_deferred_gobjectpath_properties(int p_idx) const;

	int get_connection_count() const;
	GObjectPath get_connection_source(int p_idx) const;
	StringName get_connection_signal(int p_idx) const;
	GObjectPath get_connection_target(int p_idx) const;
	StringName get_connection_method(int p_idx) const;
	int get_connection_flags(int p_idx) const;
	int get_connection_unbinds(int p_idx) const;
	Array get_connection_binds(int p_idx) const;

	bool has_connection(const GObjectPath& p_gobject_from, const StringName& p_signal, const GObjectPath& p_gobject_to, const StringName& p_method, bool p_no_inheritance = false);

	Vector<GObjectPath> get_editable_instances() const;

	//build API

	int add_name(const StringName& p_name);
	int add_value(const Variant& p_value);
	int add_gobject_path(const GObjectPath& p_path);
	int add_gobject(int p_parent, int p_owner, int p_type, int p_name, int p_instance, int p_index);
	void add_gobject_property(int p_gobject, int p_name, int p_value, bool p_deferred_gobject_path = false);
	void add_gobject_group(int p_gobject, int p_group);
	void set_base_scene(int p_idx);
	void add_connection(int p_from, int p_to, int p_signal, int p_method, int p_flags, int p_unbinds, const Vector<int>& p_binds);
	void add_editable_instance(const GObjectPath& p_path);

	bool remove_group_references(const StringName& p_name);
	bool rename_group_references(const StringName& p_old_name, const StringName& p_new_name);
	HashSet<StringName> get_all_groups();

	virtual void set_last_modified_time(uint64_t p_time) { last_modified_time = p_time; }
	uint64_t get_last_modified_time() const { return last_modified_time; }

	// Used when saving pointers (saves a path property instead).
	static String get_meta_pointer_property(const String& p_property);

#ifdef TOOLS_ENABLED
	static void set_instantiation_warning_notify_func(InstantiationWarningNotify p_warn_notify) { instantiation_warn_notify = p_warn_notify; }
#endif

	SceneState();
};

//VARIANT_ENUM_CAST(SceneState::GenEditState)

class PackedScene : public Resource {
	LCLASS(PackedScene, Resource);
	RES_BASE_EXTENSION("scn");

	Ref<SceneState> state;

	void _set_bundled_scene(const Dictionary& p_scene);
	Dictionary _get_bundled_scene() const;

protected:
	virtual bool editor_can_reload_from_file() override { return false; } // this is handled by editor better
	static void _bind_methods();
	virtual void reset_state() override;

public:
	enum GenEditState {
		GEN_EDIT_STATE_DISABLED,
		GEN_EDIT_STATE_INSTANCE,
		GEN_EDIT_STATE_MAIN,
		GEN_EDIT_STATE_MAIN_INHERITED,
	};

	Error pack(GObject* p_scene);

	void clear();

	bool can_instantiate() const;
	GObject* instantiate(GenEditState p_edit_state = GEN_EDIT_STATE_DISABLED) const;

	void recreate_state();
	void replace_state(Ref<SceneState> p_by);

	virtual void reload_from_file() override;

	virtual void set_path(const String& p_path, bool p_take_over = false) override;
	virtual void set_path_cache(const String& p_path) override;

#ifdef TOOLS_ENABLED
	virtual void set_last_modified_time(uint64_t p_time) override {
		Resource::set_last_modified_time(p_time);
		state->set_last_modified_time(p_time);
	}

#endif
	Ref<SceneState> get_state() const;

	PackedScene();
};

VARIANT_ENUM_CAST(PackedScene::GenEditState)
}

#endif // PACKED_SCENE_H
