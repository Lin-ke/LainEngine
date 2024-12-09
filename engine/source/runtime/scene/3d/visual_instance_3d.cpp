#include "visual_instance_3d.h"
using namespace lain;

void lain::VisualInstance3D::_update_visibility() {
  if (!is_inside_tree()) {
    return;
  }

  RS::get_singleton()->instance_set_visible(get_instance(), is_visible_in_tree());
}

void VisualInstance3D::_notification(int p_what) {
  switch (p_what) {
    case NOTIFICATION_ENTER_WORLD: {
      ERR_FAIL_COND(get_world_3d().is_null());
      RS::get_singleton()->instance_set_scenario(instance, get_world_3d()->get_scenario());
      _update_visibility();
    } break;

    case NOTIFICATION_TRANSFORM_CHANGED: {
      Transform3D gt = get_global_transform();
      RS::get_singleton()->instance_set_transform(instance, gt);
    } break;

    case NOTIFICATION_EXIT_WORLD: {
      RS::get_singleton()->instance_set_scenario(instance, RID());
      // RenderingServer::get_singleton()->instance_attach_skeleton(instance, RID());
    } break;

    case NOTIFICATION_VISIBILITY_CHANGED: {
      _update_visibility();
    } break;
  }
}

void lain::VisualInstance3D::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_layer_mask", "mask"), &VisualInstance3D::set_layer_mask);
  ClassDB::bind_method(D_METHOD("get_layer_mask"), &VisualInstance3D::get_layer_mask);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "layers", PROPERTY_HINT_LAYERS_3D_RENDER), "set_layer_mask", "get_layer_mask");
}

RID VisualInstance3D::get_instance() const {
  return instance;
}