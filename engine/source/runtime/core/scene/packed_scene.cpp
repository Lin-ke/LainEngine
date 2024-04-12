#include "packed_scene.h"
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
}