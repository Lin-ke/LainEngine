#include "camera_attributes.h"
#include "function/render/rendering_system/rendering_system.h"

using namespace lain;

lain::CameraAttributes::CameraAttributes() {
 	camera_attributes = RS::get_singleton()->camera_attributes_create();
}

CameraAttributes::~CameraAttributes() {
	ERR_FAIL_NULL(RS::get_singleton());
	RS::get_singleton()->free(camera_attributes);
}


real_t CameraAttributesPhysical::get_near() const {
	return frustum_near;
}
real_t CameraAttributesPhysical::get_far() const {
	return frustum_far;
}

real_t CameraAttributesPhysical::get_fov() const {
	return frustum_fov;
}