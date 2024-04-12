#include "scene_tree.h"
namespace lain {
	SceneTree* SceneTree::singleton = nullptr;
	SceneTree::SceneTree() {
		if (singleton == nullptr) {
			singleton = this;
		}
	}
	SceneTree::~SceneTree() {
		singleton = nullptr;
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


}