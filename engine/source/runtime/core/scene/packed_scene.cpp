#include "packed_scene.h"
#include "core/io/resource_loader.h"
namespace lain {
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

		//s->notification(Node::NOTIFICATION_SCENE_INSTANTIATED);

		return s;
	}



	bool SceneState::can_instantiate() const {
		return gobjects.size() > 0;
	}
	int SceneState::get_gobject_count() const {
		return gobjects.size();
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

	GObject* SceneState::instantiate(GenEditState p_edit_state) const {

#define GOBJ_FROM_ID(p_name, p_id)                      \
	GObject *p_name;                                       \
	if (p_id & FLAG_ID_IS_PATH) {                       \
		GObjectPath np = gobject_paths[p_id & FLAG_MASK];     \
		p_name = ret_nodes[0]->get_gobject_or_null(np);    \
	} else {                                            \
		ERR_FAIL_INDEX_V(p_id &FLAG_MASK, nc, nullptr); \
		p_name = ret_nodes[p_id & FLAG_MASK];           \
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
		GObject** ret_nodes = (GObject**)memnew_arr(GObject*, nc); // 为什么要在栈上分配一些指针的空间？
		bool gen_node_path_cache = p_edit_state != GEN_EDIT_STATE_DISABLED && gobject_path_cache.is_empty(); // 生成
		// Nodes where instantiation failed (because something is missing.)
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
			else if(n.instance > 0){  // node data中有一些实例化
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
							missing_node = memnew(MissingNode);
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
					Ref<Resource> res = props == nullptr? props[n.instance & FLAG_MASK] : (L_CORE_ERROR("Scene instance is missing."), nullptr);
					Ref<PackedScene> sdata = res;
					if (sdata.is_valid()) {
						gobj = sdata->instantiate(p_edit_state == GEN_EDIT_STATE_DISABLED ? PackedScene::GEN_EDIT_STATE_DISABLED : PackedScene::GEN_EDIT_STATE_INSTANCE);
						ERR_FAIL_NULL_V_MSG(gobj, nullptr, vformat("Failed to load scene dependency: \"%s\". Make sure the required scene is valid.", sdata->GetPath()));
					}
					/*else if (ResourceLoader::is_creating_missing_resources_if_class_unavailable_enabled()) {
						missing_node = memnew(MissingNode);
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
						WARN_PRINT(String("Node '" + String(ret_nodes[0]->get_path_to(parent)) + "/" + String(snames[n.name]) + "' was modified from inside an instance, but it has vanished.").ascii().get_data());
					}
#endif
				}
			}
			else {
				// Node belongs to this scene and must be created.
				Object* obj = ClassDB::instantiate(snames[n.type]);

				gobj = Object::cast_to<GObject>(obj);

				if (!gobj) {
					if (obj) {
						memdelete(obj);
						obj = nullptr;
					}
					// @TODO: missing
					/*if (ResourceLoader::is_creating_missing_resources_if_class_unavailable_enabled()) {
						missing_node = memnew(MissingNode);
						missing_node->set_original_class(snames[n.type]);
						missing_node->set_recording_properties(true);
						node = missing_node;
						obj = missing_node;
					}*/
					else {
						WARN_PRINT(vformat("Node %s of type %s cannot be created. A placeholder will be created instead.", snames[n.name], snames[n.type]).ascii().get_data());
//						if (n.parent >= 0 && n.parent < nc && ret_nodes[n.parent]) {
//							if (Object::cast_to<Control>(ret_nodes[n.parent])) {
//								obj = memnew(Control);
//							}
//							else if (Object::cast_to<Node2D>(ret_nodes[n.parent])) {
//								obj = memnew(Node2D);
//#ifndef _3D_DISABLED
//							}
//							else if (Object::cast_to<Node3D>(ret_nodes[n.parent])) {
//								obj = memnew(Node3D);
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


			}

		}

	}

}