#include "scene_tree.h"
#include "core/scene/object/gobject.h"
#include "core/scene/packed_scene.h"
namespace lain {
	SceneTree* SceneTree::singleton = nullptr;
	SceneTree::SceneTree() {
		if (singleton == nullptr) {
			singleton = this;
		}
		current_scene = nullptr;
		root = memnew(GObject);
		// 在这里设置屏幕相关数据
	}
	SceneTree::~SceneTree() {
		singleton = nullptr;
	}
	void SceneTree::initialize() {
		ERR_FAIL_NULL(root);
		MainLoop::initialize();
		root->_set_tree(this);
	}

	SceneTree::Group* SceneTree::add_to_group(const StringName& p_group, GObject* p_node) {
		_THREAD_SAFE_METHOD_

			HashMap<StringName, Group>::Iterator E = group_map.find(p_group);
		if (!E) {
			E = group_map.insert(p_group, Group());
		}

		ERR_FAIL_COND_V_MSG(E->value.nodes.has(p_node), &E->value, "Already in group: " + p_group + ".");
		E->value.nodes.push_back(p_node);
		//E->value.last_tree_version=0;
		E->value.changed = true;
		return &E->value;
	}
	void SceneTree::finalize() {
		if (root) {
			root->_set_tree(nullptr);
			root->_propagate_after_exit_tree();
			memdelete(root); //delete root
			root = nullptr;

			// In case deletion of some objects was queued when destructing the `root`.
			// E.g. if `queue_free()` was called for some node outside the tree when handling NOTIFICATION_PREDELETE for some node in the tree.
			//_flush_delete_queue();
		}
		MainLoop::finalize();

	}


}