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

bool lain::RendererSceneRender::environment_get_fog_enabled(RID p_env) const {
  return false;
}

RS::EnvironmentBG RendererSceneRender::environment_get_background(RID p_env) const {
	return environment_storage.environment_get_background(p_env);
}

RID RendererSceneRender::environment_get_sky(RID p_env) const {
	return environment_storage.environment_get_sky(p_env);
}

float RendererSceneRender::environment_get_sky_custom_fov(RID p_env) const {
	return environment_storage.environment_get_sky_custom_fov(p_env);
}

Basis RendererSceneRender::environment_get_sky_orientation(RID p_env) const {
	return environment_storage.environment_get_sky_orientation(p_env);
}

Color RendererSceneRender::environment_get_bg_color(RID p_env) const {
	return environment_storage.environment_get_bg_color(p_env);
}

float RendererSceneRender::environment_get_bg_energy_multiplier(RID p_env) const {
	return environment_storage.environment_get_bg_energy_multiplier(p_env);
}

float RendererSceneRender::environment_get_bg_intensity(RID p_env) const {
	return environment_storage.environment_get_bg_intensity(p_env);
}

int RendererSceneRender::environment_get_canvas_max_layer(RID p_env) const {
	return environment_storage.environment_get_canvas_max_layer(p_env);
}

RS::EnvironmentAmbientSource RendererSceneRender::environment_get_ambient_source(RID p_env) const {
	return environment_storage.environment_get_ambient_source(p_env);
}

Color RendererSceneRender::environment_get_ambient_light(RID p_env) const {
	return environment_storage.environment_get_ambient_light(p_env);
}

float RendererSceneRender::environment_get_ambient_light_energy(RID p_env) const {
	return environment_storage.environment_get_ambient_light_energy(p_env);
}

float RendererSceneRender::environment_get_ambient_sky_contribution(RID p_env) const {
	return environment_storage.environment_get_ambient_sky_contribution(p_env);
}

RS::EnvironmentReflectionSource RendererSceneRender::environment_get_reflection_source(RID p_env) const {
	return environment_storage.environment_get_reflection_source(p_env);
}

RID lain::RendererSceneRender::compositor_allocate() {
	return compositor_storage.compositor_allocate();

}

void lain::RendererSceneRender::compositor_initialize(RID p_rid) {
  compositor_storage.compositor_initialize(p_rid);
}

void lain::RendererSceneRender::compositor_free(RID p_rid) {
  compositor_storage.compositor_free(p_rid);
}

