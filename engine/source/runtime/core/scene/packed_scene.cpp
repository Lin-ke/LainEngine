#include "packed_scene.h"
#include "core/io/resource_loader.h"

namespace lain {
	bool SceneState::disable_placeholders = false;

	void SceneState::set_disable_placeholders(bool p_disable) {
		disable_placeholders = p_disable;
	}


	void PackedScene::SetPath(const String& p_path, bool p_take_over) {
		state->set_path(p_path);
		Resource::SetPath(p_path, p_take_over);
	}
	void PackedScene::SetPathCache(const String& p_path) {
		state->set_path(p_path);
		Resource::SetPathCache(p_path);
	}
	PackedScene::PackedScene() { state = Ref<SceneState>(memnew(SceneState)); }

	bool PackedScene::can_instantiate() const {
		return state->can_instantiate();
	}

	GObject* PackedScene::instantiate(GenEditState p_edit_state) const {
#ifndef TOOLS_ENABLED
		ERR_FAIL_COND_V_MSG(p_edit_state != GEN_EDIT_STATE_DISABLED, nullptr, "Edit state is only for editors, does not work without tools compiled.");
#endif

		GObject* s = state->instantiate((SceneState::GenEditState)p_edit_state);
		if (!s) {
			return nullptr;
		}

		if (p_edit_state != GEN_EDIT_STATE_DISABLED) {
			s->set_scene_instance_state(state);
		}

		if (!IsBuiltIn()) {
			s->set_scene_file_path(GetPath());
		}

		//s->notification(GObject::NOTIFICATION_SCENE_INSTANTIATED);

		return s;
	}

	void SceneState::set_base_scene(int p_idx) {
		ERR_FAIL_INDEX(p_idx, variants.size());
		base_scene_idx = p_idx;
	}

	bool SceneState::can_instantiate() const {
		return gobjects.size() > 0;
	}
	int SceneState::get_gobject_count() const {
		return gobjects.size();
	}

	int SceneState::get_gobject_index(int p_idx) const {
		ERR_FAIL_INDEX_V(p_idx, gobjects.size(), -1);
		return gobjects[p_idx].index;
	}

	GObjectPath SceneState::get_gobject_path(int p_idx, bool p_for_parent) const {
		ERR_FAIL_INDEX_V(p_idx, gobjects.size(), GObjectPath());

		if (gobjects[p_idx].parent < 0 || gobjects[p_idx].parent == NO_PARENT_SAVED) {
			if (p_for_parent) {
				return GObjectPath();
			}
			else {
				return GObjectPath(".");
			}
		}

		Vector<StringName> sub_path;
		GObjectPath base_path;
		int nidx = p_idx;
		while (true) {
			if (gobjects[nidx].parent == NO_PARENT_SAVED || gobjects[nidx].parent < 0) {
				sub_path.insert(0, ".");
				break;
			}

			if (!p_for_parent || p_idx != nidx) {
				sub_path.insert(0, names[gobjects[nidx].name]);
			}

			if (gobjects[nidx].parent & FLAG_ID_IS_PATH) {
				base_path = gobject_paths[gobjects[nidx].parent & FLAG_MASK];
				break;
			}
			else {
				nidx = gobjects[nidx].parent & FLAG_MASK;
			}
		}

		for (int i = base_path.get_name_count() - 1; i >= 0; i--) {
			sub_path.insert(0, base_path.get_name(i));
		}

		if (sub_path.is_empty()) {
			return GObjectPath(".");
		}

		return GObjectPath(sub_path, false);
	}

	StringName SceneState::get_gobject_name(int p_idx) const {
		return names[gobjects[p_idx].name];
	}
	StringName SceneState::get_gobject_type(int p_idx) const {
		return names[gobjects[p_idx].type];
	}
	Ref<PackedScene> SceneState::get_gobject_instance(int p_idx) const {
		ERR_FAIL_INDEX_V(p_idx, gobjects.size(), Ref<PackedScene>());

		if (gobjects[p_idx].instance >= 0) { // 是实例化的
			if (gobjects[p_idx].instance & FLAG_INSTANCE_IS_PLACEHOLDER) {
				return Ref<PackedScene>();
			}
			else {
				return variants[gobjects[p_idx].instance & FLAG_MASK];
			}
		}
		else if (gobjects[p_idx].parent < 0 || gobjects[p_idx].parent == NO_PARENT_SAVED) {
			if (base_scene_idx >= 0) {
				return variants[base_scene_idx];
			}
		}

		return Ref<PackedScene>();
	}

