#include "viewport.h"
#include "scene/3d/camera_3d.h"
using namespace lain;

Rect2 lain::Viewport::get_visible_rect() const {
	ERR_READ_THREAD_GUARD_V(Rect2());
	Rect2 r;

	if (size == Size2()) {
		r = Rect2(Point2(), WindowSystem::GetSingleton()->window_get_size());
	} else {
		r = Rect2(Point2(), size);
	}

	if (size_2d_override != Size2i()) {
		r.size = size_2d_override;
	}

	return r;
}
Vector2 Viewport::get_camera_coords(const Vector2 &p_viewport_coords) const {
	ERR_READ_THREAD_GUARD_V(Vector2());
	Transform2D xf = stretch_transform * global_canvas_transform;
	return xf.xform(p_viewport_coords);
}
Vector2 lain::Viewport::get_camera_rect_size() const {
	ERR_READ_THREAD_GUARD_V(Vector2());
	return size;
}

void lain::Viewport::_camera_3d_transform_changed_notify() {}

Ref<World3D> Viewport::get_world_3d() const {
  ERR_READ_THREAD_GUARD_V(Ref<World3D>());
  return world_3d;
}



void Viewport::_camera_3d_set(Camera3D *p_camera) {
	if (camera_3d == p_camera) {
		return;
	}

	if (camera_3d) {
		camera_3d->notification(Camera3D::NOTIFICATION_LOST_CURRENT);
	}

	camera_3d = p_camera;

	if (!camera_3d_override) {
		if (camera_3d) {
			RS::get_singleton()->viewport_attach_camera(viewport, camera_3d->get_camera());
		} else {
			RS::get_singleton()->viewport_attach_camera(viewport, RID());
		}
	}

	if (camera_3d) {
		camera_3d->notification(Camera3D::NOTIFICATION_BECAME_CURRENT);
	}

	// _update_audio_listener_3d();
	_camera_3d_transform_changed_notify();
}

bool Viewport::_camera_3d_add(Camera3D *p_camera) {
	camera_3d_set.insert(p_camera);
	return camera_3d_set.size() == 1;
}

void Viewport::_camera_3d_remove(Camera3D *p_camera) {
	bool succ = camera_3d_set.erase(p_camera);
	
	if (camera_3d == p_camera) {
		_camera_3d_set(nullptr);
	}
}
void Viewport::_camera_3d_make_next_current(Camera3D *p_exclude) {
	for (Camera3D *E : camera_3d_set) {
		if (p_exclude == E) {
			continue;
		}
		if (!E->is_inside_tree()) {
			continue;
		}
		if (camera_3d != nullptr) {
			return;
		}

		E->make_current();
	}
}



Ref<World3D> Viewport::find_world_3d() const {
	ERR_READ_THREAD_GUARD_V(Ref<World3D>());
	if (own_world_3d.is_valid()) {
		return own_world_3d;
	} else if (world_3d.is_valid()) {
		return world_3d;
	} else if (parent) {
		return parent->find_world_3d();
	} else {
		return Ref<World3D>();
	}
}
Camera3D *Viewport::get_camera_3d() const {
	ERR_READ_THREAD_GUARD_V(nullptr);
	return camera_3d;
}