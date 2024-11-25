#include "environment_storage.h"

using namespace lain;
// Storage

RendererEnvironmentStorage* RendererEnvironmentStorage::singleton = nullptr;
RendererEnvironmentStorage::RendererEnvironmentStorage() {
  singleton = this;
}

RendererEnvironmentStorage::~RendererEnvironmentStorage() {
  singleton = nullptr;
}

// Environment

RID RendererEnvironmentStorage::environment_allocate() {
  return environment_owner.allocate_rid();
}

void RendererEnvironmentStorage::environment_initialize(RID p_rid) {
  environment_owner.initialize_rid(p_rid, Environment());
}

void RendererEnvironmentStorage::environment_free(RID p_rid) {
  environment_owner.free(p_rid);
}

// Background

void RendererEnvironmentStorage::environment_set_background(RID p_env, RS::EnvironmentBG p_bg) {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL(env);
  env->background = p_bg;
}

void RendererEnvironmentStorage::environment_set_sky(RID p_env, RID p_sky) {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL(env);
  env->sky = p_sky;
}

void RendererEnvironmentStorage::environment_set_sky_custom_fov(RID p_env, float p_scale) {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL(env);
  env->sky_custom_fov = p_scale;
}

void RendererEnvironmentStorage::environment_set_sky_orientation(RID p_env, const Basis& p_orientation) {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL(env);
  env->sky_orientation = p_orientation;
}

void RendererEnvironmentStorage::environment_set_bg_color(RID p_env, const Color& p_color) {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL(env);
  env->bg_color = p_color;
}

void RendererEnvironmentStorage::environment_set_bg_energy(RID p_env, float p_multiplier, float p_intensity) {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL(env);
  env->bg_energy_multiplier = p_multiplier;
  env->bg_intensity = p_intensity;
}

void RendererEnvironmentStorage::environment_set_canvas_max_layer(RID p_env, int p_max_layer) {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL(env);
  env->canvas_max_layer = p_max_layer;
}

void RendererEnvironmentStorage::environment_set_ambient_light(RID p_env, const Color& p_color, RS::EnvironmentAmbientSource p_ambient, float p_energy,
                                                               float p_sky_contribution, RS::EnvironmentReflectionSource p_reflection_source) {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL(env);
  env->ambient_light = p_color;
  env->ambient_source = p_ambient;
  env->ambient_light_energy = p_energy;
  env->ambient_sky_contribution = p_sky_contribution;
  env->reflection_source = p_reflection_source;
}

RS::EnvironmentBG RendererEnvironmentStorage::environment_get_background(RID p_env) const {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL_V(env, RS::ENV_BG_CLEAR_COLOR);
  return env->background;
}

RID RendererEnvironmentStorage::environment_get_sky(RID p_env) const {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL_V(env, RID());
  return env->sky;
}

float RendererEnvironmentStorage::environment_get_sky_custom_fov(RID p_env) const {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL_V(env, 0.0);
  return env->sky_custom_fov;
}

Basis RendererEnvironmentStorage::environment_get_sky_orientation(RID p_env) const {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL_V(env, Basis());
  return env->sky_orientation;
}

Color RendererEnvironmentStorage::environment_get_bg_color(RID p_env) const {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL_V(env, Color());
  return env->bg_color;
}

float RendererEnvironmentStorage::environment_get_bg_energy_multiplier(RID p_env) const {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL_V(env, 1.0);
  return env->bg_energy_multiplier;
}

float RendererEnvironmentStorage::environment_get_bg_intensity(RID p_env) const {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL_V(env, 1.0);
  return env->bg_intensity;
}

int RendererEnvironmentStorage::environment_get_canvas_max_layer(RID p_env) const {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL_V(env, 0);
  return env->canvas_max_layer;
}

RS::EnvironmentAmbientSource RendererEnvironmentStorage::environment_get_ambient_source(RID p_env) const {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL_V(env, RS::ENV_AMBIENT_SOURCE_BG);
  return env->ambient_source;
}

Color RendererEnvironmentStorage::environment_get_ambient_light(RID p_env) const {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL_V(env, Color());
  return env->ambient_light;
}

float RendererEnvironmentStorage::environment_get_ambient_light_energy(RID p_env) const {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL_V(env, 1.0);
  return env->ambient_light_energy;
}

float RendererEnvironmentStorage::environment_get_ambient_sky_contribution(RID p_env) const {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL_V(env, 1.0);
  return env->ambient_sky_contribution;
}

RS::EnvironmentReflectionSource RendererEnvironmentStorage::environment_get_reflection_source(RID p_env) const {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL_V(env, RS::ENV_REFLECTION_SOURCE_BG);
  return env->reflection_source;
}

