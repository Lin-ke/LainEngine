#include "renderer_scene_render_rd.h"
#include "storage/framebuffer_cache_rd.h"
#include "storage/light_storage.h"
#include "storage/render_data_rd.h"
#include "storage/render_scene_buffers_rd.h"
#include "storage/render_scene_data_rd.h"
#include "storage/texture_storage.h"
using namespace lain::RendererRD;
using namespace lain;
// p_sample_count max = 32
void get_vogel_disk(float* r_kernel, int p_sample_count) {
  const float golden_angle = 2.4;

  for (int i = 0; i < p_sample_count; i++) {
    float r = Math::sqrt(float(i) + 0.5) / Math::sqrt(float(p_sample_count));
    float theta = float(i) * golden_angle;

    r_kernel[i * 4] = Math::cos(theta) * r;
    r_kernel[i * 4 + 1] = Math::sin(theta) * r;
  }
}

RendererSceneRenderRD* RendererSceneRenderRD::singleton = nullptr;

lain::RendererSceneRenderRD::RendererSceneRenderRD() {
  singleton = this;

  // init is called in scene->init()
  directional_penumbra_shadow_kernel = memnew_arr(float, 129);
  directional_soft_shadow_kernel = memnew_arr(float, 129);
  penumbra_shadow_kernel = memnew_arr(float, 129);
  soft_shadow_kernel = memnew_arr(float, 129);
}