	GObjectPath SceneState::get_gobject_owner_path(int p_idx) const {
		ERR_FAIL_INDEX_V(p_idx, gobjects.size(), GObjectPath());
		if (gobjects[p_idx].owner < 0 || gobjects[p_idx].owner == NO_PARENT_SAVED) {
			return GObjectPath(); //root likely
		}
		if (gobjects[p_idx].owner & FLAG_ID_IS_PATH) {
			return gobject_paths[gobjects[p_idx].owner & FLAG_MASK];
		}
		else {
			return get_gobject_path(gobjects[p_idx].owner & FLAG_MASK);
		}
	}
	int SceneState::add_name(const StringName& p_name) {
		names.push_back(p_name);
		return names.size() - 1;
	}

	void SceneState::add_gobject_group(int p_node, int p_group) {
		ERR_FAIL_INDEX(p_node, gobjects.size());
		ERR_FAIL_INDEX(p_group, names.size());
		gobjects.write[p_node].groups.push_back(p_group);
	}

	int SceneState::add_value(const Variant& p_value) {
		variants.push_back(p_value);
		return variants.size() - 1;
	}

	int SceneState::add_gobject_path(const GObjectPath& p_path) {
		gobject_paths.push_back(p_path);
		return (gobject_paths.size() - 1) | FLAG_ID_IS_PATH;
	}

	int SceneState::add_gobject(int p_parent, int p_owner, int p_type, int p_name, int p_instance, int p_index) {
		GObjectData nd;
		nd.parent = p_parent;
		nd.owner = p_owner;
		nd.type = p_type;
		nd.name = p_name;
		nd.instance = p_instance;
		nd.index = p_index;

		gobjects.push_back(nd);

		return gobjects.size() - 1;
	}
	// 复制指针 还是直接指？因为vector是有引用计数的
	void SceneState::add_components(int p_index,const Vector<Component*>& p_components ) {
		ERR_FAIL_INDEX(p_index, gobjects.size());
		gobjects.write[p_index].components = p_components;
	}
	void SceneState::add_instance_res(int p_index, const String& p_ins_res) {
		ERR_FAIL_INDEX(p_index, gobjects.size());
		gobjects.write[p_index].node_ins_res = p_ins_res;
	}

