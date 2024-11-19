#ifndef RENDERER_SCENE_RENDER_RD_H
#define RENDERER_SCENE_RENDER_RD_H
#include "function/render/scene/renderer_scene_renderer_api.h"
#include "storage/material_storage.h"
#include "storage/render_data_rd.h"
#include "storage/render_scene_buffers_rd.h"
#include "storage/forward_id.h"
#include "environment/renderer_sky.h"
#include "environment/renderer_gi.h"
#include "effects/copy_effects.h"
#include "effects/resolve.h"
namespace lain {
class RendererSceneRenderRD : public RendererSceneRender {
  static RendererSceneRenderRD* singleton;

protected:
	RendererRD::ForwardIDStorage *forward_id_storage = nullptr;
	double time = 0.0;
	double time_step = 0.0;
	bool use_physical_light_units = false;
	RendererRD::SkyRD sky;
	RendererRD::GI gi;
	RendererRD::CopyEffects* copy_effects; 
	RendererRD::Resolve* resolve_effects;
	
 public:
  static RendererSceneRenderRD* get_singleton() { return singleton; }
  virtual RendererRD::ForwardIDStorage *create_forward_id_storage() { return memnew(RendererRD::ForwardIDStorage); };

  RendererSceneRenderRD();
  ~RendererSceneRenderRD();
  void cull_scene();
  uint64_t get_scene_pass();
  // 这个函数处理渲染数据，真正渲染在_render_scene 的实现中。
  // -> render_forward_clustered.h
  // 提供实现的渲染API
  virtual void render_scene(const Ref<RenderSceneBuffers>& p_render_buffers, const CameraData* p_camera_data, const CameraData* p_prev_camera_data,
                            const PagedArray<RenderGeometryInstance*>& p_instances, const PagedArray<RID>& p_lights, const PagedArray<RID>& p_reflection_probes,
                            const PagedArray<RID>& p_voxel_gi_instances, const PagedArray<RID>& p_decals, const PagedArray<RID>& p_lightmaps,
                            const PagedArray<RID>& p_fog_volumes, RID p_environment, RID p_camera_attributes, RID p_compositor, RID p_shadow_atlas, RID p_occluder_debug_tex,
                            RID p_reflection_atlas, RID p_reflection_probe, int p_reflection_probe_pass, float p_screen_mesh_lod_threshold,
                            const RenderShadowData* p_render_shadows, int p_render_shadow_count, const RenderSDFGIData* p_render_sdfgi_regions,
                            int p_render_sdfgi_region_count, const RenderSDFGIUpdateData* p_sdfgi_update_data = nullptr,
                            RenderingMethod::RenderInfo* r_render_info = nullptr) override;

	virtual Ref<RenderSceneBuffers> render_buffers_create() override;
	virtual bool _render_buffers_can_be_storage() { return true;}
  virtual RD::DataFormat _render_buffers_get_color_format() const {return RD::DATA_FORMAT_R16G16B16_SFLOAT;}
  /* 提供 compositor 的渲染相关*/
void _process_compositor_effects(RS::CompositorEffectCallbackType p_callback_type, const RenderDataRD *p_render_data);


  virtual void set_scene_pass(uint64_t p_pass) override { scene_pass = p_pass; }
  virtual void update() override;

	void init(); // 调用一些init，填充从projectsettings 中获取的一些参数
	virtual bool free(RID p_rid) override;
  // 后端特定的渲染API
	virtual void setup_render_buffer_data(Ref<RenderSceneBuffersRD> p_render_buffers) = 0;
	virtual void _render_scene(RenderDataRD *p_render_data, const Color &p_default_color) = 0;
	virtual void _update_shader_quality_settings() {}
	virtual RID _render_buffers_get_normal_texture(Ref<RenderSceneBuffersRD> p_render_buffers) = 0;
	virtual RID _render_buffers_get_velocity_texture(Ref<RenderSceneBuffersRD> p_render_buffers) = 0;
	// virtual void _render_buffers_debug_draw(const RenderDataRD *p_render_data);
	// virtual void _render_material(const Transform3D &p_cam_transform, const Projection &p_cam_projection, bool p_cam_orthogonal, const PagedArray<RenderGeometryInstance *> &p_instances, RID p_framebuffer, const Rect2i &p_region, float p_exposure_normalization) = 0;
	// virtual void _render_uv2(const PagedArray<RenderGeometryInstance *> &p_instances, RID p_framebuffer, const Rect2i &p_region) = 0;
	// virtual void _render_sdfgi(Ref<RenderSceneBuffersRD> p_render_buffers, const Vector3i &p_from, const Vector3i &p_size, const AABB &p_bounds, const PagedArray<RenderGeometryInstance *> &p_instances, const RID &p_albedo_texture, const RID &p_emission_texture, const RID &p_emission_aniso_texture, const RID &p_geom_facing_texture, float p_exposure_normalization) = 0;
	// virtual void _render_particle_collider_heightfield(RID p_fb, const Transform3D &p_cam_transform, const Projection &p_cam_projection, const PagedArray<RenderGeometryInstance *> &p_instances) = 0;
	virtual void base_uniforms_changed() = 0;


