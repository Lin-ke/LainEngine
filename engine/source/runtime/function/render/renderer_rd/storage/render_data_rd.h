#ifndef RENDER_DATA_RD_H
#define RENDER_DATA_RD_H
#include "function/render/rendering_device/rendering_device.h"
#include "function/render/rendering_system/rendering_system.h"
#include "function/render/scene/render_data_api.h"
#include "function/render/scene/render_scene_data_api.h"
#include "render_scene_buffers_rd.h"
#include "render_scene_data_rd.h"
namespace lain {
class RenderDataRD : public RenderData {
  LCLASS(RenderDataRD, RenderData);

 public:
  Ref<RenderSceneBuffers> get_render_scene_buffers() const override;
  RenderSceneData* get_render_scene_data() const override;
  RID get_environment() const override;

  RID get_camera_attributes() const override;

  // Members are publicly accessible within the render engine.
  Ref<RenderSceneBuffersRD> render_buffers;
  RenderSceneDataRD* scene_data = nullptr;

  const PagedArray<RenderGeometryInstance*>* instances = nullptr;
  const PagedArray<RID>* lights = nullptr;
  const PagedArray<RID>* reflection_probes = nullptr;
  const PagedArray<RID>* voxel_gi_instances = nullptr;
  const PagedArray<RID>* decals = nullptr;
  const PagedArray<RID>* lightmaps = nullptr;
  const PagedArray<RID>* fog_volumes = nullptr;
  RID environment;
  RID camera_attributes;
  RID compositor;
  RID shadow_atlas;
  RID occluder_debug_tex;
  RID reflection_atlas;
  RID reflection_probe;
  int reflection_probe_pass = 0;

  RID cluster_buffer;
  uint32_t cluster_size = 0;
  uint32_t cluster_max_elements = 0;

  uint32_t directional_light_count = 0;
  bool directional_light_soft_shadows = false;

  RenderingMethod::RenderInfo* render_info = nullptr;

  /* Viewport data */
  bool transparent_bg = false;

  /* Shadow data */
  const RendererSceneRender::RenderShadowData* render_shadows = nullptr;
  int render_shadow_count = 0;

  LocalVector<int> cube_shadows;
  LocalVector<int> shadows;
  LocalVector<int> directional_shadows;

  /* GI info */
  const RendererSceneRender::RenderSDFGIData* render_sdfgi_regions = nullptr;
  int render_sdfgi_region_count = 0;
  const RendererSceneRender::RenderSDFGIUpdateData* sdfgi_update_data = nullptr;

  uint32_t voxel_gi_count = 0;
};
}  // namespace lain

#endif