	Vector<Component*> SceneState::get_gobject_components(int p_index) {
		return gobjects[p_index].components;
	}
	String SceneState::get_gobject_insres(int p_index) {
		return gobjects[p_index].node_ins_res;
	}
	GObject* SceneState::instantiate(GenEditState p_edit_state) const {

#define GOBJ_FROM_ID(p_name, p_id)                      \
	GObject *p_name;                                       \
	if (p_id & FLAG_ID_IS_PATH) {                       \
		GObjectPath np = gobject_paths[p_id & FLAG_MASK];     \
		p_name = ret_gobjects[0]->get_gobject_or_null(np);    \
	} else {                                            \
		ERR_FAIL_INDEX_V(p_id &FLAG_MASK, nc, nullptr); \
		p_name = ret_gobjects[p_id & FLAG_MASK];           \
	}
		int nc = gobjects.size();
		ERR_FAIL_COND_V_MSG(nc == 0, nullptr, vformat("Failed to instantiate scene state of \"%s\", node count is 0. Make sure the PackedScene resource is valid.", path));
		const StringName* snames = nullptr;
		int sname_count = names.size();
		snames = sname_count > 0 ? names.ptr() : nullptr;
		const Variant* props = nullptr;
		int prop_count = variants.size();

		props = prop_count > 0 ? variants.ptr() : nullptr;

		const GObjectData* nd = gobjects.ptr();
		GObject** ret_gobjects = (GObject**)alloca(sizeof(GObject*)*nc); // 为什么要在栈上分配一些指针的空间？
		bool gen_gobject_path_cache = p_edit_state != GEN_EDIT_STATE_DISABLED && gobject_path_cache.is_empty(); // 重新生成路径
		// GObjects where instantiation failed (because something is missing.)
		List<GObject*> stray_instances;

		for (int i = 0; i < nc; ++i) {
			const GObjectData& n = nd[i];
			GObject* parent = nullptr;
			// 一些安全性代码
			if (i > 0) {
				ERR_FAIL_COND_V_MSG(n.parent == -1, nullptr, vformat("Invalid scene: node %s does not specify its parent node.", snames[n.name]));
				GOBJ_FROM_ID(nparent, n.parent);
				parent = nparent;
			}
			else {
				// i == 0 is root node.
				ERR_FAIL_COND_V_MSG(n.parent != -1, nullptr, vformat("Invalid scene: root node %s cannot specify a parent node.", snames[n.name]));
				ERR_FAIL_COND_V_MSG(n.type == TYPE_INSTANTIATED && base_scene_idx < 0, nullptr, vformat("Invalid scene: root node %s in an instance, but there's no base scene.", snames[n.name]));
			}
			GObject* gobj = nullptr;
			if (i == 0 && base_scene_idx >=0) { 
				// Scene inheritance on root node. 根节点是继承节点，实例化节点不许成为根
				Ref<PackedScene> sdata = props[base_scene_idx];
				ERR_FAIL_COND_V(!sdata.is_valid(), nullptr);
				gobj = sdata->instantiate(p_edit_state == GEN_EDIT_STATE_DISABLED ? PackedScene::GEN_EDIT_STATE_DISABLED : PackedScene::GEN_EDIT_STATE_INSTANCE); //only main gets main edit state
				ERR_FAIL_NULL_V(gobj, nullptr);
				if (p_edit_state != GEN_EDIT_STATE_DISABLED) {
					gobj->set_scene_inherited_state(sdata->get_state()); 
				}
			}
			else if(n.instance >= 0){  // node data中有一些实例化
				// 占位符
				if (n.instance & FLAG_INSTANCE_IS_PLACEHOLDER) {
					const String scene_path = props[n.instance & FLAG_MASK];
					if (disable_placeholders) {
						Ref<PackedScene> sdata = ResourceLoader::load(scene_path, "PackedScene");
						if (sdata.is_valid()) {
							gobj = sdata->instantiate(p_edit_state == GEN_EDIT_STATE_DISABLED ? PackedScene::GEN_EDIT_STATE_DISABLED : PackedScene::GEN_EDIT_STATE_INSTANCE);
							ERR_FAIL_NULL_V(gobj, nullptr);
						}
						// @TODO missing node
						/*else if (ResourceLoader::is_creating_missing_resources_if_class_unavailable_enabled()) {
							missing_node = memnew(MissingGObject);
							missing_node->set_original_scene(scene_path);
							missing_node->set_recording_properties(true);
							node = missing_node;
						}*/ 
						else {
							ERR_FAIL_V_MSG(nullptr, "Placeholder scene is missing.");
						}
					}
					else { 
						// @TODO 占位符，延迟加载
						/*InstancePlaceholder* ip = memnew(InstancePlaceholder);
						ip->set_instance_path(scene_path);
						node = ip;*/
					}
					//gobj->set_scene_instance_load_placeholder(true);
				}
				else {
					Ref<Resource> res = props[n.instance & FLAG_MASK];
					Ref<PackedScene> sdata = res;
					if (sdata.is_valid()) {
						gobj = sdata->instantiate(p_edit_state == GEN_EDIT_STATE_DISABLED ? PackedScene::GEN_EDIT_STATE_DISABLED : PackedScene::GEN_EDIT_STATE_INSTANCE);
						ERR_FAIL_NULL_V_MSG(gobj, nullptr, vformat("Failed to load scene dependency: \"%s\". Make sure the required scene is valid.", sdata->GetPath()));
					}
					/*else if (ResourceLoader::is_creating_missing_resources_if_class_unavailable_enabled()) {
						missing_node = memnew(MissingGObject);
#ifdef TOOLS_ENABLED
						if (res.is_valid()) {
							missing_node->set_original_scene(res->get_meta("__load_path__", ""));
						}
#endif
						missing_node->set_recording_properties(true);
						node = missing_node;
					}*/
					else {
						ERR_FAIL_V_MSG(nullptr, "Scene instance is missing.");
					}
				}

			} 
			else if (n.type == TYPE_INSTANTIATED) {
				// Get the node from somewhere, it likely already exists from another instance.
				if (parent) {
					gobj = parent->_get_child_by_name(snames[n.name]);
#ifdef DEBUG_ENABLED
					if (!gobj) {
						WARN_PRINT(String("GObject '" + String(ret_gobjects[0]->get_path_to(parent)) + "/" + String(snames[n.name]) + "' was modified from inside an instance, but it has vanished.").ascii().get_data());
					}
#endif
				}
			}
			else {
				// GObject belongs to this scene and must be created. @TODO
				//Object* obj = ClassDB::instantiate(snames[n.type]);
				Object* obj = nullptr;
				if (n.node_ins_res == "{}") {
					obj = static_cast<Object*>(Reflection::TypeMeta::memnewByName(SCSTR(snames[n.type])));
				}
				else {
					obj = static_cast<Object*>(Reflection::TypeMeta::newFromNameAndJson(SCSTR(snames[n.type]), CSTR(n.node_ins_res)).m_instance);
				}
				gobj = Object::cast_to<GObject>(obj);

				if (!gobj) {
					if (obj) {
						memdelete(obj);
						obj = nullptr;
					}
					// @TODO: missing
					/*if (ResourceLoader::is_creating_missing_resources_if_class_unavailable_enabled()) {
						missing_node = memnew(MissingGObject);
						missing_node->set_original_class(snames[n.type]);
						missing_node->set_recording_properties(true);
						node = missing_node;
						obj = missing_node;
					}*/
					else {
						WARN_PRINT(vformat("GObject %s of type %s cannot be created. A placeholder will be created instead.", snames[n.name], snames[n.type]).ascii().get_data());
//						if (n.parent >= 0 && n.parent < nc && ret_gobjects[n.parent]) {
//							if (Object::cast_to<Control>(ret_gobjects[n.parent])) {
//								obj = memnew(Control);
//							}
//							else if (Object::cast_to<GObject2D>(ret_gobjects[n.parent])) {
//								obj = memnew(GObject2D);
//#ifndef _3D_DISABLED
//							}
//							else if (Object::cast_to<GObject3D>(ret_gobjects[n.parent])) {
//								obj = memnew(GObject3D);
//#endif // _3D_DISABLED
//							}
//						}
//
						if (!obj) {
							obj = memnew(GObject);
						}
//
						gobj = Object::cast_to<GObject>(obj);
					}
				}
			} 
			// end of instantiate
			if (gobj) {
				// 添加 properties 和 关系
				for (int j = 0; j < n.groups.size(); j++) {
					ERR_FAIL_INDEX_V(n.groups[j], sname_count, nullptr);
					gobj->add_to_group(snames[n.groups[j]], true);
				}
				if (n.instance >= 0 || n.type != TYPE_INSTANTIATED || i == 0) {
					//if node was not part of instance, must set its name, parenthood and ownership
					if (i > 0) {
						if (parent) {
							bool pending_add = true;
							if (pending_add) {
								parent->_add_child_nocheck(gobj, snames[n.name]);
							}
							if (n.index >= 0 && n.index < parent->get_child_count() - 1) {
								parent->move_child(gobj, n.index);
							}
						}
						else {
							//it may be possible that an instantiated scene has changed
							//and the node has nowhere to go anymore
							stray_instances.push_back(gobj); //can't be added, go to stray list
						}
					}
					else {
						//if (Engine::get_singleton()->is_editor_hint()) {
						//	//validate name if using editor, to avoid broken
						//	node->set_name(snames[n.name]);
						//}
						//else {
						//	node->_set_name_nocheck(snames[n.name]);
						//}
						gobj->_set_name_nocheck(snames[n.name]);
					}
				}

				if (n.owner >= 0) {
					GOBJ_FROM_ID(owner, n.owner);
					if (owner) {
						gobj->_set_owner_nocheck(owner);
						if (gobj->data.unique_name_in_owner) {
							gobj->_acquire_unique_name_in_owner();
						}
					}
				}
				for (int idx = 0; n.instance < 0 && idx < n.components.size(); ++idx) // if n.cowdata = nullptr, return 0
				{
					gobj->add_component(n.components[idx]);
				}

				
			}
			ret_gobjects[i] = gobj;
			if (gobj && gen_gobject_path_cache && ret_gobjects[0]) {
				GObjectPath n2 = ret_gobjects[0]->get_path_to(gobj);
				gobject_path_cache[n2] = i;
			}

		}
		//remove nodes that could not be added, likely as a result that
		while (stray_instances.size()) { 
			memdelete(stray_instances.front()->get());
			stray_instances.pop_front();
		}

		for (int i = 0; i < editable_instances.size(); i++) {
			GObject* ei = ret_gobjects[0]->get_gobject_or_null(editable_instances[i]);
			if (ei) {
				ret_gobjects[0]->set_editable_instance(ei, true);
			}
		}
		return ret_gobjects[0]; // root


	}
	static int _vm_get_variant(const Variant& p_variant, HashMap<Variant, int, VariantHasher, VariantComparator>& variant_map) {
		if (variant_map.has(p_variant)) {
			return variant_map[p_variant];
		}

		int idx = variant_map.size();
		variant_map[p_variant] = idx;
		return idx;
	}
	static int _nm_get_string(const String& p_string, HashMap<StringName, int>& name_map) {
		if (name_map.has(p_string)) {
			return name_map[p_string];
		}

		int idx = name_map.size();
		name_map[p_string] = idx;
		return idx;
	}

