#include "viewport.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/world_environment.h"
using namespace lain;

void ViewportTexture::setup_local_to_scene() {
  // For the same target viewport, setup is only allowed once to prevent multiple free or multiple creations.
  if (!vp_changed) {
    return;
  }

  if (vp_pending) {
    return;
  }

  GObject* loc_scene = get_local_scene();
  if (!loc_scene) {
    return;
  }

  if (vp) {
    vp->viewport_textures.erase(this);
    vp = nullptr;
  }

  if (loc_scene->is_ready()) {
    _setup_local_to_scene(loc_scene);
  } else {
    // loc_scene->connect(SceneStringName(ready), callable_mp(this, &ViewportTexture::_setup_local_to_scene).bind(loc_scene), CONNECT_ONE_SHOT);
    vp_pending = true;
  }
}
ViewportTexture::ViewportTexture() {
	set_local_to_scene(true);
}


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
Vector2 Viewport::get_camera_coords(const Vector2& p_viewport_coords) const {
  ERR_READ_THREAD_GUARD_V(Vector2());
  Transform2D xf = stretch_transform * global_canvas_transform;
  return xf.xform(p_viewport_coords);
}
Vector2 lain::Viewport::get_camera_rect_size() const {
  ERR_READ_THREAD_GUARD_V(Vector2());
  return size;
}

RID Viewport::get_viewport_rid() const {
  ERR_READ_THREAD_GUARD_V(RID());
  return viewport;
}

void lain::Viewport::_camera_3d_transform_changed_notify() {}

void lain::Viewport::_propagate_enter_world_3d(GObject* p_node) {
  if (p_node != this) {
    if (!p_node->is_inside_tree()) {  //may not have entered scene yet
      return;
    }

    if (Object::cast_to<GObject3D>(p_node) || Object::cast_to<WorldEnvironment>(p_node)) {
      p_node->notification(GObject3D::NOTIFICATION_ENTER_WORLD);
    } else {
      Viewport* v = Object::cast_to<Viewport>(p_node);
      if (v) {
        if (v->world_3d.is_valid() || v->own_world_3d.is_valid()) {
          return;
        }
      }
    }
  }

  for (int i = 0; i < p_node->get_child_count(); i++) {
    _propagate_enter_world_3d(p_node->get_child(i));
  }
}

void lain::Viewport::_propagate_exit_world_3d(GObject* p_node) {
  	if (p_node != this) {
		if (!p_node->is_inside_tree()) { //may have exited scene already
			return;
		}

		if (Object::cast_to<GObject3D>(p_node) || Object::cast_to<WorldEnvironment>(p_node)) {
			p_node->notification(GObject3D::NOTIFICATION_EXIT_WORLD);
		} else {
			Viewport *v = Object::cast_to<Viewport>(p_node);
			if (v) {
				if (v->world_3d.is_valid() || v->own_world_3d.is_valid()) {
					return;
				}
			}
		}
	}

	for (int i = 0; i < p_node->get_child_count(); i++) {
		_propagate_exit_world_3d(p_node->get_child(i));
	}
}

Transform2D lain::Viewport::get_final_transform() const
{
ERR_READ_THREAD_GUARD_V(Transform2D());
	return stretch_transform * global_canvas_transform;
}

void lain::Viewport::push_input(const Ref<InputEvent>& p_event, bool p_local_coords) {
  ERR_MAIN_THREAD_GUARD;
	ERR_FAIL_COND(!is_inside_tree());
	ERR_FAIL_COND(p_event.is_null());
  
}

void Viewport::_notification(int p_what) {
  ERR_MAIN_THREAD_GUARD;
  switch (p_what) {
    case NOTIFICATION_ENTER_TREE:
      // _update_viewport_path();
      {
        if (get_parent()) {
          parent = get_parent()->get_viewport();
          RS::get_singleton()->viewport_set_parent_viewport(viewport, parent->get_viewport_rid());
        } else {
          parent = nullptr;
        }
        RS::get_singleton()->viewport_set_scenario(viewport, find_world_3d()->get_scenario());
        add_to_group("_viewports");
				RS::get_singleton()->viewport_set_active(get_viewport_rid(), true);

      }
      break;
    case NOTIFICATION_READY: {
      if (camera_3d_set.size() && !camera_3d) {
        // There are cameras but no current camera, pick first in tree and make it current.
        Camera3D* first = nullptr;
        for (Camera3D* E : camera_3d_set) {
          if (first == nullptr || first->is_greater_than(E)) {
            first = E;
          }
        }

        if (first) {
          first->make_current();
        }
      }
    }
		break;
		case NOTIFICATION_EXIT_TREE:{
			RS::get_singleton()->viewport_set_scenario(viewport, RID());
			RS::get_singleton()->viewport_set_active(viewport, false);

		}
    default:
      break;
  }
}

