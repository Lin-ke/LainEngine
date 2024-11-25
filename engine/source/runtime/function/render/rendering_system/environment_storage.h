#ifndef ENVIRONMENT_STORAGE_H
#define ENVIRONMENT_STORAGE_H

#include "core/io/rid_owner.h"
#include "rendering_system.h"
namespace lain {
// environment storage的实现与后端无关
class RendererEnvironmentStorage {
 private:
  static RendererEnvironmentStorage* singleton;
  struct Environment {
    // BG
    RS::EnvironmentBG background = RS::ENV_BG_CLEAR_COLOR;
		RID sky;
		float sky_custom_fov = 0.0;
		Basis sky_orientation;
		Color bg_color;
		float bg_energy_multiplier = 1.0;
		float bg_intensity = 1.0; // Measured in nits or candela/m^2. Default to 1.0 so this doesn't impact rendering when Physical Light Units disabled.
		int canvas_max_layer = 0;
		RS::EnvironmentAmbientSource ambient_source = RS::ENV_AMBIENT_SOURCE_BG;
		Color ambient_light;
		float ambient_light_energy = 1.0;
		float ambient_sky_contribution = 1.0;
		RS::EnvironmentReflectionSource reflection_source = RS::ENV_REFLECTION_SOURCE_BG;

		// Tonemap
		RS::EnvironmentToneMapper tone_mapper;
		float exposure = 1.0;
		float white = 1.0;

    		// Glow
		bool glow_enabled = false;
		Vector<float> glow_levels;
		float glow_intensity = 0.8;
		float glow_strength = 1.0;
		float glow_bloom = 0.0;
		float glow_mix = 0.01;
		RS::EnvironmentGlowBlendMode glow_blend_mode = RS::ENV_GLOW_BLEND_MODE_SOFTLIGHT;
		float glow_hdr_bleed_threshold = 1.0;
		float glow_hdr_luminance_cap = 12.0;
		float glow_hdr_bleed_scale = 2.0;
		float glow_map_strength = 0.0f; // 1.0f in GLES3 ??
		RID glow_map;

    // SSR
		bool ssr_enabled = false;
		int ssr_max_steps = 64;
		float ssr_fade_in = 0.15;
		float ssr_fade_out = 2.0;
		float ssr_depth_tolerance = 0.2;

		// SSAO
		bool ssao_enabled = false;
		float ssao_radius = 1.0;
		float ssao_intensity = 2.0;
		float ssao_power = 1.5;
		float ssao_detail = 0.5;
		float ssao_horizon = 0.06;
		float ssao_sharpness = 0.98;
		float ssao_direct_light_affect = 0.0;
		float ssao_ao_channel_affect = 0.0;

		// SSIL
		bool ssil_enabled = false;
		float ssil_radius = 5.0;
		float ssil_intensity = 1.0;
		float ssil_sharpness = 0.98;
		float ssil_normal_rejection = 1.0;

		// SDFGI
		bool sdfgi_enabled = false;
		int sdfgi_cascades = 4;
		float sdfgi_min_cell_size = 0.2;
		bool sdfgi_use_occlusion = false;
		float sdfgi_bounce_feedback = 0.5;
		bool sdfgi_read_sky_light = true;
		float sdfgi_energy = 1.0;
		float sdfgi_normal_bias = 1.1;
		float sdfgi_probe_bias = 1.1;
		RS::EnvironmentSDFGIYScale sdfgi_y_scale = RS::ENV_SDFGI_Y_SCALE_75_PERCENT;

		// Adjustments
		bool adjustments_enabled = false;
		float adjustments_brightness = 1.0f;
		float adjustments_contrast = 1.0f;
		float adjustments_saturation = 1.0f;
		bool use_1d_color_correction = false;
		RID color_correction;
  };
  mutable RID_Owner<Environment,true> environment_owner;
public:
	static RendererEnvironmentStorage *get_singleton() { return singleton; }