	void SceneState::clear() {
		names.clear();
		variants.clear();
		gobjects.clear();
		//connections.clear();
		gobject_path_cache.clear();
		gobject_paths.clear();
		editable_instances.clear();
		base_scene_idx = -1;
	}


	String SceneState::get_path() const {
		return path;
	}

	Error SceneState::pack(GObject* p_scene) {
		ERR_FAIL_NULL_V(p_scene, ERR_INVALID_PARAMETER);

		clear();

		GObject* scene = p_scene;

		HashMap<StringName, int> name_map;
		HashMap<Variant, int, VariantHasher, VariantComparator> variant_map;
		HashMap<GObject*, int> gobject_map;
		HashMap<GObject*, int> nodepath_map;

		// If using scene inheritance, pack the scene it inherits from.
		if (scene->get_scene_inherited_state().is_valid()) {
			String scene_path = scene->get_scene_inherited_state()->get_path();
			Ref<PackedScene> instance = ResourceLoader::load(scene_path);
			if (instance.is_valid()) {
				base_scene_idx = _vm_get_variant(instance, variant_map);
			}
		}

		// Instanced, only direct sub-scenes are supported of course.
		Error err = _parse_gobject(scene, scene, -1, name_map, variant_map, gobject_map, nodepath_map);
		if (err) {
			clear();
			ERR_FAIL_V(err);
		}

		/*err = _parse_connections(scene, scene, name_map, variant_map, gobject_map, nodepath_map);
		if (err) {
			clear();
			ERR_FAIL_V(err);
		}*/

		names.resize(name_map.size());

		for (const KeyValue<StringName, int>& E : name_map) {
			names.write[E.value] = E.key;
		}

		variants.resize(variant_map.size());

		for (const KeyValue<Variant, int>& E : variant_map) {
			int idx = E.value;
			variants.write[idx] = E.key;
		}

		gobject_paths.resize(nodepath_map.size());
		for (const KeyValue<GObject*, int>& E : nodepath_map) {
			gobject_paths.write[E.value] = scene->get_path_to(E.key);
		}

		//if (Engine::get_singleton()->is_editor_hint()) {
		//	// Build node path cache
		//	for (const KeyValue<GObject*, int>& E : gobject_map) {
		//		gobject_path_cache[scene->get_path_to(E.key)] = E.value;
		//	}
		//}

		return OK;
	}
	Error SceneState::_parse_gobject(GObject* p_owner, GObject* p_node, int p_parent_idx, HashMap<StringName, int>& name_map, HashMap<Variant, int, VariantHasher, VariantComparator>& variant_map, HashMap<GObject*, int>& gobject_map, HashMap<GObject*, int>& nodepath_map) {
		// this function handles all the work related to properly packing scenes, be it
		// instantiated or inherited.
		// given the complexity of this process, an attempt will be made to properly
		// document it. if you fail to understand something, please ask!

		//discard nodes that *do not* belong to be processed
		if (p_node != p_owner && p_node->get_owner() != p_owner && !p_owner->is_editable_instance(p_node->get_owner())) {
			return OK;
		}

		bool is_editable_instance = false;

		// save the child instantiated scenes that are chosen as editable, so they can be restored
		// upon load back
		if (p_node != p_owner && !p_node->get_scene_file_path().is_empty() && p_owner->is_editable_instance(p_node)) {
			editable_instances.push_back(p_owner->get_path_to(p_node));
			// GObject is the root of an editable instance.
			is_editable_instance = true;
		}
		else if (p_node->get_owner() && p_owner->is_ancestor_of(p_node->get_owner()) && p_owner->is_editable_instance(p_node->get_owner())) {
			// GObject is part of an editable instance.
			is_editable_instance = true;
		}

		GObjectData nd;

		nd.name = _nm_get_string(p_node->get_name(), name_map);
		nd.instance = -1; //not instantiated by default

		//really convoluted condition, but it basically checks that index is only saved when part of an inherited scene OR the node parent is from the edited scene
		if (p_owner->get_scene_inherited_state().is_null() && (p_node == p_owner || (p_node->get_owner() == p_owner && (p_node->get_parent() == p_owner || p_node->get_parent()->get_owner() == p_owner)))) {
			//do not save index, because it belongs to saved scene and scene is not inherited
			nd.index = -1;
		}
		else if (p_node == p_owner) {
			//This (hopefully) happens if the node is a scene root, so its index is irrelevant.
			nd.index = -1;
		}
		else {
			//part of an inherited scene, or parent is from an instantiated scene
			nd.index = p_node->get_index();
		}

		// if this node is part of an instantiated scene or sub-instantiated scene
		// we need to get the corresponding instance states.
		// with the instance states, we can query for identical properties/groups
		// and only save what has changed

		bool instantiated_by_owner = false;
		Vector<SceneState::PackState> states_stack = _get_gobject_states_stack(p_node, p_owner, &instantiated_by_owner);

		if (!p_node->get_scene_file_path().is_empty() && p_node->get_owner() == p_owner && instantiated_by_owner) {
			if (p_node->get_scene_instance_load_placeholder()) {
				//it's a placeholder, use the placeholder path
				nd.instance = _vm_get_variant(p_node->get_scene_file_path(), variant_map);
				nd.instance |= FLAG_INSTANCE_IS_PLACEHOLDER;
			}
			else {
				//must instance ourselves
				Ref<PackedScene> instance = ResourceLoader::load(p_node->get_scene_file_path());
				if (!instance.is_valid()) {
					return ERR_CANT_OPEN;
				}

				nd.instance = _vm_get_variant(instance, variant_map);
			}
		}

		// all setup, we then proceed to check all properties for the node
		// and save the ones that are worth saving

		//List<PropertyInfo> plist;
		//p_node->get_property_list(&plist);

		//Array pinned_props = _sanitize_gobject_pinned_properties(p_node);
		//Dictionary missing_resource_properties = p_node->get_meta(META_MISSING_RESOURCES, Dictionary());

		//for (const PropertyInfo& E : plist) {
		//	if (!(E.usage & PROPERTY_USAGE_STORAGE)) {
		//		continue;
		//	}

		//	if (E.name == META_PROPERTY_MISSING_RESOURCES) {
		//		continue; // Ignore this property when packing.
		//	}

		//	// If instance or inheriting, not saving if property requested so.
		//	if (!states_stack.is_empty()) {
		//		if ((E.usage & PROPERTY_USAGE_NO_INSTANCE_STATE)) {
		//			continue;
		//		}
		//	}

			/*StringName name = E.name;
			Variant value = p_node->get(name);
			bool use_deferred_gobject_path_bit = false;*/

		//	if (E.type == Variant::OBJECT && E.hint == PROPERTY_HINT_NODE_TYPE) {
		//		if (value.get_type() == Variant::OBJECT) {
		//			if (GObject* n = Object::cast_to<GObject>(value)) {
		//				value = p_node->get_path_to(n);
		//			}
		//			use_deferred_gobject_path_bit = true;
		//		}
		//		if (value.get_type() != Variant::NODE_PATH) {
		//			continue; //was never set, ignore.
		//		}
		//	}
		//	else if (E.type == Variant::OBJECT && missing_resource_properties.has(E.name)) {
		//		// Was this missing resource overridden? If so do not save the old value.
		//		Ref<Resource> ures = value;
		//		if (ures.is_null()) {
		//			value = missing_resource_properties[E.name];
		//		}
		//	}
		//	else if (E.type == Variant::ARRAY && E.hint == PROPERTY_HINT_TYPE_STRING) {
		//		int hint_subtype_separator = E.hint_string.find(":");
		//		if (hint_subtype_separator >= 0) {
		//			String subtype_string = E.hint_string.substr(0, hint_subtype_separator);
		//			int slash_pos = subtype_string.find("/");
		//			PropertyHint subtype_hint = PropertyHint::PROPERTY_HINT_NONE;
		//			if (slash_pos >= 0) {
		//				subtype_hint = PropertyHint(subtype_string.get_slice("/", 1).to_int());
		//				subtype_string = subtype_string.substr(0, slash_pos);
		//			}
		//			Variant::Type subtype = Variant::Type(subtype_string.to_int());

		//			if (subtype == Variant::OBJECT && subtype_hint == PROPERTY_HINT_NODE_TYPE) {
		//				use_deferred_gobject_path_bit = true;
		//				Array array = value;
		//				Array new_array;
		//				for (int i = 0; i < array.size(); i++) {
		//					Variant elem = array[i];
		//					if (elem.get_type() == Variant::OBJECT) {
		//						if (GObject* n = Object::cast_to<GObject>(elem)) {
		//							new_array.push_back(p_node->get_path_to(n));
		//							continue;
		//						}
		//					}
		//					new_array.push_back(elem);
		//				}
		//				value = new_array;
		//			}
		//		}
		//	}

		//	if (!pinned_props.has(name)) {
		//		bool is_valid_default = false;
		//		Variant default_value = PropertyUtils::get_property_default_value(p_node, name, &is_valid_default, &states_stack, true);

		//		if (is_valid_default && !PropertyUtils::is_property_value_different(value, default_value)) {
		//			if (value.get_type() == Variant::ARRAY && has_local_resource(value)) {
		//				// Save anyway
		//			}
		//			else if (value.get_type() == Variant::DICTIONARY) {
		//				Dictionary dictionary = value;
		//				if (!has_local_resource(dictionary.values()) && !has_local_resource(dictionary.keys())) {
		//					continue;
		//				}
		//			}
		//			else {
		//				continue;
		//			}
		//		}
		//	}

		//	GObjectData::Property prop;
		//	prop.name = _nm_get_string(name, name_map);
		//	prop.value = _vm_get_variant(value, variant_map);
		//	if (use_deferred_gobject_path_bit) {
		//		prop.name |= FLAG_PATH_PROPERTY_IS_NODE;
		//	}
		//	nd.properties.push_back(prop);
		//}

		//// save the groups this node is into
		//// discard groups that come from the original scene

		//List<GObject::GroupInfo> groups;
		//p_node->get_groups(&groups);
		//for (const GObject::GroupInfo& gi : groups) {
		//	if (!gi.persistent) {
		//		continue;
		//	}

		//	bool skip = false;
		//	for (const SceneState::PackState& ia : states_stack) {
		//		//check all levels of pack to see if the group was added somewhere
		//		if (ia.state->is_gobject_in_group(ia.node, gi.name)) {
		//			skip = true;
		//			break;
		//		}
		//	}

		//	if (skip) {
		//		continue;
		//	}

		//	nd.groups.push_back(_nm_get_string(gi.name, name_map));
		//}

		// save the right owner
		// for the saved scene root this is -1
		// for nodes of the saved scene this is 0
		// for nodes of instantiated scenes this is >0
		nd.components = p_node->get_components();
		if (p_node == p_owner) {
			//saved scene root
			nd.owner = -1;
		}
		else if (p_node->get_owner() == p_owner) {
			//part of saved scene
			nd.owner = 0;
		}
		else {
			nd.owner = -1;
		}

		if (states_stack.is_empty() && !is_editable_instance) {
			nd.type = _nm_get_string(p_node->get_class(), name_map);
		}
		else {
			nd.type = TYPE_INSTANTIATED;
		}
		//MissingGObject* missing_node = Object::cast_to<MissingGObject>(p_node);

		//// Save the right type. If this node was created by an instance
		//// then flag that the node should not be created but reused
		//if (states_stack.is_empty() && !is_editable_instance) {
		//	//This node is not part of an instantiation process, so save the type.
		//	if (missing_node != nullptr) {
		//		// It's a missing node (type non existent on load).
		//		nd.type = _nm_get_string(missing_node->get_original_class(), name_map);
		//	}
		//	else {
		//		nd.type = _nm_get_string(p_node->get_class(), name_map);
		//	}
		//}
		//else {
		//	// this node is part of an instantiated process, so do not save the type.
		//	// instead, save that it was instantiated
		//	nd.type = TYPE_INSTANTIATED;
		//}

		// determine whether to save this node or not
		// if this node is part of an instantiated sub-scene, we can skip storing it if basically
		// no properties changed and no groups were added to it.
		// below condition is true for all nodes of the scene being saved, and ones in subscenes
		// that hold changes

		bool save_node = nd.properties.size() || nd.groups.size(); // some local properties or groups exist
		save_node = save_node || p_node == p_owner; // owner is always saved
		save_node = save_node || (p_node->get_owner() == p_owner && instantiated_by_owner); //part of scene and not instanced

		int idx = gobjects.size();
		int parent_node = NO_PARENT_SAVED;

		if (save_node) {
			//don't save the node if nothing and subscene

			gobject_map[p_node] = idx;

			//ok validate parent node
			if (p_parent_idx == NO_PARENT_SAVED) {
				int sidx;
				if (nodepath_map.has(p_node->get_parent())) {
					sidx = nodepath_map[p_node->get_parent()];
				}
				else {
					sidx = nodepath_map.size();
					nodepath_map[p_node->get_parent()] = sidx;
				}

				nd.parent = FLAG_ID_IS_PATH | sidx;
			}
			else {
				nd.parent = p_parent_idx;
			}

			parent_node = idx;
			nd.node_ins_res = Reflection::TypeMeta::writeByName(CSTR(p_node->get_class()), static_cast<void*>(p_node)).dump().c_str();
			if (nd.node_ins_res == "null") {
				WARN_PRINT("JSON Serializer NODE ERROR");
			}
			gobjects.push_back(nd);
		}

		for (int i = 0; i < p_node->get_child_count(); i++) {
			GObject* c = p_node->get_child(i);
			Error err = _parse_gobject(p_owner, c, parent_node, name_map, variant_map, gobject_map, nodepath_map);
			if (err) {
				return err;
			}
		}

		return OK;
	}
	Ref<SceneState> SceneState::get_base_scene_state() const {
		if (base_scene_idx >= 0) {
			Ref<PackedScene> ps = variants[base_scene_idx];
			if (ps.is_valid()) {
				return ps->get_state();
			}
		}

		return Ref<SceneState>();
	}
	int SceneState::_find_base_scene_gobject_remap_key(int p_idx) const {
		for (const KeyValue<int, int>& E : base_scene_gobject_remap) {
			if (E.value == p_idx) {
				return E.key;
			}
		}
		return -1;
	}


