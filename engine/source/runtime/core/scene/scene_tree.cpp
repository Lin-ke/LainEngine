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


}