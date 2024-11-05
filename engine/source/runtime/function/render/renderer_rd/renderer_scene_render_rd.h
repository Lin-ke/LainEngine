#include "function/render/scene/renderer_scene_renderer_api.h"
#include "storage/material_storage.h"
namespace lain::RendererRD {
// Implementaion 3D scene rendering using Rendering Device
class RendererSceneRenderRD : public RendererSceneRender {
  static RendererSceneRenderRD* singleton;

 public:
  static RendererSceneRenderRD* get_singleton() { return singleton; }
  RendererSceneRenderRD();
  ~RendererSceneRenderRD();
  void cull_scene();
  uint64_t get_scene_pass();
  virtual void render_scene(const Ref<RenderSceneBuffers>& p_render_buffers, const CameraData* p_camera_data, const CameraData* p_prev_camera_data,
                            const PagedArray<RenderGeometryInstance*>& p_instances, const PagedArray<RID>& p_lights, const PagedArray<RID>& p_reflection_probes,
                            const PagedArray<RID>& p_voxel_gi_instances, const PagedArray<RID>& p_decals, const PagedArray<RID>& p_lightmaps,
                            const PagedArray<RID>& p_fog_volumes, RID p_environment, RID p_camera_attributes, RID p_compositor, RID p_shadow_atlas, RID p_occluder_debug_tex,
                            RID p_reflection_atlas, RID p_reflection_probe, int p_reflection_probe_pass, float p_screen_mesh_lod_threshold,
                            const RenderShadowData* p_render_shadows, int p_render_shadow_count, const RenderSDFGIData* p_render_sdfgi_regions,
                            int p_render_sdfgi_region_count, const RenderSDFGIUpdateData* p_sdfgi_update_data = nullptr,
                            RenderingMethod::RenderInfo* r_render_info = nullptr) override;

  virtual void update() override;
};
}  // namespace lain::RendererRD
