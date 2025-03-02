#ifndef RENDERER_SCENE_RENDER_H
#define RENDERER_SCENE_RENDER_H
#include "core/templates/paged_array.h"
#include "function/render/rendering_system/environment_storage.h"
#include "function/render/rendering_system/compositor_storage.h"

#include "function/render/scene/render_scene_buffers_api.h"
#include "renderer_geometry_instance_api.h"
#include "function/render/rendering_system/rendering_method_api.h"
namespace lain {
class RendererSceneRender {
  // Scene Renderer 主要控制什么呢：
  // 1. 场景的渲染，开放render_scene API，其实现依赖 _rd 的 render_scene 以及在Implementation命名空间里的函数
  // 2. Enviroment 的管理 （包括 Fog， Tone Map， Sky， 屏幕空间效果，...)
  // 3. GeometryInstance 的创建和释放
  // 4. SDFGI的更新相关 (voxel GI)
  // 5. COMPOSITOR 的管理
  // 依赖注入
 private:
  RendererEnvironmentStorage environment_storage;
  RendererCompositorStorage compositor_storage; 

 public:
 
	/* Geometry Instance */
  // geometry instance通过 forward_clustered 等 
	virtual RenderGeometryInstance *geometry_instance_create(RID p_base) = 0;
	virtual void geometry_instance_free(RenderGeometryInstance *p_geometry_instance) = 0;
	virtual uint32_t geometry_instance_get_pair_mask() = 0;


	virtual bool free(RID p_rid) = 0;

  enum {
    MAX_DIRECTIONAL_LIGHTS = 8,  // 8有向光，层级4级别
    MAX_DIRECTIONAL_LIGHT_CASCADES = 4,
    MAX_RENDER_VIEWS = 2
  };

	virtual RID voxel_gi_instance_create(RID p_voxel_gi) = 0;
	virtual void voxel_gi_instance_set_transform_to_data(RID p_probe, const Transform3D &p_xform) = 0;
	virtual bool voxel_gi_needs_update(RID p_probe) const = 0;
	virtual void voxel_gi_update(RID p_probe, bool p_update_light_instances, const Vector<RID> &p_light_instances, const PagedArray<RenderGeometryInstance *> &p_dynamic_objects) = 0;

	virtual void voxel_gi_set_quality(RS::VoxelGIQuality) = 0;
	
  struct RenderShadowData {
    RID light;
    int pass = 0;                                   // 例如cubemap记录六个面
    PagedArray<RenderGeometryInstance*> instances;  // 要绘制的geometry
  };

  struct RenderSDFGIData {
    int region = 0;
    PagedArray<RenderGeometryInstance*> instances;
  };

  /* ENVIRONMENT API */
  RID environment_allocate();
  void environment_initialize(RID p_rid);
  void environment_free(RID p_rid);

  bool is_environment(RID p_env) const;
  bool environment_get_fog_enabled(RID p_env) const;
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



  /* COMPOSITOR API*/
  RID compositor_allocate();
  void compositor_initialize(RID p_rid);
  void compositor_free(RID p_rid);

  bool is_compositor(RID p_compositor) const;

  	/* COMPOSITOR EFFECT API */

	RID compositor_effect_allocate();
	void compositor_effect_initialize(RID p_rid);
	void compositor_effect_free(RID p_rid);

	bool is_compositor_effect(RID p_compositor) const;
	void compositor_effect_set_enabled(RID p_compositor, bool p_enabled);
	void compositor_effect_set_callback(RID p_compositor, RS::CompositorEffectCallbackType p_callback_type, const Callable &p_callback);
	void compositor_effect_set_flag(RID p_compositor, RS::CompositorEffectFlags p_flag, bool p_set);
  

  virtual Ref<RenderSceneBuffers> render_buffers_create() = 0;

  virtual void update() = 0;

  struct CameraData {
    // flags
    uint32_t view_count;
    bool is_orthogonal;
    uint32_t visible_layers;
    bool vaspect;
    // OpenGL风格，摄像机位于原点，与z轴方向相反
    // transform包括旋转(basis) 和 位移(origin)
    // projection是相机的投影矩阵

    // Main/center projection
    Transform3D main_transform;
    Projection main_projection;

    Transform3D view_offset[RendererSceneRender::MAX_RENDER_VIEWS];
    Projection view_projection[RendererSceneRender::MAX_RENDER_VIEWS];
    Vector2 taa_jitter;

    void set_camera(const Transform3D p_transform, const Projection p_projection, bool p_is_orthogonal, bool p_vaspect, const Vector2& p_taa_jitter = Vector2(),
                    uint32_t p_visible_layers = 0xFFFFFFFF);
    void set_multiview_camera(uint32_t p_view_count, const Transform3D* p_transforms, const Projection* p_projections, bool p_is_orthogonal, bool p_vaspect){
			
		}
	};