Viewport::Viewport() {
	Rect2 attach_to_screen_rect = Rect2(Point2(), WindowSystem::GetSingleton()->window_get_size());
  viewport = RS::get_singleton()->viewport_create();
  texture_rid = RS::get_singleton()->viewport_get_texture(viewport);
	default_texture.instantiate();
	default_texture->vp = const_cast<Viewport *>(this);
	viewport_textures.insert(default_texture.ptr());
	set_positional_shadow_atlas_size(positional_shadow_atlas_size);

	RS::get_singleton()->viewport_set_update_mode(get_viewport_rid(), RS::VIEWPORT_UPDATE_WHEN_VISIBLE);
	RS::get_singleton()->viewport_attach_to_screen(get_viewport_rid(), attach_to_screen_rect, WindowSystem::MAIN_WINDOW_ID);
	// sub viewport
	RS::get_singleton()->viewport_set_size(get_viewport_rid(), WindowSystem::GetSingleton()->window_get_size().width(), WindowSystem::GetSingleton()->window_get_size().height());

  String id = itos(get_instance_id());
	input_group = "_vp_input" + id;

}

Ref<World3D> Viewport::get_world_3d() const {
  ERR_READ_THREAD_GUARD_V(Ref<World3D>());
  return world_3d;
}

void Viewport::set_world_3d(const Ref<World3D>& p_world_3d) {
  ERR_MAIN_THREAD_GUARD;
  if (world_3d == p_world_3d) {
    return;
  }

  if (is_inside_tree()) {
    _propagate_exit_world_3d(this);
  }

  if (own_world_3d.is_valid() && world_3d.is_valid()) {
    // world_3d->disconnect_changed(callable_mp(this, &Viewport::_own_world_3d_changed));
  }

  world_3d = p_world_3d;

  if (own_world_3d.is_valid()) {
    if (world_3d.is_valid()) {
      own_world_3d = world_3d->duplicate();
      // world_3d->connect_changed(callable_mp(this, &Viewport::_own_world_3d_changed));
    } else {
      own_world_3d = Ref<World3D>(memnew(World3D));
    }
  }

  if (is_inside_tree()) {
    _propagate_enter_world_3d(this);
  }

  if (is_inside_tree()) {
    RS::get_singleton()->viewport_set_scenario(viewport, find_world_3d()->get_scenario());
  }

  // _update_audio_listener_3d();
}

