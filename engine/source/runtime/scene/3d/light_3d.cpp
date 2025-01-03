#include "light_3d.h"
using namespace lain;
lain::Light3D::Light3D(RS::LightType p_type) {
  type = p_type;
  switch (p_type) {
    case RS::LIGHT_DIRECTIONAL:
      light = RS::get_singleton()->directional_light_create();
      break;
    case RS::LIGHT_OMNI:
      light = RS::get_singleton()->omni_light_create();
      break;
    case RS::LIGHT_SPOT:
      light = RS::get_singleton()->spot_light_create();
      break;
    default: {
    };
  }

  RS::get_singleton()->instance_set_base(get_instance(), light);

  set_color(Color(1, 1, 1, 1));
  set_shadow(false);
  // set_negative(false);
  // set_cull_mask(0xFFFFFFFF);

  set_param(PARAM_ENERGY, 1);
  set_param(PARAM_INDIRECT_ENERGY, 1);
  set_param(PARAM_VOLUMETRIC_FOG_ENERGY, 1);
  set_param(PARAM_SPECULAR, 0.5);
  set_param(PARAM_RANGE, 5);
  set_param(PARAM_SIZE, 0);
  set_param(PARAM_ATTENUATION, 1);
  set_param(PARAM_SPOT_ANGLE, 45);
  set_param(PARAM_SPOT_ATTENUATION, 1);
  set_param(PARAM_SHADOW_MAX_DISTANCE, 0);
  set_param(PARAM_SHADOW_SPLIT_1_OFFSET, 0.1);
  set_param(PARAM_SHADOW_SPLIT_2_OFFSET, 0.2);
  set_param(PARAM_SHADOW_SPLIT_3_OFFSET, 0.5);
  set_param(PARAM_SHADOW_FADE_START, 0.8);
  set_param(PARAM_SHADOW_PANCAKE_SIZE, 20.0);
  set_param(PARAM_SHADOW_OPACITY, 1.0);
  set_param(PARAM_SHADOW_BLUR, 1.0);
  set_param(PARAM_SHADOW_BIAS, 0.1);
  set_param(PARAM_SHADOW_NORMAL_BIAS, 1.0);
  set_param(PARAM_TRANSMITTANCE_BIAS, 0.05);
  set_param(PARAM_SHADOW_FADE_START, 1);
  // For OmniLight3D and SpotLight3D, specified in Lumens.
  set_param(PARAM_INTENSITY, 1000.0);
  // set_temperature(6500.0); // Nearly white.
  set_disable_scale(true);
}

void lain::Light3D::set_param(Param p_param, real_t p_value) {
  ERR_FAIL_INDEX(p_param, PARAM_MAX);
  param[p_param] = p_value;

  RS::get_singleton()->light_set_param(light, RS::LightParam(p_param), p_value);

  if (p_param == PARAM_SPOT_ANGLE || p_param == PARAM_RANGE) {
    update_gizmos();

    if (p_param == PARAM_SPOT_ANGLE) {
      // update_configuration_warnings();
    }
  }
}

real_t lain::Light3D::get_param(Param p_param) const {
	ERR_FAIL_INDEX_V(p_param, PARAM_MAX, 0);
	return param[p_param];
}

void lain::Light3D::set_shadow(bool p_enable) {
  shadow = p_enable;
  RS::get_singleton()->light_set_shadow(light, shadow);
}

bool lain::Light3D::has_shadow() const {
  return shadow;
}

lain::Light3D::~Light3D() {
  ERR_FAIL_NULL(RS::get_singleton());
  RS::get_singleton()->instance_set_base(get_instance(), RID());

  if (light.is_valid()) {
    RS::get_singleton()->free(light);
  }
}

void Light3D::set_color(const Color& p_color) {
  color = p_color;
  RS::get_singleton()->light_set_color(light, color);
}

Color lain::Light3D::get_color() const {
  return color;
}

void lain::Light3D::_update_visibility() {
  if (!is_inside_tree()) {
		return;
	}
  bool editor_ok = true;
	RS::get_singleton()->instance_set_visible(get_instance(), is_visible_in_tree() && editor_ok);

}