	RendererEnvironmentStorage();
	virtual ~RendererEnvironmentStorage();

	// Environment
	RID environment_allocate();
	void environment_initialize(RID p_rid);
	void environment_free(RID p_rid);

	bool is_environment(RID p_environment) const {
		return environment_owner.owns(p_environment);
	}

  	// Background
	void environment_set_background(RID p_env, RS::EnvironmentBG p_bg);
	void environment_set_sky(RID p_env, RID p_sky);
	void environment_set_sky_custom_fov(RID p_env, float p_scale);
	void environment_set_sky_orientation(RID p_env, const Basis &p_orientation);
	void environment_set_bg_color(RID p_env, const Color &p_color);
	void environment_set_bg_energy(RID p_env, float p_multiplier, float p_exposure_value);
	void environment_set_canvas_max_layer(RID p_env, int p_max_layer);
	void environment_set_ambient_light(RID p_env, const Color &p_color, RS::EnvironmentAmbientSource p_ambient = RS::ENV_AMBIENT_SOURCE_BG, float p_energy = 1.0, float p_sky_contribution = 0.0, RS::EnvironmentReflectionSource p_reflection_source = RS::ENV_REFLECTION_SOURCE_BG);

  RS::EnvironmentBG environment_get_background(RID p_env) const;
	RID environment_get_sky(RID p_env) const;
	float environment_get_sky_custom_fov(RID p_env) const;
	Basis environment_get_sky_orientation(RID p_env) const;
	Color environment_get_bg_color(RID p_env) const;
	float environment_get_bg_energy_multiplier(RID p_env) const;
	float environment_get_bg_intensity(RID p_env) const;
	int environment_get_canvas_max_layer(RID p_env) const;
	RS::EnvironmentAmbientSource environment_get_ambient_source(RID p_env) const;
	Color environment_get_ambient_light(RID p_env) const;
	float environment_get_ambient_light_energy(RID p_env) const;
	float environment_get_ambient_sky_contribution(RID p_env) const;
	RS::EnvironmentReflectionSource environment_get_reflection_source(RID p_env) const;

	// SSAO
	void environment_set_ssao(RID p_env, bool p_enable, float p_radius, float p_intensity, float p_power, float p_detail, float p_horizon, float p_sharpness, float p_light_affect, float p_ao_channel_affect);
	bool environment_get_ssao_enabled(RID p_env) const;
	float environment_get_ssao_radius(RID p_env) const;
	float environment_get_ssao_intensity(RID p_env) const;
	float environment_get_ssao_power(RID p_env) const;
	float environment_get_ssao_detail(RID p_env) const;
	float environment_get_ssao_horizon(RID p_env) const;
	float environment_get_ssao_sharpness(RID p_env) const;
	float environment_get_ssao_direct_light_affect(RID p_env) const;
	float environment_get_ssao_ao_channel_affect(RID p_env) const;

		// SSIL
	void environment_set_ssil(RID p_env, bool p_enable, float p_radius, float p_intensity, float p_sharpness, float p_normal_rejection);
	bool environment_get_ssil_enabled(RID p_env) const;
	float environment_get_ssil_radius(RID p_env) const;
	float environment_get_ssil_intensity(RID p_env) const;
	float environment_get_ssil_sharpness(RID p_env) const;
	float environment_get_ssil_normal_rejection(RID p_env) const;
	// SSR
	void environment_set_ssr(RID p_env, bool p_enable, int p_max_steps, float p_fade_int, float p_fade_out, float p_depth_tolerance);
	bool environment_get_ssr_enabled(RID p_env) const;
	int environment_get_ssr_max_steps(RID p_env) const;
	float environment_get_ssr_fade_in(RID p_env) const;
	float environment_get_ssr_fade_out(RID p_env) const;
	float environment_get_ssr_depth_tolerance(RID p_env) const;
};
}  // namespace lain
#endif