void lain::RendererEnvironmentStorage::environment_set_ssao(RID p_env, bool p_enable, float p_radius, float p_intensity, float p_power, float p_detail, float p_horizon,
                                                            float p_sharpness, float p_light_affect, float p_ao_channel_affect) {
  Environment* env = environment_owner.get_or_null(p_env);
  ERR_FAIL_NULL(env);
#ifdef DEBUG_ENABLED
  if (OS::GetSingleton()->is_forward_rdm() && p_enable) {
    WARN_PRINT_ONCE_ED("Screen-space ambient occlusion (SSAO) can only be enabled when using the Forward+ rendering backend.");
  }
#endif
  env->ssao_enabled = p_enable;
  env->ssao_radius = p_radius;
  env->ssao_intensity = p_intensity;
  env->ssao_power = p_power;
  env->ssao_detail = p_detail;
  env->ssao_horizon = p_horizon;
  env->ssao_sharpness = p_sharpness;
  env->ssao_direct_light_affect = p_light_affect;
  env->ssao_ao_channel_affect = p_ao_channel_affect;
}

bool RendererEnvironmentStorage::environment_get_ssao_enabled(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, false);
	return env->ssao_enabled;
}


float RendererEnvironmentStorage::environment_get_ssao_radius(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 1.0);
	return env->ssao_radius;
}

float RendererEnvironmentStorage::environment_get_ssao_intensity(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 2.0);
	return env->ssao_intensity;
}

float RendererEnvironmentStorage::environment_get_ssao_power(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 1.5);
	return env->ssao_power;
}

float RendererEnvironmentStorage::environment_get_ssao_detail(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 0.5);
	return env->ssao_detail;
}

float RendererEnvironmentStorage::environment_get_ssao_horizon(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 0.06);
	return env->ssao_horizon;
}

float RendererEnvironmentStorage::environment_get_ssao_sharpness(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 0.98);
	return env->ssao_sharpness;
}

float RendererEnvironmentStorage::environment_get_ssao_direct_light_affect(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 0.0);
	return env->ssao_direct_light_affect;
}

float RendererEnvironmentStorage::environment_get_ssao_ao_channel_affect(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 0.0);
	return env->ssao_ao_channel_affect;
}


void RendererEnvironmentStorage::environment_set_ssil(RID p_env, bool p_enable, float p_radius, float p_intensity, float p_sharpness, float p_normal_rejection) {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL(env);
#ifdef DEBUG_ENABLED
	if (OS::GetSingleton()->is_forward_rdm() && p_enable) {
		WARN_PRINT_ONCE_ED("Screen-space indirect lighting (SSIL) can only be enabled when using the Forward+ rendering backend.");
	}
#endif
	env->ssil_enabled = p_enable;
	env->ssil_radius = p_radius;
	env->ssil_intensity = p_intensity;
	env->ssil_sharpness = p_sharpness;
	env->ssil_normal_rejection = p_normal_rejection;
}

bool RendererEnvironmentStorage::environment_get_ssil_enabled(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, false);
	return env->ssil_enabled;
}

float RendererEnvironmentStorage::environment_get_ssil_radius(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 5.0);
	return env->ssil_radius;
}

float RendererEnvironmentStorage::environment_get_ssil_intensity(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 1.0);
	return env->ssil_intensity;
}

float RendererEnvironmentStorage::environment_get_ssil_sharpness(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 0.98);
	return env->ssil_sharpness;
}

float RendererEnvironmentStorage::environment_get_ssil_normal_rejection(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 1.0);
	return env->ssil_normal_rejection;
}
// SSR

void RendererEnvironmentStorage::environment_set_ssr(RID p_env, bool p_enable, int p_max_steps, float p_fade_int, float p_fade_out, float p_depth_tolerance) {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL(env);
#ifdef DEBUG_ENABLED
	if (OS::GetSingleton()->is_forward_rdm() && p_enable) {
		WARN_PRINT_ONCE_ED("Screen-space reflections (SSR) can only be enabled when using the Forward+ rendering backend.");
	}
#endif
	env->ssr_enabled = p_enable;
	env->ssr_max_steps = p_max_steps;
	env->ssr_fade_in = p_fade_int;
	env->ssr_fade_out = p_fade_out;
	env->ssr_depth_tolerance = p_depth_tolerance;
}

bool RendererEnvironmentStorage::environment_get_ssr_enabled(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, false);
	return env->ssr_enabled;
}

int RendererEnvironmentStorage::environment_get_ssr_max_steps(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 64);
	return env->ssr_max_steps;
}

float RendererEnvironmentStorage::environment_get_ssr_fade_in(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 0.15);
	return env->ssr_fade_in;
}

float RendererEnvironmentStorage::environment_get_ssr_fade_out(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 2.0);
	return env->ssr_fade_out;
}

float RendererEnvironmentStorage::environment_get_ssr_depth_tolerance(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 0.2);
	return env->ssr_depth_tolerance;
}