	int SceneState::find_gobject_by_path(const GObjectPath& p_node) const {
		ERR_FAIL_COND_V_MSG(gobject_path_cache.is_empty(), -1, "This operation requires the node cache to have been built.");

		if (!gobject_path_cache.has(p_node)) { // not in cache
			if (get_base_scene_state().is_valid()) {
				int idx = get_base_scene_state()->find_gobject_by_path(p_node);
				if (idx != -1) {
					int rkey = _find_base_scene_gobject_remap_key(idx);
					if (rkey == -1) {
						rkey = gobjects.size() + base_scene_gobject_remap.size();
						base_scene_gobject_remap[rkey] = idx;
					}
					return rkey;
				}
			}
			return -1;
		}

		int nid = gobject_path_cache[p_node];

		if (get_base_scene_state().is_valid() && !base_scene_gobject_remap.has(nid)) {
			//for nodes that _do_ exist in current scene, still try to look for
			//the node in the instantiated scene, as a property may be missing
			//from the local one
			// 加入一下
			int idx = get_base_scene_state()->find_gobject_by_path(p_node);
			if (idx != -1) {
				base_scene_gobject_remap[nid] = idx;
			}
		}

		return nid;
	}

	struct _FastPackState {
		SceneState* state = nullptr;
		int node = -1;
	};
	static bool _collect_inheritance_chain(const Ref<SceneState>& p_state, const GObjectPath& p_path, LocalVector<_FastPackState>& r_states_stack) {
		bool found = false;

		LocalVector<_FastPackState> inheritance_states;

		Ref<SceneState> state = p_state;
		while (state.is_valid()) {
			int node = state->find_gobject_by_path(p_path);
			if (node >= 0) {
				// This one has state for this node
				inheritance_states.push_back({ state.ptr(), node });
				found = true;
			}
			state = state->get_base_scene_state();
		}

		if (inheritance_states.size() > 0) {
			for (int i = inheritance_states.size() - 1; i >= 0; --i) {
				r_states_stack.push_back(inheritance_states[i]);
			}
		}

		return found;
	}

