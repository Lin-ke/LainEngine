#include "node_3d.h"
#include "scene/main/viewport.h"
#include "core/engine/engine.h"
#include "scene/3d/visual_instance_3d.h"
#include "node_3d_data.h"
using namespace lain;


Ref<World3D> GObject3D::get_world_3d() const {
	ERR_READ_THREAD_GUARD_V(Ref<World3D>()); // World3D can only be set from main thread, so it's safe to obtain on threads.
	ERR_FAIL_COND_V(!is_inside_world(), Ref<World3D>());
	ERR_FAIL_NULL_V(data.viewport, Ref<World3D>());

	return data.viewport->find_world_3d();
}

void lain::GObject3D::set_position(const Vector3& p_position) {
		ERR_THREAD_GUARD;
	data.local_transform.origin = p_position;
	_propagate_transform_changed(this);
	if (data.notify_local_transform) {
		notification(NOTIFICATION_LOCAL_TRANSFORM_CHANGED);
	}
}

void lain::GObject3D::set_notify_transform(bool p_enabled) {
		ERR_THREAD_GUARD;
	data.notify_transform = p_enabled;
}
void GObject3D::set_disable_scale(bool p_enabled) {
	ERR_THREAD_GUARD;
	data.disable_scale = p_enabled;
}

void GObject3D::_notification(int p_what){
	switch (p_what) {
		// enter tree 那么肯定就enter world 啦
		case NOTIFICATION_ENTER_TREE: {
			ERR_MAIN_THREAD_GUARD;
			ERR_FAIL_NULL(get_tree());

			GObject *p = get_parent();
			if (p) {
				data.parent = Object::cast_to<GObject3D>(p);
			}

			if (data.parent) {
				data.C = data.parent->data.children.push_back(this);
			} else {
				data.C = nullptr;
			}

			if (data.top_level && !Engine::GetSingleton()->is_editor_hint()) {
				if (data.parent) {
					if (!data.top_level) {
						data.local_transform = data.parent->get_global_transform() * get_transform();
					} else {
						data.local_transform = get_transform();
					}
					_replace_dirty_mask(DIRTY_EULER_ROTATION_AND_SCALE); // As local transform was updated, rot/scale should be dirty.
				}
			}

			_set_dirty_bits(DIRTY_GLOBAL_TRANSFORM); // Global is always dirty upon entering a scene.
			_notify_dirty();

			notification(NOTIFICATION_ENTER_WORLD);
			_update_visibility_parent(true);
		} break;

		case NOTIFICATION_EXIT_TREE: {
			ERR_MAIN_THREAD_GUARD;

			notification(NOTIFICATION_EXIT_WORLD, true);
			if (xform_change.in_list()) {
				get_tree()->xform_change_list.remove(&xform_change);
			}
			if (data.C) {
				data.parent->data.children.erase(data.C);
			}
			data.parent = nullptr;
			data.C = nullptr;
			_update_visibility_parent(true);
		} break;

		case NOTIFICATION_ENTER_WORLD: {
			ERR_MAIN_THREAD_GUARD;

			data.inside_world = true;
			data.viewport = nullptr;
			GObject *parent = get_parent();
			while (parent && !data.viewport) {
				data.viewport = Object::cast_to<Viewport>(parent);
				parent = parent->get_parent();
			}

			ERR_FAIL_NULL(data.viewport);

			// if (get_script_instance()) {
			// 	get_script_instance()->call(SNAME("_enter_world"));
			// }

#ifdef TOOLS_ENABLED
			if (is_part_of_edited_scene()) {
				get_tree()->call_group_flags(SceneTree::GROUP_CALL_DEFERRED, SceneStringName(_spatial_editor_group), SNAME("_request_gizmo_for_id"), get_instance_id());
			}
#endif
		} break;

		case NOTIFICATION_EXIT_WORLD: {
			ERR_MAIN_THREAD_GUARD;

#ifdef TOOLS_ENABLED
			clear_gizmos();
#endif

			// if (get_script_instance()) {
			// 	get_script_instance()->call(SNAME("_exit_world"));
			// }

			data.viewport = nullptr;
			data.inside_world = false;
		} break;

		case NOTIFICATION_TRANSFORM_CHANGED: {
			ERR_THREAD_GUARD;

#ifdef TOOLS_ENABLED
			for (int i = 0; i < data.gizmos.size(); i++) {
				data.gizmos.write[i]->transform();
			}
#endif
		} break;
	}
}
void GObject3D::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_transform", "local"), &GObject3D::set_transform);
	ClassDB::bind_method(D_METHOD("get_transform"), &GObject3D::get_transform);
	ClassDB::bind_method(D_METHOD("set_position", "position"), &GObject3D::set_position);
	ClassDB::bind_method(D_METHOD("get_position"), &GObject3D::get_position);
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM3D, "transform", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NO_EDITOR), "set_transform", "get_transform");

}
// 一路乘上来即可
Transform3D lain::GObject3D::get_global_transform() const {
ERR_FAIL_COND_V(!is_inside_tree(), Transform3D());

	/* Due to how threads work at scene level, while this global transform won't be able to be changed from outside a thread,
	 * it is possible that multiple threads can access it while it's dirty from previous work. Due to this, we must ensure that
	 * the dirty/update process is thread safe by utilizing atomic copies.
	 */

	uint32_t dirty = _read_dirty_mask();
	if (dirty & DIRTY_GLOBAL_TRANSFORM) {
		if (dirty & DIRTY_LOCAL_TRANSFORM) {
			_update_local_transform(); // Update local transform atomically.
		}

		Transform3D new_global;
		if (data.parent && !data.top_level) {
			new_global = data.parent->get_global_transform() * data.local_transform;
		} else {
			new_global = data.local_transform;
		}

		if (data.disable_scale) {
			new_global.basis.orthonormalize();
		}

		data.global_transform = new_global;
		_clear_dirty_bits(DIRTY_GLOBAL_TRANSFORM);
	}

	return data.global_transform;
}

