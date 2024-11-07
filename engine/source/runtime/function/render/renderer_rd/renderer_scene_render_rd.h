#include "function/render/scene/renderer_scene_renderer_api.h"
#include "storage/material_storage.h"
#include "storage/render_data_rd.h"
#include "storage/render_scene_buffers_rd.h"
namespace lain {
class RendererSceneRenderRD : public RendererSceneRender {
  static RendererSceneRenderRD* singleton;
	double time = 0.0;
	double time_step = 0.0;
 public:
  static RendererSceneRenderRD* get_singleton() { return singleton; }
  RendererSceneRenderRD();
  ~RendererSceneRenderRD();
  void cull_scene();
  uint64_t get_scene_pass();
  // 这个函数处理渲染数据，真正渲染在_render_scene 的实现中。
  // -> render_forward_clustered.h
  virtual void render_scene(const Ref<RenderSceneBuffers>& p_render_buffers, const CameraData* p_camera_data, const CameraData* p_prev_camera_data,
                            const PagedArray<RenderGeometryInstance*>& p_instances, const PagedArray<RID>& p_lights, const PagedArray<RID>& p_reflection_probes,
                            const PagedArray<RID>& p_voxel_gi_instances, const PagedArray<RID>& p_decals, const PagedArray<RID>& p_lightmaps,
                            const PagedArray<RID>& p_fog_volumes, RID p_environment, RID p_camera_attributes, RID p_compositor, RID p_shadow_atlas, RID p_occluder_debug_tex,
                            RID p_reflection_atlas, RID p_reflection_probe, int p_reflection_probe_pass, float p_screen_mesh_lod_threshold,
                            const RenderShadowData* p_render_shadows, int p_render_shadow_count, const RenderSDFGIData* p_render_sdfgi_regions,
                            int p_render_sdfgi_region_count, const RenderSDFGIUpdateData* p_sdfgi_update_data = nullptr,
                            RenderingMethod::RenderInfo* r_render_info = nullptr) override;

  virtual void update() override;
  // 后端特定的渲染API
	virtual void setup_render_buffer_data(Ref<RenderSceneBuffersRD> p_render_buffers) = 0;
	virtual void _render_scene(RenderDataRD *p_render_data, const Color &p_default_color) = 0;
	virtual void _update_shader_quality_settings() {}
	virtual RID _render_buffers_get_normal_texture(Ref<RenderSceneBuffersRD> p_render_buffers) = 0;
	virtual RID _render_buffers_get_velocity_texture(Ref<RenderSceneBuffersRD> p_render_buffers) = 0;
	virtual void _render_buffers_debug_draw(const RenderDataRD *p_render_data);
	virtual void _render_material(const Transform3D &p_cam_transform, const Projection &p_cam_projection, bool p_cam_orthogonal, const PagedArray<RenderGeometryInstance *> &p_instances, RID p_framebuffer, const Rect2i &p_region, float p_exposure_normalization) = 0;
	virtual void _render_uv2(const PagedArray<RenderGeometryInstance *> &p_instances, RID p_framebuffer, const Rect2i &p_region) = 0;
	virtual void _render_sdfgi(Ref<RenderSceneBuffersRD> p_render_buffers, const Vector3i &p_from, const Vector3i &p_size, const AABB &p_bounds, const PagedArray<RenderGeometryInstance *> &p_instances, const RID &p_albedo_texture, const RID &p_emission_texture, const RID &p_emission_aniso_texture, const RID &p_geom_facing_texture, float p_exposure_normalization) = 0;
	virtual void _render_particle_collider_heightfield(RID p_fb, const Transform3D &p_cam_transform, const Projection &p_cam_projection, const PagedArray<RenderGeometryInstance *> &p_instances) = 0;


  /* RENDER BUFFERS */

  /* GI */
  bool screen_space_roughness_limiter = false;
  float screen_space_roughness_limiter_amount = 0.25;
  float screen_space_roughness_limiter_limit = 0.18;

  bool screen_space_roughness_limiter_is_active() const { return screen_space_roughness_limiter; }

  float screen_space_roughness_limiter_get_amount() const { return screen_space_roughness_limiter_amount; }

  float screen_space_roughness_limiter_get_limit() const { return screen_space_roughness_limiter_limit; }

	RS::ViewportDebugDraw debug_draw = RS::VIEWPORT_DEBUG_DRAW_DISABLED;
	_FORCE_INLINE_ RS::ViewportDebugDraw get_debug_draw_mode() const {
		return debug_draw;
	}

  float *directional_penumbra_shadow_kernel = nullptr;
	float *directional_soft_shadow_kernel = nullptr;
	float *penumbra_shadow_kernel = nullptr;
	float *soft_shadow_kernel = nullptr;
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
  
	RendererRD::SkyRD sky;
	RendererRD::SkyRD *get_sky() { return &sky; }

};
}  // namespace lain
