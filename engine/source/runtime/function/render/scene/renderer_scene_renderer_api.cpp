#include "renderer_scene_renderer_api.h"
using namespace lain;

RID RendererSceneRender::environment_allocate() {
  return environment_storage.environment_allocate();
}

void lain::RendererSceneRender::environment_initialize(RID p_rid) {
  environment_storage.environment_initialize(p_rid);
}

void lain::RendererSceneRender::environment_free(RID p_rid) {
  environment_storage.environment_free(p_rid);
}

bool lain::RendererSceneRender::is_environment(RID p_env) const {
  return environment_storage.is_environment(p_env);
}

RS::EnvironmentBG lain::RendererSceneRender::environment_get_background(RID p_env) const {
  return environment_storage.environment_get_background(p_env);
}

int lain::RendererSceneRender::environment_get_canvas_max_layer(RID p_env) const {
  return environment_storage.environment_get_canvas_max_layer(p_env);
}

void lain::RendererSceneRender::CameraData::set_camera(const Transform3D p_transform, const Projection p_projection, bool p_is_orthogonal, bool p_vaspect,
                                                       const Vector2& p_taa_jitter, uint32_t p_visible_layers) {
  view_count = 1;
  is_orthogonal = p_is_orthogonal;
  vaspect = p_vaspect;

  main_transform = p_transform;
  main_projection = p_projection;

  visible_layers = p_visible_layers;
  view_offset[0] = Transform3D();
  view_projection[0] = p_projection;
  taa_jitter = p_taa_jitter;
}
