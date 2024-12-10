#include "light_3d.h"
using namespace lain;
lain::Light3D::Light3D() {}

lain::Light3D::~Light3D() {
  ERR_FAIL_NULL(RS::get_singleton());
	RS::get_singleton()->instance_set_base(get_instance(), RID());

	if (light.is_valid()) {
		RS::get_singleton()->free(light);
	}
}

void Light3D::set_color(const Color &p_color) {
	color = p_color;
  RS::get_singleton()->light_set_color(light, color);
}

void DirectionalLight3D::_bind_methods() {
	ADD_PROPERTY(PropertyInfo(Variant::INT, "directional_shadow_mode", PROPERTY_HINT_ENUM, "Orthogonal (Fast),PSSM 2 Splits (Average),PSSM 4 Splits (Slow)"), "set_shadow_mode", "get_shadow_mode");

}

void lain::DirectionalLight3D::set_shadow_mode(ShadowMode p_mode) {
  	shadow_mode = p_mode;
	RS::get_singleton()->light_directional_set_shadow_mode(light, RS::LightDirectionalShadowMode(p_mode));
	notify_property_list_changed();
}

DirectionalLight3D::ShadowMode lain::DirectionalLight3D::get_shadow_mode() const {
  return shadow_mode;
}