lain::RendererSceneRenderRD::~RendererSceneRenderRD() {
  singleton = nullptr;
  // memdelete
  memdelete_arr(directional_penumbra_shadow_kernel);
  memdelete_arr(directional_soft_shadow_kernel);
  memdelete_arr(penumbra_shadow_kernel);
  memdelete_arr(soft_shadow_kernel);
  RSG::light_storage->directional_shadow_atlas_set_size(0);
  if (forward_id_storage) {
    memdelete(forward_id_storage);
  }
}

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

  if (get_debug_draw_mode() == RS::VIEWPORT_DEBUG_DRAW_UNSHADED || get_debug_draw_mode() == RS::VIEWPORT_DEBUG_DRAW_OVERDRAW ||
      get_debug_draw_mode() == RS::VIEWPORT_DEBUG_DRAW_LIGHTING || get_debug_draw_mode() == RS::VIEWPORT_DEBUG_DRAW_PSSM_SPLITS) {
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

Ref<RenderSceneBuffers> lain::RendererSceneRenderRD::render_buffers_create() {
  Ref<RenderSceneBuffersRD> rb;
  rb.instantiate();

  rb->set_can_be_storage(_render_buffers_can_be_storage());
  rb->set_max_cluster_elements(max_cluster_elements);
  rb->set_base_data_format(_render_buffers_get_color_format());
  // if (vrs) {
  // 	rb->set_vrs(vrs);
  // }

  setup_render_buffer_data(rb);

  return rb;
}

void RendererSceneRenderRD::_process_compositor_effects(RS::CompositorEffectCallbackType p_callback_type, const RenderDataRD* p_render_data) {
  RendererCompositorStorage* comp_storage = RendererCompositorStorage::get_singleton();

  if (p_render_data->compositor.is_null()) {
    return;
  }

  if (p_render_data->reflection_probe.is_valid()) {
    return;
  }

  ERR_FAIL_COND(!comp_storage->is_compositor(p_render_data->compositor));

  Vector<RID> re_rids = comp_storage->compositor_get_compositor_effects(p_render_data->compositor, p_callback_type, true);

  for (RID rid : re_rids) {
    Array arr;
    Callable callback = comp_storage->compositor_effect_get_callback(rid);

    arr.push_back(p_callback_type);
    arr.push_back(p_render_data);

    callback.callv(arr);
  }
}

void RendererSceneRenderRD::update() {
  // update dirty sky
  // @todo
}

void RendererSceneRenderRD::positional_soft_shadow_filter_set_quality(RS::ShadowQuality p_quality) {
  ERR_FAIL_INDEX_MSG(p_quality, RS::SHADOW_QUALITY_MAX, "Shadow quality too high, please see RenderingServer's ShadowQuality enum");

  if (shadows_quality != p_quality) {
    shadows_quality = p_quality;

    switch (shadows_quality) {
      case RS::SHADOW_QUALITY_HARD: {
        penumbra_shadow_samples = 4;
        soft_shadow_samples = 0;
        shadows_quality_radius = 1.0;
      } break;
      case RS::SHADOW_QUALITY_SOFT_VERY_LOW: {
        penumbra_shadow_samples = 4;
        soft_shadow_samples = 1;
        shadows_quality_radius = 1.5;
      } break;
      case RS::SHADOW_QUALITY_SOFT_LOW: {
        penumbra_shadow_samples = 8;
        soft_shadow_samples = 4;
        shadows_quality_radius = 2.0;
      } break;
      case RS::SHADOW_QUALITY_SOFT_MEDIUM: {
        penumbra_shadow_samples = 12;
        soft_shadow_samples = 8;
        shadows_quality_radius = 2.0;
      } break;
      case RS::SHADOW_QUALITY_SOFT_HIGH: {
        penumbra_shadow_samples = 24;
        soft_shadow_samples = 16;
        shadows_quality_radius = 3.0;
      } break;
      case RS::SHADOW_QUALITY_SOFT_ULTRA: {
        penumbra_shadow_samples = 32;
        soft_shadow_samples = 32;
        shadows_quality_radius = 4.0;
      } break;
      case RS::SHADOW_QUALITY_MAX:
        break;
    }
    get_vogel_disk(penumbra_shadow_kernel, penumbra_shadow_samples);
    get_vogel_disk(soft_shadow_kernel, soft_shadow_samples);
  }

  _update_shader_quality_settings();
}
void RendererSceneRenderRD::directional_soft_shadow_filter_set_quality(RS::ShadowQuality p_quality) {
  ERR_FAIL_INDEX_MSG(p_quality, RS::SHADOW_QUALITY_MAX, "Shadow quality too high, please see RenderingServer's ShadowQuality enum");

  if (directional_shadow_quality != p_quality) {
    directional_shadow_quality = p_quality;

    switch (directional_shadow_quality) {
      case RS::SHADOW_QUALITY_HARD: {
        directional_penumbra_shadow_samples = 4;
        directional_soft_shadow_samples = 0;
        directional_shadow_quality_radius = 1.0;
      } break;
      case RS::SHADOW_QUALITY_SOFT_VERY_LOW: {
        directional_penumbra_shadow_samples = 4;
        directional_soft_shadow_samples = 1;
        directional_shadow_quality_radius = 1.5;
      } break;
      case RS::SHADOW_QUALITY_SOFT_LOW: {
        directional_penumbra_shadow_samples = 8;
        directional_soft_shadow_samples = 4;
        directional_shadow_quality_radius = 2.0;
      } break;
      case RS::SHADOW_QUALITY_SOFT_MEDIUM: {
        directional_penumbra_shadow_samples = 12;
        directional_soft_shadow_samples = 8;
        directional_shadow_quality_radius = 2.0;
      } break;
      case RS::SHADOW_QUALITY_SOFT_HIGH: {
        directional_penumbra_shadow_samples = 24;
        directional_soft_shadow_samples = 16;
        directional_shadow_quality_radius = 3.0;
      } break;
      case RS::SHADOW_QUALITY_SOFT_ULTRA: {
        directional_penumbra_shadow_samples = 32;
        directional_soft_shadow_samples = 32;
        directional_shadow_quality_radius = 4.0;
      } break;
      case RS::SHADOW_QUALITY_MAX:
        break;
    }
    get_vogel_disk(directional_penumbra_shadow_kernel, directional_penumbra_shadow_samples);
    get_vogel_disk(directional_soft_shadow_kernel, directional_soft_shadow_samples);
  }

  _update_shader_quality_settings();
}

void lain::RendererSceneRenderRD::init() {
  max_cluster_elements = GLOBAL_GET("rendering/limits/cluster_builder/max_clustered_elements");
  max_cluster_elements = CLAMP(max_cluster_elements, 1, 1024);
  RendererRD::LightStorage::get_singleton()->set_max_cluster_elements(max_cluster_elements);
  /* Forward ID */
  forward_id_storage = create_forward_id_storage();
  {  // shader init
     /* SKY SHADER */
    sky.init();
    gi.init();  // make defalut texture
    {           //decals
      RendererRD::TextureStorage::get_singleton()->set_max_decals(max_cluster_elements);
    }
    // fog.init()
    bool can_use_storage = _render_buffers_can_be_storage();
    resolve_effects = memnew(RendererRD::Resolve);
    copy_effects = memnew(RendererRD::CopyEffects(!can_use_storage));
  }

  // 在rendering_system::init() 中 已经注册过了
  RSG::camera_attributes->camera_attributes_set_dof_blur_bokeh_shape(RS::DOFBokehShape(int(GLOBAL_GET("rendering/camera/depth_of_field/depth_of_field_bokeh_shape"))));
  RSG::camera_attributes->camera_attributes_set_dof_blur_quality(RS::DOFBlurQuality(int(GLOBAL_GET("rendering/camera/depth_of_field/depth_of_field_bokeh_quality"))),
                                                                 GLOBAL_GET("rendering/camera/depth_of_field/depth_of_field_use_jitter"));
  use_physical_light_units = GLOBAL_GET("rendering/lights_and_shadows/use_physical_light_units");

  positional_soft_shadow_filter_set_quality(RS::ShadowQuality(int(GLOBAL_GET("rendering/lights_and_shadows/positional_shadow/soft_shadow_filter_quality"))));
  directional_soft_shadow_filter_set_quality(RS::ShadowQuality(int(GLOBAL_GET("rendering/lights_and_shadows/directional_shadow/soft_shadow_filter_quality"))));

  cull_argument.set_page_pool(&cull_argument_pool);
	tone_mapper = memnew(RendererRD::ToneMapper);
}

bool RendererSceneRenderRD::free(RID p_rid) {
  if (is_environment(p_rid)) {
    environment_free(p_rid);
  } else if (is_compositor(p_rid)) {
    compositor_free(p_rid);
  } else if (is_compositor_effect(p_rid)) {
    compositor_effect_free(p_rid);
  } else if (RSG::camera_attributes->owns_camera_attributes(p_rid)) {
    RSG::camera_attributes->camera_attributes_free(p_rid);
  }
  // else if (gi.voxel_gi_instance_owns(p_rid)) {
  // gi.voxel_gi_instance_free(p_rid);
  // }
  // else if (sky.sky_owner.owns(p_rid)) {
  // 	sky.update_dirty_skys();
  // 	sky.free_sky(p_rid);
  // } else if (RendererRD::Fog::get_singleton()->owns_fog_volume_instance(p_rid)) {
  // 	RendererRD::Fog::get_singleton()->fog_instance_free(p_rid);
  // }
  else {
    return false;
  }

  return true;
}

RID lain::RendererSceneRenderRD::render_buffers_get_default_voxel_gi_buffer() {
  return gi.default_voxel_gi_buffer;
}

RID RendererSceneRenderRD::sky_allocate() {
	return sky.allocate_sky_rid();
}
void RendererSceneRenderRD::sky_initialize(RID p_rid) {
	sky.initialize_sky_rid(p_rid);
}

void RendererSceneRenderRD::sky_set_radiance_size(RID p_sky, int p_radiance_size) {
	sky.sky_set_radiance_size(p_sky, p_radiance_size);
}

void RendererSceneRenderRD::sky_set_mode(RID p_sky, RS::SkyMode p_mode) {
	sky.sky_set_mode(p_sky, p_mode);
}

void RendererSceneRenderRD::sky_set_material(RID p_sky, RID p_material) {
	sky.sky_set_material(p_sky, p_material);
}

Ref<Image> RendererSceneRenderRD::sky_bake_panorama(RID p_sky, float p_energy, bool p_bake_irradiance, const Size2i &p_size) {
	return sky.sky_bake_panorama(p_sky, p_energy, p_bake_irradiance, p_size);
}
bool RendererSceneRenderRD::_compositor_effects_has_flag(const RenderDataRD* p_render_data, RS::CompositorEffectFlags p_flag,
                                                         RS::CompositorEffectCallbackType p_callback_type) {
  RendererCompositorStorage* comp_storage = RendererCompositorStorage::get_singleton();

  if (p_render_data->compositor.is_null()) {
    return false;
  }

  if (p_render_data->reflection_probe.is_valid()) {
    return false;
  }

  ERR_FAIL_COND_V(!comp_storage->is_compositor(p_render_data->compositor), false);
  Vector<RID> re_rids = comp_storage->compositor_get_compositor_effects(p_render_data->compositor, p_callback_type, true);

  for (RID rid : re_rids) {
    if (comp_storage->compositor_effect_get_flag(rid, p_flag)) {
      return true;
    }
  }

  return false;
}

void RendererSceneRenderRD::_render_buffers_copy_screen_texture(const RenderDataRD* p_render_data) {
  Ref<RenderSceneBuffersRD> rb = p_render_data->render_buffers;
  ERR_FAIL_COND(rb.is_null());

  if (!rb->has_internal_texture()) {
    // We're likely rendering reflection probes where we can't use our backbuffers.
    return;
  }

  RD::get_singleton()->draw_command_begin_label("Copy screen texture");

  StringName texture_name;
  bool can_use_storage = _render_buffers_can_be_storage();
  Size2i size = rb->get_internal_size();

  // When upscaling, the blur texture needs to be at the target size for post-processing to work. We prefer to use a
  // dedicated backbuffer copy texture instead if the blur texture is not an option so shader effects work correctly.
  Size2i target_size = rb->get_target_size();
  bool internal_size_matches = (size.width() == target_size.width()) && (size.height() == target_size.height());
  bool reuse_blur_texture = !rb->has_upscaled_texture() || internal_size_matches;
  if (reuse_blur_texture) {
    rb->allocate_blur_textures();
    texture_name = RB_TEX_BLUR_0;
  } else {
    uint32_t usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT;
    usage_bits |= can_use_storage ? RD::TEXTURE_USAGE_STORAGE_BIT : RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT;
    rb->create_texture(RB_SCOPE_BUFFERS, RB_TEX_BACK_COLOR, rb->get_base_data_format(), usage_bits);
    texture_name = RB_TEX_BACK_COLOR;
  }

  for (uint32_t v = 0; v < rb->get_view_count(); v++) {
    RID texture = rb->get_internal_texture(v);
    int mipmaps = int(rb->get_texture_format(RB_SCOPE_BUFFERS, texture_name).mipmaps);
    RID dest = rb->get_texture_slice(RB_SCOPE_BUFFERS, texture_name, v, 0);

    if (can_use_storage) {
      copy_effects->copy_to_rect(texture, dest, Rect2i(0, 0, size.x, size.y));
    } else {
      RID fb = FramebufferCacheRD::get_singleton()->get_cache(dest);
      copy_effects->copy_to_fb_rect(texture, fb, Rect2i(0, 0, size.x, size.y));
    }

    for (int i = 1; i < mipmaps; i++) {
      RID source = dest;
      dest = rb->get_texture_slice(RB_SCOPE_BUFFERS, texture_name, v, i);
      Size2i msize = rb->get_texture_slice_size(RB_SCOPE_BUFFERS, texture_name, i);

      if (can_use_storage) {
        copy_effects->make_mipmap(source, dest, msize);
      } else {
        copy_effects->make_mipmap_raster(source, dest, msize);
      }
    }
  }

  RD::get_singleton()->draw_command_end_label();
}

void RendererSceneRenderRD::_render_buffers_copy_depth_texture(const RenderDataRD* p_render_data) {
  Ref<RenderSceneBuffersRD> rb = p_render_data->render_buffers;
  ERR_FAIL_COND(rb.is_null());

  if (!rb->has_depth_texture()) {
    // We're likely rendering reflection probes where we can't use our backbuffers.
    return;
  }

  RD::get_singleton()->draw_command_begin_label("Copy depth texture");

  // note, this only creates our back depth texture if we haven't already created it.
  uint32_t usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT;
  usage_bits |= RD::TEXTURE_USAGE_CAN_COPY_TO_BIT | RD::TEXTURE_USAGE_STORAGE_BIT;
  usage_bits |= RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT;  // set this as color attachment because we're copying data into it, it's not actually used as a depth buffer

  rb->create_texture(RB_SCOPE_BUFFERS, RB_TEX_BACK_DEPTH, RD::DATA_FORMAT_R32_SFLOAT, usage_bits, RD::TEXTURE_SAMPLES_1);

  bool can_use_storage = _render_buffers_can_be_storage();
  Size2i size = rb->get_internal_size();
  for (uint32_t v = 0; v < p_render_data->scene_data->view_count; v++) {
    RID depth_texture = rb->get_depth_texture(v);
    RID depth_back_texture = rb->get_texture_slice(RB_SCOPE_BUFFERS, RB_TEX_BACK_DEPTH, v, 0);

    if (can_use_storage) {
      copy_effects->copy_to_rect(depth_texture, depth_back_texture, Rect2i(0, 0, size.x, size.y));
    } else {
      RID depth_back_fb = FramebufferCacheRD::get_singleton()->get_cache(depth_back_texture);
      copy_effects->copy_to_fb_rect(depth_texture, depth_back_fb, Rect2i(0, 0, size.x, size.y));
    }
  }

  RD::get_singleton()->draw_command_end_label();
}

void lain::RendererSceneRenderRD::_render_buffers_post_process_and_tonemap(const RenderDataRD* p_render_data) {
  RendererRD::TextureStorage* texture_storage = RendererRD::TextureStorage::get_singleton();

  ERR_FAIL_NULL(p_render_data);

  Ref<RenderSceneBuffersRD> rb = p_render_data->render_buffers;
  ERR_FAIL_COND(rb.is_null());

  ERR_FAIL_COND_MSG(p_render_data->reflection_probe.is_valid(), "Post processes should not be applied on reflection probes.");

  // Glow, auto exposure and DoF (if enabled).

  Size2i target_size = rb->get_target_size();
  bool can_use_effects = target_size.x >= 8 && target_size.y >= 8;  // FIXME I think this should check internal size, we do all our post processing at this size...
  // can_use_effects &= _debug_draw_can_use_effects(debug_draw);
  bool can_use_storage = _render_buffers_can_be_storage();

  // bool use_fsr = fsr && can_use_effects && rb->get_scaling_3d_mode() == RS::VIEWPORT_SCALING_3D_MODE_FSR;
  bool use_fsr = false;
  bool use_upscaled_texture = rb->has_upscaled_texture() && rb->get_scaling_3d_mode() == RS::VIEWPORT_SCALING_3D_MODE_FSR2;

  RID render_target = rb->get_render_target();
  RID color_texture = use_upscaled_texture ? rb->get_upscaled_texture() : rb->get_internal_texture(); // 在fb中绘制好的color texture
  Size2i color_size = use_upscaled_texture ? target_size : rb->get_internal_size();
  // @todo: dof; glow; auto exposure
	bool dest_is_msaa_2d = rb->get_view_count() == 1 && texture_storage->render_target_get_msaa(render_target) != RS::VIEWPORT_MSAA_DISABLED;
  {
    RENDER_TIMESTAMP("Tonemap");
    RD::get_singleton()->draw_command_begin_label("Tonemap");

    RendererRD::ToneMapper::TonemapSettings tonemap; // setting 类

    // tonemap.exposure_texture = luminance->get_current_luminance_buffer(rb);
    // if (can_use_effects && RSG::camera_attributes->camera_attributes_uses_auto_exposure(p_render_data->camera_attributes) && tonemap.exposure_texture.is_valid()) {
    // 	tonemap.use_auto_exposure = true;
    // 	tonemap.auto_exposure_scale = auto_exposure_scale;
    // } else {
    // 	tonemap.exposure_texture = texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_WHITE);
    // }
    tonemap.exposure_texture = texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_WHITE);

    // if (can_use_effects && p_render_data->environment.is_valid() && environment_get_glow_enabled(p_render_data->environment)) {
    // 	tonemap.use_glow = true;
    // 	tonemap.glow_mode = RendererRD::ToneMapper::TonemapSettings::GlowMode(environment_get_glow_blend_mode(p_render_data->environment));
    // 	tonemap.glow_intensity = environment_get_glow_blend_mode(p_render_data->environment) == RS::ENV_GLOW_BLEND_MODE_MIX ? environment_get_glow_mix(p_render_data->environment) : environment_get_glow_intensity(p_render_data->environment);
    // 	for (int i = 0; i < RS::MAX_GLOW_LEVELS; i++) {
    // 		tonemap.glow_levels[i] = environment_get_glow_levels(p_render_data->environment)[i];
    // 	}

    // 	Size2i msize = rb->get_texture_slice_size(RB_SCOPE_BUFFERS, RB_TEX_BLUR_1, 0);
    // 	tonemap.glow_texture_size.x = msize.width;
    // 	tonemap.glow_texture_size.y = msize.height;
    // 	tonemap.glow_use_bicubic_upscale = glow_bicubic_upscale;
    // 	tonemap.glow_texture = rb->get_texture(RB_SCOPE_BUFFERS, RB_TEX_BLUR_1);
    // 	if (environment_get_glow_map(p_render_data->environment).is_valid()) {
    // 		tonemap.glow_map_strength = environment_get_glow_map_strength(p_render_data->environment);
    // 		tonemap.glow_map = texture_storage->texture_get_rd_texture(environment_get_glow_map(p_render_data->environment));
    // 	} else {
    // 		tonemap.glow_map_strength = 0.0f;
    // 		tonemap.glow_map = texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_WHITE);
    // 	}

    // } else {
    // 	tonemap.glow_texture = texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_BLACK);
    // 	tonemap.glow_map = texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_WHITE);
    // }
    tonemap.glow_texture = texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_BLACK);
    tonemap.glow_map = texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_WHITE);

    if (rb->get_screen_space_aa() == RS::VIEWPORT_SCREEN_SPACE_AA_FXAA) {
      tonemap.use_fxaa = true;
    }

    tonemap.use_debanding = rb->get_use_debanding();
    tonemap.texture_size = Vector2i(color_size.x, color_size.y);

    // if (p_render_data->environment.is_valid()) {
    //   tonemap.tonemap_mode = environment_get_tone_mapper(p_render_data->environment);
    //   tonemap.white = environment_get_white(p_render_data->environment);
    //   tonemap.exposure = environment_get_exposure(p_render_data->environment);
    // }

    tonemap.use_color_correction = false;
    tonemap.use_1d_color_correction = false;
    tonemap.color_correction_texture = texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_3D_WHITE);

    // if (can_use_effects && p_render_data->environment.is_valid()) {
    //   tonemap.use_bcs = environment_get_adjustments_enabled(p_render_data->environment);
    //   tonemap.brightness = environment_get_adjustments_brightness(p_render_data->environment);
    //   tonemap.contrast = environment_get_adjustments_contrast(p_render_data->environment);
    //   tonemap.saturation = environment_get_adjustments_saturation(p_render_data->environment);
    //   if (environment_get_adjustments_enabled(p_render_data->environment) && environment_get_color_correction(p_render_data->environment).is_valid()) {
    //     tonemap.use_color_correction = true;
    //     tonemap.use_1d_color_correction = environment_get_use_1d_color_correction(p_render_data->environment);
    //     tonemap.color_correction_texture = texture_storage->texture_get_rd_texture(environment_get_color_correction(p_render_data->environment));
    //   }
    // }

    tonemap.luminance_multiplier = _render_buffers_get_luminance_multiplier();
    tonemap.view_count = rb->get_view_count();

    tonemap.convert_to_srgb = !texture_storage->render_target_is_using_hdr(render_target);

    RID dest_fb;
    bool use_intermediate_fb = use_fsr;
    if (use_intermediate_fb) {
      // If we use FSR to upscale we need to write our result into an intermediate buffer.
      // Note that this is cached so we only create the texture the first time.
      RID dest_texture = rb->create_texture(SNAME("Tonemapper"), SNAME("destination"), _render_buffers_get_color_format(),
                                            RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT);
      dest_fb = FramebufferCacheRD::get_singleton()->get_cache(dest_texture);
    } else {
      // If we do a bilinear upscale we just render into our render target and our shader will upscale automatically.
      // Target size in this case is lying as we never get our real target size communicated.
      // Bit nasty but...

      if (dest_is_msaa_2d) { // 用 msaa 的纹理做的fb
        dest_fb = FramebufferCacheRD::get_singleton()->get_cache(texture_storage->render_target_get_rd_texture_msaa(render_target));
        texture_storage->render_target_set_msaa_needs_resolve(render_target, true);  // Make sure this gets resolved.
      } else {
        dest_fb = texture_storage->render_target_get_rd_framebuffer(render_target);
      }
    }

    tone_mapper->tonemapper(color_texture, dest_fb, tonemap);

    RD::get_singleton()->draw_command_end_label();
  }
  // @todo: fsr
  if (use_fsr) {}
  texture_storage->render_target_disable_clear_request(render_target);
}