void Light3D::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_color", "color"), &Light3D::set_color);
  ClassDB::bind_method(D_METHOD("get_color"), &Light3D::get_color);
	ClassDB::bind_method(D_METHOD("set_shadow", "enabled"), &Light3D::set_shadow);
	ClassDB::bind_method(D_METHOD("has_shadow"), &Light3D::has_shadow);
	ClassDB::bind_method(D_METHOD("set_param", "param", "value"), &Light3D::set_param);
	ClassDB::bind_method(D_METHOD("get_param", "param"), &Light3D::get_param);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "shadow_enabled"), "set_shadow", "has_shadow");
  ADD_PROPERTY(PropertyInfo(Variant::COLOR, "light_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_color", "get_color");
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "light_energy", PROPERTY_HINT_RANGE, "0,16,0.001,or_greater"), "set_param", "get_param", PARAM_ENERGY);
ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "light_size", PROPERTY_HINT_RANGE, "0,1,0.001,or_greater,suffix:m"), "set_param", "get_param", PARAM_SIZE);
}

void lain::Light3D::_notification(int p_what) {
  switch (p_what) {
		case NOTIFICATION_TRANSFORM_CHANGED: {
			// update_configuration_warnings();
		} break;
		case NOTIFICATION_VISIBILITY_CHANGED:
		case NOTIFICATION_ENTER_TREE: {
			_update_visibility();
		} break;
	}
}

void DirectionalLight3D::_bind_methods() {

  ClassDB::bind_method(D_METHOD("set_shadow_mode", "mode"), &DirectionalLight3D::set_shadow_mode);
	ClassDB::bind_method(D_METHOD("get_shadow_mode"), &DirectionalLight3D::get_shadow_mode);


  ADD_PROPERTY(PropertyInfo(Variant::INT, "directional_shadow_mode", PROPERTY_HINT_ENUM, "Orthogonal (Fast),PSSM 2 Splits (Average),PSSM 4 Splits (Slow)"), "set_shadow_mode",
               "get_shadow_mode");
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "directional_shadow_max_distance", PROPERTY_HINT_RANGE, "0,8192,0.1,or_greater,exp"), "set_param", "get_param", PARAM_SHADOW_MAX_DISTANCE);

}

void lain::DirectionalLight3D::set_shadow_mode(ShadowMode p_mode) {
  shadow_mode = p_mode;
  RS::get_singleton()->light_directional_set_shadow_mode(light, RS::LightDirectionalShadowMode(p_mode));
  notify_property_list_changed();
}

DirectionalLight3D::ShadowMode lain::DirectionalLight3D::get_shadow_mode() const {
  return shadow_mode;
}

lain::DirectionalLight3D::DirectionalLight3D():
Light3D(RS::LIGHT_DIRECTIONAL) {
  set_param(PARAM_SHADOW_MAX_DISTANCE, 100);
	set_param(PARAM_SHADOW_FADE_START, 0.8);
	// Increase the default shadow normal bias to better suit most scenes.
	set_param(PARAM_SHADOW_NORMAL_BIAS, 2.0);
	set_param(PARAM_INTENSITY, 100000.0); // Specified in Lux, approximate mid-day sun.
	set_shadow_mode(SHADOW_PARALLEL_4_SPLITS);
	set_sky_mode(SKY_MODE_LIGHT_AND_SKY);
  blend_splits = false;
}


void DirectionalLight3D::set_sky_mode(SkyMode p_mode) {
	sky_mode = p_mode;
	RS::get_singleton()->light_directional_set_sky_mode(light, RS::LightDirectionalSkyMode(p_mode));
}

DirectionalLight3D::SkyMode DirectionalLight3D::get_sky_mode() const {
	return sky_mode;
}

void OmniLight3D::set_shadow_mode(ShadowMode p_mode) {
	shadow_mode = p_mode;
	RS::get_singleton()->light_omni_set_shadow_mode(light, RS::LightOmniShadowMode(p_mode));
}

OmniLight3D::ShadowMode OmniLight3D::get_shadow_mode() const {
	return shadow_mode;
}
OmniLight3D::OmniLight3D() :
		Light3D(RS::LIGHT_OMNI) {
	set_shadow_mode(SHADOW_CUBE);
}

void OmniLight3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_shadow_mode", "mode"), &OmniLight3D::set_shadow_mode);
	ClassDB::bind_method(D_METHOD("get_shadow_mode"), &OmniLight3D::get_shadow_mode);

	// ADD_GROUP("Omni", "omni_");
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "omni_range", PROPERTY_HINT_RANGE, "0,4096,0.001,or_greater,exp"), "set_param", "get_param", PARAM_RANGE);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "omni_attenuation", PROPERTY_HINT_RANGE, "-10,10,0.001,or_greater,or_less"), "set_param", "get_param", PARAM_ATTENUATION);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "omni_shadow_mode", PROPERTY_HINT_ENUM, "Dual Paraboloid,Cube"), "set_shadow_mode", "get_shadow_mode");

	// BIND_ENUM_CONSTANT(SHADOW_DUAL_PARABOLOID);
	// BIND_ENUM_CONSTANT(SHADOW_CUBE);
}