void Viewport::_camera_3d_set(Camera3D* p_camera) {
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

bool Viewport::_camera_3d_add(Camera3D* p_camera) {
  camera_3d_set.insert(p_camera);
  return camera_3d_set.size() == 1;
}

void Viewport::_camera_3d_remove(Camera3D* p_camera) {
  bool succ = camera_3d_set.erase(p_camera);

  if (camera_3d == p_camera) {
    _camera_3d_set(nullptr);
  }
}
void Viewport::_camera_3d_make_next_current(Camera3D* p_exclude) {
  for (Camera3D* E : camera_3d_set) {
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
Camera3D* Viewport::get_camera_3d() const {
  ERR_READ_THREAD_GUARD_V(nullptr);
  return camera_3d;
}

lain::ViewportTexture::~ViewportTexture() {
  if (vp) {
    vp->viewport_textures.erase(this);
  }

  ERR_FAIL_NULL(RS::get_singleton());

  if (proxy_ph.is_valid()) {
    RS::get_singleton()->free(proxy_ph);
  }
  if (proxy.is_valid()) {
    RS::get_singleton()->free(proxy);
  }
}

void ViewportTexture::reset_local_to_scene() {
  vp_changed = true;

  if (vp) {
    vp->viewport_textures.erase(this);
    vp = nullptr;
  }

  // if (proxy.is_valid() && proxy_ph.is_null()) {
  // 	proxy_ph = RS::get_singleton()->texture_2d_placeholder_create();
  // 	RS::get_singleton()->texture_proxy_update(proxy, proxy_ph);
  // }
}

void ViewportTexture::set_viewport_path_in_scene(const GObjectPath& p_path) {
  if (path == p_path) {
    return;
  }

  path = p_path;

  reset_local_to_scene();

  if (get_local_scene() && !path.is_empty()) {
    setup_local_to_scene();
  } else {
    emit_changed();
  }
}

GObjectPath ViewportTexture::get_viewport_path_in_scene() const {
  return path;
}

int ViewportTexture::get_width() const {
  if (!vp) {
    if (!vp_pending) {
      ERR_PRINT("Viewport Texture must be set to use it.");
    }
    return 0;
  }
  return vp->size.width();
}

int ViewportTexture::get_height() const {
  if (!vp) {
    if (!vp_pending) {
      ERR_PRINT("Viewport Texture must be set to use it.");
    }
    return 0;
  }
  return vp->size.height();
}

Size2 ViewportTexture::get_size() const {
  if (!vp) {
    if (!vp_pending) {
      ERR_PRINT("Viewport Texture must be set to use it.");
    }
    return Size2();
  }
  return vp->size;
}

RID ViewportTexture::GetRID() const {
  // if (proxy.is_null()) {
  // 	proxy_ph = RS::get_singleton()->texture_2d_placeholder_create();
  // 	proxy = RS::get_singleton()->texture_proxy_create(proxy_ph);
  // }
  return proxy;
}

bool ViewportTexture::has_alpha() const {
  return false;
}

Ref<Image> ViewportTexture::get_image() const {
  if (!vp) {
    if (!vp_pending) {
      ERR_PRINT("Viewport Texture must be set to use it.");
    }
    return Ref<Image>();
  }
  return RS::get_singleton()->texture_2d_get(vp->texture_rid);
}

void ViewportTexture::_setup_local_to_scene(const GObject* p_loc_scene) {
  // Always reset this, even if this call fails with an error.
  vp_pending = false;

  GObject* vpn = p_loc_scene->get_gobject_or_null(path);
  ERR_FAIL_NULL_MSG(vpn, "Path to node is invalid: '" + path + "'.");
  vp = Object::cast_to<Viewport>(vpn);
  ERR_FAIL_NULL_MSG(vp, "Path to node does not point to a viewport: '" + path + "'.");

  vp->viewport_textures.insert(this);

  ERR_FAIL_NULL(RS::get_singleton());
  if (proxy_ph.is_valid()) {
    // RS::get_singleton()->texture_proxy_update(proxy, vp->texture_rid);
    RS::get_singleton()->free(proxy_ph);
    proxy_ph = RID();
  } else {
    ERR_FAIL_COND(proxy.is_valid());  // Should be invalid.
                                      // proxy = RS::get_singleton()->texture_proxy_create(vp->texture_rid);
  }
  vp_changed = false;

  emit_changed();
}

Viewport::~Viewport() {
  // Erase itself from viewport textures.
  for (ViewportTexture* E : viewport_textures) {
    E->vp = nullptr;
  }
  ERR_FAIL_NULL(RS::get_singleton());
  RS::get_singleton()->free(viewport);
}

void Viewport::set_positional_shadow_atlas_size(int p_size) {
	ERR_MAIN_THREAD_GUARD;
	positional_shadow_atlas_size = p_size;
	// RS::get_singleton()->viewport_set_positional_shadow_atlas_size(viewport, p_size, positional_shadow_atlas_16_bits);
}


Ref<InputEvent> Viewport::_make_input_local(const Ref<InputEvent> &ev) {
	if (ev.is_null()) {
		return ev; // No transformation defined for null event
	}

	Transform2D ai = get_final_transform().affine_inverse();
	Ref<InputEventMouse> me = ev;
	if (me.is_valid()) {
		me = me->xformed_by(ai);
		// For InputEventMouse, the global position is not adjusted by ev->xformed_by() and needs to be set separately.
		me->set_global_position(me->get_position());
		return me;
	}
	return ev->xformed_by(ai);
}