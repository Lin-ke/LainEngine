#ifndef RENDERING_METHOD_API_H
#define RENDERING_METHOD_API_H
#include "rendering_system.h"
#include "function/render/scene/render_scene_buffers_api.h"
namespace lain{
class RenderingMethod {
	//负责 camera, occluder, scenario, instance, environment 的创建和初始化
	// 实现为 RendererSceneCull
	// 其中 environment 传给了 scene，用scene实际管理执行
	// enviroment 实际传给了 environment_storage
	// scenario 由 实现类型 RendererSceneCull 实现
  public:
	
	struct RenderInfo {
		int info[RS::VIEWPORT_RENDER_INFO_TYPE_MAX][RS::VIEWPORT_RENDER_INFO_MAX] = {};
	};

  // camera 
  virtual RID camera_allocate() = 0;
	virtual void camera_initialize(RID p_rid) = 0;

	virtual void camera_set_perspective(RID p_camera, float p_fovy_degrees, float p_z_near, float p_z_far) = 0;
	virtual void camera_set_orthogonal(RID p_camera, float p_size, float p_z_near, float p_z_far) = 0;
	virtual void camera_set_frustum(RID p_camera, float p_size, Vector2 p_offset, float p_z_near, float p_z_far) = 0;
	virtual void camera_set_transform(RID p_camera, const Transform3D &p_transform) = 0;
	virtual void camera_set_cull_mask(RID p_camera, uint32_t p_layers) = 0;
	virtual void camera_set_environment(RID p_camera, RID p_env) = 0;
	virtual void camera_set_camera_attributes(RID p_camera, RID p_attributes) = 0;
	virtual void camera_set_compositor(RID p_camera, RID p_compositor) = 0;
	virtual void camera_set_use_vertical_aspect(RID p_camera, bool p_enable) = 0;
	virtual bool is_camera(RID p_camera) const = 0;
  // occluder
  virtual RID occluder_allocate() = 0;
	virtual void occluder_initialize(RID p_occluder) = 0;
	virtual void occluder_set_mesh(RID p_occluder, const PackedVector3Array &p_vertices, const PackedInt32Array &p_indices) = 0;

  // senario
	virtual RID scenario_allocate() = 0;
	virtual void scenario_initialize(RID p_rid) = 0;
	virtual void scenario_set_environment(RID p_scenario, RID p_environment) = 0;
	virtual void scenario_set_camera_attributes(RID p_scenario, RID p_attributes) = 0;
	virtual void scenario_set_fallback_environment(RID p_scenario, RID p_environment) = 0;
	virtual void scenario_set_compositor(RID p_scenario, RID p_compositor) = 0;
	virtual void scenario_set_reflection_atlas_size(RID p_scenario, int p_reflection_size, int p_reflection_count) = 0;
	virtual bool is_scenario(RID p_scenario) const = 0;
	virtual RID scenario_get_environment(RID p_scenario) = 0;
	virtual void scenario_add_viewport_visibility_mask(RID p_scenario, RID p_viewport) = 0;
	virtual void scenario_remove_viewport_visibility_mask(RID p_scenario, RID p_viewport) = 0;

  // instance
  	virtual RID instance_allocate() = 0;
	virtual void instance_initialize(RID p_rid) = 0;

	virtual void instance_set_base(RID p_instance, RID p_base) = 0;
	virtual void instance_set_scenario(RID p_instance, RID p_scenario) = 0;
	virtual void instance_set_layer_mask(RID p_instance, uint32_t p_mask) = 0;
	virtual void instance_set_pivot_data(RID p_instance, float p_sorting_offset, bool p_use_aabb_center) = 0;
	virtual void instance_set_transform(RID p_instance, const Transform3D &p_transform) = 0;
	virtual void instance_attach_object_instance_id(RID p_instance, ObjectID p_id) = 0;
	virtual void instance_set_blend_shape_weight(RID p_instance, int p_shape, float p_weight) = 0;
	virtual void instance_set_surface_override_material(RID p_instance, int p_surface, RID p_material) = 0;
	virtual void instance_set_visible(RID p_instance, bool p_visible) = 0;
	virtual void instance_geometry_set_transparency(RID p_instance, float p_transparency) = 0;

	virtual void instance_set_custom_aabb(RID p_instance, AABB p_aabb) = 0;

	virtual void instance_attach_skeleton(RID p_instance, RID p_skeleton) = 0;

	virtual void instance_set_extra_visibility_margin(RID p_instance, real_t p_margin) = 0;
	virtual void instance_set_visibility_parent(RID p_instance, RID p_parent_instance) = 0;

	virtual void instance_set_ignore_culling(RID p_instance, bool p_enabled) = 0;

	virtual void instance_geometry_set_material_override(RID p_instance, RID p_material) = 0;
	virtual void instance_geometry_set_lightmap(RID p_instance, RID p_lightmap,const Rect2 &p_lightmap_uv_scale, int p_slice_index ) = 0;
	virtual void instance_geometry_set_material_overlay(RID p_instance, RID p_material) = 0;
	// shader parameter
	virtual void instance_geometry_set_shader_parameter(RID p_instance, const StringName &p_parameter, const Variant &p_value) = 0;
	virtual void instance_geometry_get_shader_parameter_list(RID p_instance, List<PropertyInfo> *p_parameters) const = 0;
	virtual Variant instance_geometry_get_shader_parameter(RID p_instance, const StringName &p_parameter) const = 0;
	virtual Variant instance_geometry_get_shader_parameter_default_value(RID p_instance, const StringName &p_parameter) const = 0;
	/* ENVIRONMENT API */