void GObject3D::reparent(GObject *p_parent, bool p_keep_global_transform) {
	ERR_THREAD_GUARD;
	if (p_keep_global_transform) {
		Transform3D temp = get_global_transform();
		GObject::reparent(p_parent);
		set_global_transform(temp);
	} else {
		GObject::reparent(p_parent);
	}
}

GObject3D::GObject3D() : 
xform_change(this)
{}

void GObject3D::_from_data(void* p_data) {
	GObject3DData* data = (GObject3DData*)p_data;
	if(data->is_transform_used)
		set_transform(data->transform);
}

Transform3D GObject3D::get_transform() const {
	ERR_READ_THREAD_GUARD_V(Transform3D());
	if (_test_dirty_bits(DIRTY_LOCAL_TRANSFORM)) {
		// This update can happen if needed over multiple threads.
		_update_local_transform();
	}

	return data.local_transform;
}

void GObject3D::_update_local_transform() const {
	// This function is called when the local transform (data.local_transform) is dirty and the right value is contained in the Euler rotation and scale.
	data.local_transform.basis.set_euler_scale(data.euler_rotation, data.scale, data.euler_rotation_order);
	_clear_dirty_bits(DIRTY_LOCAL_TRANSFORM);
}
void GObject3D::_notify_dirty() {
#ifdef TOOLS_ENABLED
	if ((!data.gizmos.is_empty() || data.notify_transform) && !data.ignore_notification && !xform_change.in_list()) {
#else
	if (data.notify_transform && !data.ignore_notification && !xform_change.in_list()) {

#endif
		get_tree()->xform_change_list.add(&xform_change);
	}
}

void GObject3D::_clear_dirty_bits(uint32_t p_bits) const {
	if (is_group_processing()) {
		data.dirty.mt.bit_and(~p_bits);
	} else {
		data.dirty.st &= ~p_bits;
	}
}
void GObject3D::_set_dirty_bits(uint32_t p_bits) const {
	if (is_group_processing()) {
		data.dirty.mt.bit_or(p_bits);
	} else {
		data.dirty.st |= p_bits;
	}
}

void GObject3D::_replace_dirty_mask(uint32_t p_mask) const {
	if (is_group_processing()) {
		data.dirty.mt.set(p_mask);
	} else {
		data.dirty.st = p_mask;
	}
}
void GObject3D::_update_visibility_parent(bool p_update_root) {
	RID new_parent;

	if (!visibility_parent_path.is_empty()) {
		if (!p_update_root) {
			return;
		}
		GObject *parent = get_gobject_or_null(visibility_parent_path);
		ERR_FAIL_NULL_MSG(parent, "Can't find visibility parent node at path: " + visibility_parent_path);
		ERR_FAIL_COND_MSG(parent == this, "The visibility parent can't be the same node.");
		GeometryInstance3D *gi = Object::cast_to<GeometryInstance3D>(parent);
		ERR_FAIL_NULL_MSG(gi, "The visibility parent node must be a GeometryInstance3D, at path: " + visibility_parent_path);
		new_parent = gi ? gi->get_instance() : RID();
	} else if (data.parent) {
		new_parent = data.parent->data.visibility_parent;
	}

	if (new_parent == data.visibility_parent) {
		return;
	}

	data.visibility_parent = new_parent;

	VisualInstance3D *vi = Object::cast_to<VisualInstance3D>(this);
	if (vi) {
		RS::get_singleton()->instance_set_visibility_parent(vi->get_instance(), data.visibility_parent);
	}

	for (GObject3D *c : data.children) {
		c->_update_visibility_parent(false);
	}
}

void GObject3D::set_global_transform(const Transform3D &p_transform) {
	ERR_THREAD_GUARD;
	Transform3D xform = (data.parent && !data.top_level)
			? data.parent->get_global_transform().affine_inverse() * p_transform // 将 transform 变换到 parent的local坐标系
			: p_transform;

	set_transform(xform);
}
void GObject3D::set_transform(const Transform3D &p_transform) {
	ERR_THREAD_GUARD;
	data.local_transform = p_transform;
	_replace_dirty_mask(DIRTY_EULER_ROTATION_AND_SCALE); // Make rot/scale dirty.

	_propagate_transform_changed(this);
	if (data.notify_local_transform) {
		notification(NOTIFICATION_LOCAL_TRANSFORM_CHANGED);
	}
}


void GObject3D::_propagate_transform_changed(GObject3D *p_origin) {
	if (!is_inside_tree()) {
		return;
	}

	for (GObject3D *&E : data.children) {
		if (E->data.top_level) {
			continue; //don't propagate to a top_level
		}
		E->_propagate_transform_changed(p_origin);
	}
#ifdef TOOLS_ENABLED
	if ((!data.gizmos.is_empty() || data.notify_transform) && !data.ignore_notification && !xform_change.in_list()) {
#else
	if (data.notify_transform && !data.ignore_notification && !xform_change.in_list()) {
#endif
		if (likely(is_accessible_from_caller_thread())) {
			get_tree()->xform_change_list.add(&xform_change);
		} else {
			// This should very rarely happen, but if it does at least make sure the notification is received eventually.
			// callable_mp(this, &Node3D::_propagate_transform_changed_deferred).call_deferred();
		}
	}
	_set_dirty_bits(DIRTY_GLOBAL_TRANSFORM);
}

Vector3 lain::GObject3D::get_position() const {
	ERR_READ_THREAD_GUARD_V(Vector3());
	return data.local_transform.origin;
}

bool GObject3D::is_visible_in_tree() const {
	ERR_READ_THREAD_GUARD_V(false); // Since visibility can only be changed from main thread, this is safe to call.
  const GObject3D *n = this;
	while (n) {
		if (!n->data.visible) {
			return false;
		}
		n = n->data.parent;
	}
	return true;
}