  /* RENDER BUFFERS */
	virtual float _render_buffers_get_luminance_multiplier(){return 1.0f;}
	// 跟获得特定纹理相关的
	RID render_buffers_get_default_voxel_gi_buffer();

  /* GI */
  bool screen_space_roughness_limiter = false;
  float screen_space_roughness_limiter_amount = 0.25;
  float screen_space_roughness_limiter_limit = 0.18;

  bool screen_space_roughness_limiter_is_active() const { return screen_space_roughness_limiter; }

  float screen_space_roughness_limiter_get_amount() const { return screen_space_roughness_limiter_amount; }

  float screen_space_roughness_limiter_get_limit() const { return screen_space_roughness_limiter_limit; }

/* Light data */

	uint64_t scene_pass = 0;

	uint32_t max_cluster_elements = 512;

	RS::ViewportDebugDraw debug_draw = RS::VIEWPORT_DEBUG_DRAW_DISABLED;
	_FORCE_INLINE_ RS::ViewportDebugDraw get_debug_draw_mode() const {
		return debug_draw;
	}


	public:
	RendererRD::SkyRD *get_sky() { return &sky; }
	int get_roughness_layers() const{
		return sky.roughness_layers;
	}
	bool is_using_radiance_cubemap_array(){
		return sky.sky_use_cubemap_array;
	}

  void set_time(double p_time, double p_frame_step) { time = p_time;
  time_step = p_frame_step; }
	bool _compositor_effects_has_flag(const RenderDataRD *p_render_data, RS::CompositorEffectFlags p_flag, RS::CompositorEffectCallbackType p_callback_type = RS::COMPOSITOR_EFFECT_CALLBACK_TYPE_ANY);

	// * getters *
	private:
			/* Shadow atlas */
	RS::ShadowQuality shadows_quality = RS::SHADOW_QUALITY_MAX; //So it always updates when first set
	RS::ShadowQuality directional_shadow_quality = RS::SHADOW_QUALITY_MAX;
	float shadows_quality_radius = 1.0;
	float directional_shadow_quality_radius = 1.0;

	float *directional_penumbra_shadow_kernel = nullptr;
	float *directional_soft_shadow_kernel = nullptr;
	float *penumbra_shadow_kernel = nullptr;
	float *soft_shadow_kernel = nullptr;
	int directional_penumbra_shadow_samples = 0;
	int directional_soft_shadow_samples = 0;
	int penumbra_shadow_samples = 0;
	int soft_shadow_samples = 0;
	RS::DecalFilter decals_filter = RS::DECAL_FILTER_LINEAR_MIPMAPS;
	RS::LightProjectorFilter light_projectors_filter = RS::LIGHT_PROJECTOR_FILTER_LINEAR_MIPMAPS;
	public:

	_FORCE_INLINE_ RS::ShadowQuality shadows_quality_get() const {
		return shadows_quality;
	}
	_FORCE_INLINE_ RS::ShadowQuality directional_shadow_quality_get() const {
		return directional_shadow_quality;
	}
	_FORCE_INLINE_ float shadows_quality_radius_get() const {
		return shadows_quality_radius;
	}
	_FORCE_INLINE_ float directional_shadow_quality_radius_get() const {
		return directional_shadow_quality_radius;
	}

	_FORCE_INLINE_ float *directional_penumbra_shadow_kernel_get() {
		return directional_penumbra_shadow_kernel;
	}
	_FORCE_INLINE_ float *directional_soft_shadow_kernel_get() {
		return directional_soft_shadow_kernel;
	}
	_FORCE_INLINE_ float *penumbra_shadow_kernel_get() {
		return penumbra_shadow_kernel;
	}
	_FORCE_INLINE_ float *soft_shadow_kernel_get() {
		return soft_shadow_kernel;
	}

	_FORCE_INLINE_ int directional_penumbra_shadow_samples_get() const {
		return directional_penumbra_shadow_samples;
	}
	_FORCE_INLINE_ int directional_soft_shadow_samples_get() const {
		return directional_soft_shadow_samples;
	}
	_FORCE_INLINE_ int penumbra_shadow_samples_get() const {
		return penumbra_shadow_samples;
	}
	_FORCE_INLINE_ int soft_shadow_samples_get() const {
		return soft_shadow_samples;
	}

	_FORCE_INLINE_ RS::LightProjectorFilter light_projectors_get_filter() const {
		return light_projectors_filter;
	}
	_FORCE_INLINE_ RS::DecalFilter decals_get_filter() const {
		return decals_filter;
	}

};
}  // namespace lain

#endif