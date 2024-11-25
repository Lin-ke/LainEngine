#include "camera_attributes.h"
#include "function/render/rendering_system/rendering_system.h"

lain::CameraAttributes::CameraAttributes() {
 	camera_attributes = RS::get_singleton()->camera_attributes_create();
}