	Vector<SceneState::PackState> SceneState::_get_gobject_states_stack(const GObject* p_node, const GObject* p_owner, bool* r_instantiated_by_owner) {
		if (r_instantiated_by_owner) {
			*r_instantiated_by_owner = true;
		}

		LocalVector<_FastPackState> states_stack;
		{
			const GObject* owner = p_owner;
#ifdef TOOLS_ENABLED
			if (!p_owner && Engine::get_singleton()->is_editor_hint()) {
				owner = EditorGObject::get_singleton()->get_edited_scene();
			}
#endif

			const GObject* n = p_node;
			while (n) {
				if (n == owner) {
					const Ref<SceneState>& state = n->get_scene_inherited_state();
					if (_collect_inheritance_chain(state, n->get_path_to(p_node), states_stack)) {
						if (r_instantiated_by_owner) {
							*r_instantiated_by_owner = false;
						}
					}
					break;
				}
				else if (!n->get_scene_file_path().is_empty()) {
					const Ref<SceneState>& state = n->get_scene_instance_state();
					_collect_inheritance_chain(state, n->get_path_to(p_node), states_stack);
				}
				n = n->get_owner();
			}
		}
		// Convert to the proper type *for returning*, inverting the vector on the go
		// (it was more convenient to fill the vector in reverse order)
		Vector<SceneState::PackState> states_stack_ret;
		{
			states_stack_ret.resize(states_stack.size());
			_FastPackState* ps = states_stack.ptr();
			if (states_stack.size() > 0) {
				for (int i = states_stack.size() - 1; i >= 0; --i) {
					states_stack_ret.write[i].state.reference_ptr(ps->state);
					states_stack_ret.write[i].gobject = ps->node;
					++ps;
				}
			}
		}
		return states_stack_ret;
	}

}