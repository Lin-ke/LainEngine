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
