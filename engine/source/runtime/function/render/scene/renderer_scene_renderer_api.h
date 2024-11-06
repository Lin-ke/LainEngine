#ifndef RENDERER_SCENE_RENDER_H
#define RENDERER_SCENE_RENDER_H
#include "core/templates/paged_array.h"
#include "function/render/rendering_system/environment_storage.h"
#include "function/render/scene/render_scene_buffers_api.h"
#include "renderer_geometry_instance_api.h"
#include "function/render/rendering_system/rendering_method_api.h"
namespace lain {
class RendererSceneRender {
 private:
  RendererEnvironmentStorage environment_storage;
  // RendererCompositorStorage compositor_storage;

 public:
  enum {
    MAX_DIRECTIONAL_LIGHTS = 8,  // 8有向光，层级4级别
    MAX_DIRECTIONAL_LIGHT_CASCADES = 4,
    MAX_RENDER_VIEWS = 2
  };
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

  // 虚接口
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
};
}  // namespace lain
#endif