	struct RenderSDFGIUpdateData{

	};

  virtual void set_scene_pass(uint64_t p_pass) = 0;
  virtual void render_scene(const Ref<RenderSceneBuffers>& p_render_buffers, const CameraData* p_camera_data, const CameraData* p_prev_camera_data,
                            const PagedArray<RenderGeometryInstance*>& p_instances, const PagedArray<RID>& p_lights, const PagedArray<RID>& p_reflection_probes,
                            const PagedArray<RID>& p_voxel_gi_instances, const PagedArray<RID>& p_decals, const PagedArray<RID>& p_lightmaps,
                            const PagedArray<RID>& p_fog_volumes, RID p_environment, RID p_camera_attributes, RID p_compositor, RID p_shadow_atlas, RID p_occluder_debug_tex,
                            RID p_reflection_atlas, RID p_reflection_probe, int p_reflection_probe_pass, float p_screen_mesh_lod_threshold,
                            const RenderShadowData* p_render_shadows, int p_render_shadow_count, const RenderSDFGIData* p_render_sdfgi_regions,
                            int p_render_sdfgi_region_count, const RenderSDFGIUpdateData* p_sdfgi_update_data = nullptr,
                            RenderingMethod::RenderInfo* r_render_info = nullptr) = 0;
  // 这几个API最后在 render_forward_clustered.h中实现

/* SKY */
	virtual RID sky_allocate() = 0;
	virtual void sky_initialize(RID p_rid) = 0;

	virtual void sky_set_radiance_size(RID p_sky, int p_radiance_size) = 0;
	virtual void sky_set_mode(RID p_sky, RS::SkyMode p_samples) = 0;
	virtual void sky_set_material(RID p_sky, RID p_material) = 0;
	virtual Ref<Image> sky_bake_panorama(RID p_sky, float p_energy, bool p_bake_irradiance, const Size2i &p_size) = 0;


/* Environment*/
	// Background
	void environment_set_background(RID p_env, RS::EnvironmentBG p_bg);
	void environment_set_sky(RID p_env, RID p_sky);
	void environment_set_sky_custom_fov(RID p_env, float p_scale);
	void environment_set_sky_orientation(RID p_env, const Basis &p_orientation);
	void environment_set_bg_color(RID p_env, const Color &p_color);
	void environment_set_bg_energy(RID p_env, float p_multiplier, float p_exposure_value);
	void environment_set_canvas_max_layer(RID p_env, int p_max_layer);
	void environment_set_ambient_light(RID p_env, const Color &p_color, RS::EnvironmentAmbientSource p_ambient = RS::ENV_AMBIENT_SOURCE_BG, float p_energy = 1.0, float p_sky_contribution = 0.0, RS::EnvironmentReflectionSource p_reflection_source = RS::ENV_REFLECTION_SOURCE_BG);

  
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

	virtual void environment_set_ssao_quality(RS::EnvironmentSSAOQuality p_quality, bool p_half_size, float p_adaptive_target, int p_blur_passes, float p_fadeout_from, float p_fadeout_to) = 0;

	// SSIL
	void environment_set_ssil(RID p_env, bool p_enable, float p_radius, float p_intensity, float p_sharpness, float p_normal_rejection);
	bool environment_get_ssil_enabled(RID p_env) const;
	float environment_get_ssil_radius(RID p_env) const;
	float environment_get_ssil_intensity(RID p_env) const;
	float environment_get_ssil_sharpness(RID p_env) const;
	float environment_get_ssil_normal_rejection(RID p_env) const;

	// virtual void environment_set_ssil_quality(RS::EnvironmentSSILQuality p_quality, bool p_half_size, float p_adaptive_target, int p_blur_passes, float p_fadeout_from, float p_fadeout_to) = 0;
	// SSR
	void environment_set_ssr(RID p_env, bool p_enable, int p_max_steps, float p_fade_int, float p_fade_out, float p_depth_tolerance);
	bool environment_get_ssr_enabled(RID p_env) const;
	int environment_get_ssr_max_steps(RID p_env) const;
	float environment_get_ssr_fade_in(RID p_env) const;
	float environment_get_ssr_fade_out(RID p_env) const;
	float environment_get_ssr_depth_tolerance(RID p_env) const;

	virtual void environment_set_ssr_roughness_quality(RS::EnvironmentSSRRoughnessQuality p_quality) = 0;

  // virtual void sub_surface_scattering_set_quality(RS::SubSurfaceScatteringQuality p_quality) = 0;
	// virtual void sub_surface_scattering_set_scale(float p_scale, float p_depth_scale) = 0;
};
}  // namespace lain
#endif