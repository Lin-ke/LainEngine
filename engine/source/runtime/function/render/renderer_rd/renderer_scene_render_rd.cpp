#include "renderer_scene_render_rd.h"
#include "storage/light_storage.h"
#include "storage/texture_storage.h"
#include "storage/render_scene_buffers_rd.h"
using namespace lain::RendererRD;
using namespace lain;



uint64_t RendererSceneRenderRD::get_scene_pass() {
  return 0;
}

void RendererSceneRenderRD::render_scene(const Ref<RenderSceneBuffers>& p_render_buffers, const CameraData* p_camera_data, const CameraData* p_prev_camera_data,
                                         const PagedArray<RenderGeometryInstance*>& p_instances, const PagedArray<RID>& p_lights, const PagedArray<RID>& p_reflection_probes,
                                         const PagedArray<RID>& p_voxel_gi_instances, const PagedArray<RID>& p_decals, const PagedArray<RID>& p_lightmaps,
                                         const PagedArray<RID>& p_fog_volumes, RID p_environment, RID p_camera_attributes, RID p_compositor, RID p_shadow_atlas,
                                         RID p_occluder_debug_tex, RID p_reflection_atlas, RID p_reflection_probe, int p_reflection_probe_pass,
                                         float p_screen_mesh_lod_threshold, const RenderShadowData* p_render_shadows, int p_render_shadow_count,
                                         const RenderSDFGIData* p_render_sdfgi_regions, int p_render_sdfgi_region_count, const RenderSDFGIUpdateData* p_sdfgi_update_data,
                                         RenderingMethod::RenderInfo* r_render_info) {
  RendererRD::LightStorage* light_storage = RendererRD::LightStorage::get_singleton();
  RendererRD::TextureStorage* texture_storage = RendererRD::TextureStorage::get_singleton();

  // getting this here now so we can direct call a bunch of things more easily
  ERR_FAIL_COND(p_render_buffers.is_null());
  Ref<RenderSceneBuffersRD> rb = p_render_buffers;
  ERR_FAIL_COND(rb.is_null());
}

void RendererSceneRenderRD::update() {
  // update dirty sky
  // @todo
}
