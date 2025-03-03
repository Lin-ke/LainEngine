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
			L_PRINT("visual instance inter world " + get_name());
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

AABB lain::VisualInstance3D::get_aabb() const {
  return AABB();
}
void VisualInstance3D::set_base(const RID &p_base) {
	RS::get_singleton()->instance_set_base(instance, p_base);
	base = p_base;
}
RID VisualInstance3D::get_base() const {
	return base;
}

void lain::VisualInstance3D::set_layer_mask(uint32_t p_mask) {
		layers = p_mask;
	RS::get_singleton()->instance_set_layer_mask(instance, p_mask);
}

uint32_t lain::VisualInstance3D::get_layer_mask() const {
  return layers;
}
// visual instance ID (RID) <-> object ID <-> 实际的instance ID ，如灯的ID

lain::VisualInstance3D::VisualInstance3D() {
		instance = RS::get_singleton()->instance_create();
	RS::get_singleton()->instance_attach_object_instance_id(instance, get_instance_id());
	set_notify_transform(true);
}

lain::VisualInstance3D::~VisualInstance3D() {
	ERR_FAIL_NULL(RS::get_singleton());
	RS::get_singleton()->free(instance);
}

void lain::GeometryInstance3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_material_override", "material"), &GeometryInstance3D::set_material_override);
	ClassDB::bind_method(D_METHOD("get_material_override"), &GeometryInstance3D::get_material_override);


	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material_override", PROPERTY_HINT_RESOURCE_TYPE, "BaseMaterial3D,ShaderMaterial", PROPERTY_USAGE_DEFAULT), "set_material_override", "get_material_override");

}

void GeometryInstance3D::set_material_override(const Ref<Material> &p_material) {
	if (material_override.is_valid()) {
		// material_override->disconnect(CoreStringName(property_list_changed), callable_mp((Object *)this, &Object::notify_property_list_changed));
	}
	material_override = p_material;
	if (material_override.is_valid()) {
		// material_override->connect(CoreStringName(property_list_changed), callable_mp((Object *)this, &Object::notify_property_list_changed));
	}
	RS::get_singleton()->instance_geometry_set_material_override(get_instance(), p_material.is_valid() ? p_material->GetRID() : RID());
}

Ref<Material> GeometryInstance3D::get_material_override() const {
	return material_override;
}

const StringName *GeometryInstance3D::_instance_uniform_get_remap(const StringName &p_name) const {
	StringName *r = instance_shader_parameter_property_remap.getptr(p_name);
	if (!r) {
		String s = p_name;
		if (s.begins_with("instance_shader_parameters/")) {
			StringName pname = StringName(s);
			StringName name = s.replace("instance_shader_parameters/", "");
			instance_shader_parameter_property_remap[pname] = name;
			return instance_shader_parameter_property_remap.getptr(pname);
		}
		return nullptr;
	}

	return r;
}

bool lain::GeometryInstance3D::_set(const StringName& p_name, const Variant& p_value) {
const StringName *r = _instance_uniform_get_remap(p_name);
	if (r) {
		set_instance_shader_parameter(*r, p_value);
		return true;
	}
	return false;
}


bool GeometryInstance3D::_get(const StringName &p_name, Variant &r_ret) const {
	const StringName *r = _instance_uniform_get_remap(p_name);
	if (r) {
		r_ret = get_instance_shader_parameter(*r);
		return true;
	}

	return false;
}



void GeometryInstance3D::_get_property_list(List<PropertyInfo>* p_list) const {
  List<PropertyInfo> pinfo;
  RS::get_singleton()->instance_geometry_get_shader_parameter_list(get_instance(), &pinfo);
  for (PropertyInfo &pi : pinfo) {
  	bool has_def_value = false;
  	Variant def_value = RS::get_singleton()->instance_geometry_get_shader_parameter_default_value(get_instance(), pi.name);
  	if (def_value.get_type() != Variant::NIL) {
  		has_def_value = true;
  	}
  	if (instance_shader_parameters.has(pi.name)) {
  		pi.usage = PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_STORAGE | (has_def_value ? (PROPERTY_USAGE_CHECKABLE | PROPERTY_USAGE_CHECKED) : PROPERTY_USAGE_NONE);
  	} else {
  		pi.usage = PROPERTY_USAGE_EDITOR | (has_def_value ? PROPERTY_USAGE_CHECKABLE : PROPERTY_USAGE_NONE); //do not save if not changed
  	}

  	pi.name = "instance_shader_parameters/" + pi.name;
  	p_list->push_back(pi);
  }
}
GeometryInstance3D::GeometryInstance3D() {
}

GeometryInstance3D::~GeometryInstance3D() {
	if (material_overlay.is_valid()) {
		set_material_overlay(Ref<Material>());
	}
	if (material_override.is_valid()) {
		set_material_override(Ref<Material>());
	}
}

void GeometryInstance3D::set_material_overlay(const Ref<Material> &p_material) {
	material_overlay = p_material;
	RS::get_singleton()->instance_geometry_set_material_overlay(get_instance(), p_material.is_valid() ? p_material->GetRID() : RID());
}

GeometryInstance3D::GIMode lain::GeometryInstance3D::get_gi_mode() const {
  return gi_mode;
}

void lain::GeometryInstance3D::set_instance_shader_parameter(const StringName& p_name, const Variant& p_value) {
  if (p_value.get_type() == Variant::NIL) {
    Variant def_value = RS::get_singleton()->instance_geometry_get_shader_parameter_default_value(get_instance(), p_name);
    RS::get_singleton()->instance_geometry_set_shader_parameter(get_instance(), p_name, def_value);
    instance_shader_parameters.erase(p_value);
  } else {
    instance_shader_parameters[p_name] = p_value;
    if (p_value.get_type() == Variant::OBJECT) {
      RID tex_id = p_value;
      RS::get_singleton()->instance_geometry_set_shader_parameter(get_instance(), p_name, tex_id);
    } else {
      RS::get_singleton()->instance_geometry_set_shader_parameter(get_instance(), p_name, p_value);
    }
  }
}

Variant GeometryInstance3D::get_instance_shader_parameter(const StringName &p_name) const {
	return RS::get_singleton()->instance_geometry_get_shader_parameter(get_instance(), p_name);
}