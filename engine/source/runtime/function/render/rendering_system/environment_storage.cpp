#include "environment_storage.h"

using namespace lain;
// Storage

RendererEnvironmentStorage *RendererEnvironmentStorage::singleton = nullptr;
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
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL(env);
	env->background = p_bg;
}

void RendererEnvironmentStorage::environment_set_sky(RID p_env, RID p_sky) {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL(env);
	env->sky = p_sky;
}

void RendererEnvironmentStorage::environment_set_sky_custom_fov(RID p_env, float p_scale) {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL(env);
	env->sky_custom_fov = p_scale;
}

void RendererEnvironmentStorage::environment_set_sky_orientation(RID p_env, const Basis &p_orientation) {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL(env);
	env->sky_orientation = p_orientation;
}

void RendererEnvironmentStorage::environment_set_bg_color(RID p_env, const Color &p_color) {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL(env);
	env->bg_color = p_color;
}

void RendererEnvironmentStorage::environment_set_bg_energy(RID p_env, float p_multiplier, float p_intensity) {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL(env);
	env->bg_energy_multiplier = p_multiplier;
	env->bg_intensity = p_intensity;
}

void RendererEnvironmentStorage::environment_set_canvas_max_layer(RID p_env, int p_max_layer) {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL(env);
	env->canvas_max_layer = p_max_layer;
}

void RendererEnvironmentStorage::environment_set_ambient_light(RID p_env, const Color &p_color, RS::EnvironmentAmbientSource p_ambient, float p_energy, float p_sky_contribution, RS::EnvironmentReflectionSource p_reflection_source) {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL(env);
	env->ambient_light = p_color;
	env->ambient_source = p_ambient;
	env->ambient_light_energy = p_energy;
	env->ambient_sky_contribution = p_sky_contribution;
	env->reflection_source = p_reflection_source;
}


RS::EnvironmentBG RendererEnvironmentStorage::environment_get_background(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, RS::ENV_BG_CLEAR_COLOR);
	return env->background;
}

RID RendererEnvironmentStorage::environment_get_sky(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, RID());
	return env->sky;
}

float RendererEnvironmentStorage::environment_get_sky_custom_fov(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 0.0);
	return env->sky_custom_fov;
}

Basis RendererEnvironmentStorage::environment_get_sky_orientation(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, Basis());
	return env->sky_orientation;
}

Color RendererEnvironmentStorage::environment_get_bg_color(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, Color());
	return env->bg_color;
}

float RendererEnvironmentStorage::environment_get_bg_energy_multiplier(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 1.0);
	return env->bg_energy_multiplier;
}

float RendererEnvironmentStorage::environment_get_bg_intensity(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 1.0);
	return env->bg_intensity;
}

int RendererEnvironmentStorage::environment_get_canvas_max_layer(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 0);
	return env->canvas_max_layer;
}

RS::EnvironmentAmbientSource RendererEnvironmentStorage::environment_get_ambient_source(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, RS::ENV_AMBIENT_SOURCE_BG);
	return env->ambient_source;
}

Color RendererEnvironmentStorage::environment_get_ambient_light(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, Color());
	return env->ambient_light;
}

float RendererEnvironmentStorage::environment_get_ambient_light_energy(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 1.0);
	return env->ambient_light_energy;
}

float RendererEnvironmentStorage::environment_get_ambient_sky_contribution(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, 1.0);
	return env->ambient_sky_contribution;
}

RS::EnvironmentReflectionSource RendererEnvironmentStorage::environment_get_reflection_source(RID p_env) const {
	Environment *env = environment_owner.get_or_null(p_env);
	ERR_FAIL_NULL_V(env, RS::ENV_REFLECTION_SOURCE_BG);
	return env->reflection_source;
}