	virtual RID environment_allocate() = 0;
	virtual void environment_initialize(RID p_rid) = 0;

	virtual bool is_environment(RID p_environment) const = 0; // 确保rid是environment

	// Background
	virtual void environment_set_background(RID p_env, RS::EnvironmentBG p_bg) = 0;
	virtual void environment_set_sky(RID p_env, RID p_sky) = 0;
	virtual void environment_set_sky_custom_fov(RID p_env, float p_scale) = 0;
	virtual void environment_set_sky_orientation(RID p_env, const Basis &p_orientation) = 0;
	virtual void environment_set_bg_color(RID p_env, const Color &p_color) = 0;
	virtual void environment_set_bg_energy(RID p_env, float p_multiplier, float p_exposure_value) = 0;
	virtual void environment_set_canvas_max_layer(RID p_env, int p_max_layer) = 0;
	virtual void environment_set_ambient_light(RID p_env, const Color &p_color, RS::EnvironmentAmbientSource p_ambient = RS::ENV_AMBIENT_SOURCE_BG, float p_energy = 1.0, float p_sky_contribution = 0.0, RS::EnvironmentReflectionSource p_reflection_source = RS::ENV_REFLECTION_SOURCE_BG) = 0;

	virtual void environment_set_ssr(RID p_env, bool p_enable, int p_max_steps, float p_fade_int, float p_fade_out, float p_depth_tolerance) = 0;

	virtual bool environment_get_ssr_enabled(RID p_env) const = 0;
	virtual int environment_get_ssr_max_steps(RID p_env) const = 0;
	virtual float environment_get_ssr_fade_in(RID p_env) const = 0;
	virtual float environment_get_ssr_fade_out(RID p_env) const = 0;
	virtual float environment_get_ssr_depth_tolerance(RID p_env) const = 0;

	virtual void environment_set_ssr_roughness_quality(RS::EnvironmentSSRRoughnessQuality p_quality) = 0;

	// SSAO
	virtual void environment_set_ssao(RID p_env, bool p_enable, float p_radius, float p_intensity, float p_power, float p_detail, float p_horizon, float p_sharpness, float p_light_affect, float p_ao_channel_affect) = 0;

	virtual bool environment_get_ssao_enabled(RID p_env) const = 0;
	virtual float environment_get_ssao_radius(RID p_env) const = 0;
	virtual float environment_get_ssao_intensity(RID p_env) const = 0;
	virtual float environment_get_ssao_power(RID p_env) const = 0;
	virtual float environment_get_ssao_detail(RID p_env) const = 0;
	virtual float environment_get_ssao_horizon(RID p_env) const = 0;
	virtual float environment_get_ssao_sharpness(RID p_env) const = 0;
	virtual float environment_get_ssao_direct_light_affect(RID p_env) const = 0;
	virtual float environment_get_ssao_ao_channel_affect(RID p_env) const = 0;

	virtual void environment_set_ssao_quality(RS::EnvironmentSSAOQuality p_quality, bool p_half_size, float p_adaptive_target, int p_blur_passes, float p_fadeout_from, float p_fadeout_to) = 0;


	virtual RS::EnvironmentBG environment_get_background(RID p_Env) const = 0;
	virtual int environment_get_canvas_max_layer(RID p_env) const = 0;

	/* SKY API */

	virtual RID sky_allocate() = 0;
	virtual void sky_initialize(RID p_rid) = 0;

	virtual void sky_set_radiance_size(RID p_sky, int p_radiance_size) = 0;
	virtual void sky_set_mode(RID p_sky, RS::SkyMode p_samples) = 0;
	virtual void sky_set_material(RID p_sky, RID p_material) = 0;
	virtual Ref<Image> sky_bake_panorama(RID p_sky, float p_energy, bool p_bake_irradiance, const Size2i &p_size) = 0;



		/* COMPOSITOR EFFECT API */

	virtual RID compositor_effect_allocate() = 0;
	virtual void compositor_effect_initialize(RID p_rid) = 0;
	/* COMPOSITOR API */

	virtual RID compositor_allocate() = 0;
	virtual void compositor_initialize(RID p_rid) = 0;

	/* Render Buffers */

	virtual Ref<RenderSceneBuffers> render_buffers_create() = 0;
	// virtual void render_empty_scene(const Ref<RenderSceneBuffers> &p_render_buffers, RID p_scenario, RID p_shadow_atlas) = 0;
	virtual void update() = 0;

	virtual void render_camera(const Ref<RenderSceneBuffers> &p_render_buffers, RID p_camera, RID p_scenario, RID p_viewport, Size2 p_viewport_size, uint32_t p_jitter_phase_count, float p_screen_mesh_lod_threshold, RID p_shadow_atlas, RenderInfo *r_render_info) = 0;
	virtual void render_probes() = 0;
	// virtual void update_visibility_notifiers() = 0;

	virtual bool free(RID p_rid) = 0;
};
}
#endif // RENDERING_METHOD_API_H