bool lain::RendererSceneRender::is_compositor(RID p_compositor) const {
  return compositor_storage.is_compositor(p_compositor);
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


/* Compositor effect API */

RID RendererSceneRender::compositor_effect_allocate() {
	return compositor_storage.compositor_effect_allocate();
}

void RendererSceneRender::compositor_effect_initialize(RID p_rid) {
	compositor_storage.compositor_effect_initialize(p_rid);
}

void RendererSceneRender::compositor_effect_free(RID p_rid) {
	compositor_storage.compositor_effect_free(p_rid);
}

bool RendererSceneRender::is_compositor_effect(RID p_effect) const {
	return compositor_storage.is_compositor_effect(p_effect);
}

void RendererSceneRender::compositor_effect_set_enabled(RID p_effect, bool p_enabled) {
	compositor_storage.compositor_effect_set_enabled(p_effect, p_enabled);
}

void RendererSceneRender::compositor_effect_set_callback(RID p_effect, RS::CompositorEffectCallbackType p_callback_type, const Callable &p_callback) {
	compositor_storage.compositor_effect_set_callback(p_effect, p_callback_type, p_callback);
}

void RendererSceneRender::compositor_effect_set_flag(RID p_effect, RS::CompositorEffectFlags p_flag, bool p_set) {
	compositor_storage.compositor_effect_set_flag(p_effect, p_flag, p_set);
}
// SSAO
void RendererSceneRender::environment_set_ssao(RID p_env, bool p_enable, float p_radius, float p_intensity, float p_power, float p_detail, float p_horizon, float p_sharpness, float p_light_affect, float p_ao_channel_affect) {
	environment_storage.environment_set_ssao(p_env, p_enable, p_radius, p_intensity, p_power, p_detail, p_horizon, p_sharpness, p_light_affect, p_ao_channel_affect);
}

bool lain::RendererSceneRender::environment_get_ssao_enabled(RID p_env) const {
	return environment_storage.environment_get_ssao_enabled(p_env);
}

void RendererSceneRender::environment_set_ssr(RID p_env, bool p_enable, int p_max_steps, float p_fade_int, float p_fade_out, float p_depth_tolerance) {
	environment_storage.environment_set_ssr(p_env, p_enable, p_max_steps, p_fade_int, p_fade_out, p_depth_tolerance);
}

bool RendererSceneRender::environment_get_ssr_enabled(RID p_env) const {
	return environment_storage.environment_get_ssr_enabled(p_env);
}

int RendererSceneRender::environment_get_ssr_max_steps(RID p_env) const {
	return environment_storage.environment_get_ssr_max_steps(p_env);
}

float RendererSceneRender::environment_get_ssr_fade_in(RID p_env) const {
	return environment_storage.environment_get_ssr_fade_in(p_env);
}

float RendererSceneRender::environment_get_ssr_fade_out(RID p_env) const {
	return environment_storage.environment_get_ssr_fade_out(p_env);
}

float RendererSceneRender::environment_get_ssr_depth_tolerance(RID p_env) const {
	return environment_storage.environment_get_ssr_depth_tolerance(p_env);
}

float lain::RendererSceneRender::environment_get_ssao_direct_light_affect(RID p_env) const {
 	return environment_storage.environment_get_ssao_direct_light_affect(p_env);
}

float lain::RendererSceneRender::environment_get_ssao_ao_channel_affect(RID p_env) const {
  return environment_storage.environment_get_ssao_ao_channel_affect(p_env);
}

// SSIL
void RendererSceneRender::environment_set_ssil(RID p_env, bool p_enable, float p_radius, float p_intensity, float p_sharpness, float p_normal_rejection) {
	environment_storage.environment_set_ssil(p_env, p_enable, p_radius, p_intensity, p_sharpness, p_normal_rejection);
}

bool RendererSceneRender::environment_get_ssil_enabled(RID p_env) const {
	return environment_storage.environment_get_ssil_enabled(p_env);
}

float RendererSceneRender::environment_get_ssil_radius(RID p_env) const {
	return environment_storage.environment_get_ssil_radius(p_env);
}

float RendererSceneRender::environment_get_ssil_intensity(RID p_env) const {
	return environment_storage.environment_get_ssil_intensity(p_env);
}

float RendererSceneRender::environment_get_ssil_sharpness(RID p_env) const {
	return environment_storage.environment_get_ssil_sharpness(p_env);
}

float RendererSceneRender::environment_get_ssil_normal_rejection(RID p_env) const {
	return environment_storage.environment_get_ssil_normal_rejection(p_env);
}


// background

void RendererSceneRender::environment_set_background(RID p_env, RS::EnvironmentBG p_bg) {
	environment_storage.environment_set_background(p_env, p_bg);
}

void RendererSceneRender::environment_set_sky(RID p_env, RID p_sky) {
	environment_storage.environment_set_sky(p_env, p_sky);
}

void RendererSceneRender::environment_set_sky_custom_fov(RID p_env, float p_scale) {
	environment_storage.environment_set_sky_custom_fov(p_env, p_scale);
}

void RendererSceneRender::environment_set_sky_orientation(RID p_env, const Basis &p_orientation) {
	environment_storage.environment_set_sky_orientation(p_env, p_orientation);
}

void RendererSceneRender::environment_set_bg_color(RID p_env, const Color &p_color) {
	environment_storage.environment_set_bg_color(p_env, p_color);
}

void RendererSceneRender::environment_set_bg_energy(RID p_env, float p_multiplier, float p_exposure_value) {
	environment_storage.environment_set_bg_energy(p_env, p_multiplier, p_exposure_value);
}

void RendererSceneRender::environment_set_canvas_max_layer(RID p_env, int p_max_layer) {
	environment_storage.environment_set_canvas_max_layer(p_env, p_max_layer);
}

void RendererSceneRender::environment_set_ambient_light(RID p_env, const Color &p_color, RS::EnvironmentAmbientSource p_ambient, float p_energy, float p_sky_contribution, RS::EnvironmentReflectionSource p_reflection_source) {
	environment_storage.environment_set_ambient_light(p_env, p_color, p_ambient, p_energy, p_sky_contribution, p_reflection_source);
}
