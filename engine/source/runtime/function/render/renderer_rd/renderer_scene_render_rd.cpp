#include "renderer_scene_render_rd.h"
#include "storage/light_storage.h"
#include "storage/render_scene_buffers_rd.h"
#include "storage/render_scene_data_rd.h"
#include "storage/texture_storage.h"
#include "storage/render_data_rd.h"
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

  // setup scene data
  RenderSceneDataRD scene_data;
  // scene data是关于 相机、taa、time、shadow_atlas、lod_multiplier、screen_mesh_lod_threshold等信息的

  {
    // Our first camera is used by default
    scene_data.cam_transform = p_camera_data->main_transform;
    scene_data.cam_projection = p_camera_data->main_projection;
    scene_data.cam_orthogonal = p_camera_data->is_orthogonal;
    scene_data.camera_visible_layers = p_camera_data->visible_layers;
    scene_data.taa_jitter = p_camera_data->taa_jitter;
    scene_data.main_cam_transform = p_camera_data->main_transform;
    scene_data.flip_y = !p_reflection_probe.is_valid();

    scene_data.view_count = p_camera_data->view_count;
    for (uint32_t v = 0; v < p_camera_data->view_count; v++) {
      scene_data.view_eye_offset[v] = p_camera_data->view_offset[v].origin;
      scene_data.view_projection[v] = p_camera_data->view_projection[v];
    }

    scene_data.prev_cam_transform = p_prev_camera_data->main_transform;
    scene_data.prev_cam_projection = p_prev_camera_data->main_projection;
    scene_data.prev_taa_jitter = p_prev_camera_data->taa_jitter;
  //  填充多个view的数据
    for (uint32_t v = 0; v < p_camera_data->view_count; v++) {
      scene_data.prev_view_projection[v] = p_prev_camera_data->view_projection[v];

      scene_data.z_near = p_camera_data->main_projection.get_z_near();
      scene_data.z_far = p_camera_data->main_projection.get_z_far();

      // this should be the same for all cameras..
      const float lod_distance_multiplier = p_camera_data->main_projection.get_lod_multiplier();

      // Also, take into account resolution scaling for the multiplier, since we have more leeway with quality
      // degradation visibility. Conversely, allow upwards scaling, too, for increased mesh detail at high res.
      const float scaling_3d_scale = GLOBAL_GET("rendering/scaling_3d/scale");
      scene_data.lod_distance_multiplier = lod_distance_multiplier * (1.0 / scaling_3d_scale);

      if (get_debug_draw_mode() == RS::VIEWPORT_DEBUG_DRAW_DISABLE_LOD) {
        scene_data.screen_mesh_lod_threshold = 0.0;
      } else {
        scene_data.screen_mesh_lod_threshold = p_screen_mesh_lod_threshold;
      }

      if (p_shadow_atlas.is_valid()) {
        int shadow_atlas_size = light_storage->shadow_atlas_get_size(p_shadow_atlas);
        scene_data.shadow_atlas_pixel_size.x = 1.0 / shadow_atlas_size;
        scene_data.shadow_atlas_pixel_size.y = 1.0 / shadow_atlas_size;
      }
      {
        int directional_shadow_size = light_storage->directional_shadow_get_size();  // 这几个方法为啥不在API里写
        scene_data.directional_shadow_pixel_size.x = 1.0 / directional_shadow_size;
        scene_data.directional_shadow_pixel_size.y = 1.0 / directional_shadow_size;
      }

      scene_data.time = time;
      scene_data.time_step = time_step;
    }
  }
  
  RenderDataRD render_data;
  // renderdata 则 包含了所有的信息，包括scene_data, render_buffers, instances, lights, reflection_probes, voxel_gi_instances, decals, lightmaps, fog_volumes, environment, camera_attributes, compositor, shadow_atlas, occluder_debug_tex, reflection_atlas, reflection_probe, reflection_probe_pass, render_shadows, render_shadow_count, render_sdfgi_regions, render_sdfgi_region_count, sdfgi_update_data, r_render_info
	{
		render_data.render_buffers = rb;
		render_data.scene_data = &scene_data;

		render_data.instances = &p_instances;
		render_data.lights = &p_lights;
		render_data.reflection_probes = &p_reflection_probes;
		render_data.voxel_gi_instances = &p_voxel_gi_instances;
		render_data.decals = &p_decals;
		render_data.lightmaps = &p_lightmaps;
		render_data.fog_volumes = &p_fog_volumes;
		render_data.environment = p_environment;
		render_data.compositor = p_compositor;
		render_data.camera_attributes = p_camera_attributes;
		render_data.shadow_atlas = p_shadow_atlas;
		render_data.occluder_debug_tex = p_occluder_debug_tex;
		render_data.reflection_atlas = p_reflection_atlas;
		render_data.reflection_probe = p_reflection_probe;
		render_data.reflection_probe_pass = p_reflection_probe_pass;

		render_data.render_shadows = p_render_shadows;
		render_data.render_shadow_count = p_render_shadow_count;
		render_data.render_sdfgi_regions = p_render_sdfgi_regions;
		render_data.render_sdfgi_region_count = p_render_sdfgi_region_count;
		render_data.sdfgi_update_data = p_sdfgi_update_data;

		render_data.render_info = r_render_info;

		if (p_render_buffers.is_valid() && p_reflection_probe.is_null()) {
			render_data.transparent_bg = texture_storage->render_target_get_transparent(rb->get_render_target());
		}
	}
  PagedArray<RID> empty;

	if (get_debug_draw_mode() == RS::VIEWPORT_DEBUG_DRAW_UNSHADED || get_debug_draw_mode() == RS::VIEWPORT_DEBUG_DRAW_OVERDRAW) {
		render_data.lights = &empty;
		render_data.reflection_probes = &empty;
		render_data.voxel_gi_instances = &empty;
		render_data.lightmaps = &empty;
	}

	if (get_debug_draw_mode() == RS::VIEWPORT_DEBUG_DRAW_UNSHADED ||
			get_debug_draw_mode() == RS::VIEWPORT_DEBUG_DRAW_OVERDRAW ||
			get_debug_draw_mode() == RS::VIEWPORT_DEBUG_DRAW_LIGHTING ||
			get_debug_draw_mode() == RS::VIEWPORT_DEBUG_DRAW_PSSM_SPLITS) {
		render_data.decals = &empty;
	}

	Color clear_color;
	if (p_render_buffers.is_valid() && p_reflection_probe.is_null()) {
		clear_color = texture_storage->render_target_get_clear_request_color(rb->get_render_target());
	} else {
		clear_color = RSG::texture_storage->get_default_clear_color();
	}

	//calls _pre_opaque_render between depth pre-pass and opaque pass
	_render_scene(&render_data, clear_color);

}

void RendererSceneRenderRD::update() {
  // update dirty sky
  // @todo
}
