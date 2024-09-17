#include "renderer_scene_cull.h"
using namespace lain;
RID lain::RendererSceneCull::camera_allocate() {
  return camera_owner.allocate_rid();
}

void RendererSceneCull::camera_initialize(RID p_rid) {
	camera_owner.initialize_rid(p_rid);
}
void RendererSceneCull::camera_set_perspective(RID p_camera, float p_fovy_degrees, float p_z_near, float p_z_far) {
	Camera *camera = camera_owner.get_or_null(p_camera);
	ERR_FAIL_NULL(camera);
	camera->type = Camera::PERSPECTIVE;
	camera->fov = p_fovy_degrees;
	camera->znear = p_z_near;
	camera->zfar = p_z_far;
}

void RendererSceneCull::camera_set_orthogonal(RID p_camera, float p_size, float p_z_near, float p_z_far) {
	Camera *camera = camera_owner.get_or_null(p_camera);
	ERR_FAIL_NULL(camera);
	camera->type = Camera::ORTHOGONAL;
	camera->size = p_size;
	camera->znear = p_z_near;
	camera->zfar = p_z_far;
}

void RendererSceneCull::camera_set_frustum(RID p_camera, float p_size, Vector2 p_offset, float p_z_near, float p_z_far) {
	Camera *camera = camera_owner.get_or_null(p_camera);
	ERR_FAIL_NULL(camera);
	camera->type = Camera::FRUSTUM;
	camera->size = p_size;
	camera->offset = p_offset;
	camera->znear = p_z_near;
	camera->zfar = p_z_far;
}

void RendererSceneCull::camera_set_transform(RID p_camera, const Transform3D &p_transform) {
	Camera *camera = camera_owner.get_or_null(p_camera);
	ERR_FAIL_NULL(camera);
	camera->transform = p_transform.orthonormalized();
}

void RendererSceneCull::camera_set_cull_mask(RID p_camera, uint32_t p_layers) {
	Camera *camera = camera_owner.get_or_null(p_camera);
	ERR_FAIL_NULL(camera);

	camera->visible_layers = p_layers;
}

void RendererSceneCull::camera_set_environment(RID p_camera, RID p_env) {
	Camera *camera = camera_owner.get_or_null(p_camera);
	ERR_FAIL_NULL(camera);
	camera->env = p_env;
}

void RendererSceneCull::camera_set_camera_attributes(RID p_camera, RID p_attributes) {
	Camera *camera = camera_owner.get_or_null(p_camera);
	ERR_FAIL_NULL(camera);
	camera->attributes = p_attributes;
}

void RendererSceneCull::camera_set_compositor(RID p_camera, RID p_compositor) {
	Camera *camera = camera_owner.get_or_null(p_camera);
	ERR_FAIL_NULL(camera);
	camera->compositor = p_compositor;
}

void RendererSceneCull::camera_set_use_vertical_aspect(RID p_camera, bool p_enable) {
	Camera *camera = camera_owner.get_or_null(p_camera);
	ERR_FAIL_NULL(camera);
	camera->vaspect = p_enable;
}

bool lain::RendererSceneCull::is_camera(RID p_camera) const {
	return camera_owner.owns(p_camera);
}
