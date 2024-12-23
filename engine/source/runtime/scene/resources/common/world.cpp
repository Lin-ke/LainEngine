#include "world.h"
#include "camera_attributes.h"
using namespace lain;

void World3D::_register_camera(Camera3D *p_camera) {
	cameras.insert(p_camera);
}

void World3D::_remove_camera(Camera3D *p_camera) {
	cameras.erase(p_camera);
}

Ref<CameraAttributes> World3D::get_camera_attributes() const {
	return camera_attributes;
}


void World3D::set_environment(const Ref<Environment> &p_environment) {
	if (environment == p_environment) {
		return;
	}

	environment = p_environment;
	if (environment.is_valid()) {
		RS::get_singleton()->scenario_set_environment(scenario, environment->GetRID());
	} else {
		RS::get_singleton()->scenario_set_environment(scenario, RID());
	}

	emit_changed();
}

Ref<Environment> World3D::get_environment() const {
	return environment;
}


void World3D::set_camera_attributes(const Ref<CameraAttributes> &p_camera_attributes) {
	camera_attributes = p_camera_attributes;
	if (camera_attributes.is_valid()) {
		RS::get_singleton()->scenario_set_camera_attributes(scenario, camera_attributes->GetRID());
	} else {
		RS::get_singleton()->scenario_set_camera_attributes(scenario, RID());
	}
}
lain::World3D::World3D() {
	scenario = RS::get_singleton()->scenario_create();
}

void World3D::set_compositor(const Ref<Compositor> &p_compositor) {
	compositor = p_compositor;
	if (compositor.is_valid()) {
		RS::get_singleton()->scenario_set_compositor(scenario, compositor->GetRID());
	} else {
		RS::get_singleton()->scenario_set_compositor(scenario, RID());
	}
}
Ref<Compositor> World3D::get_compositor() const {
	return compositor;
}
lain::World2D::World2D() {}

lain::World2D::~World2D() {}
