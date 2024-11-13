#include "render_forward_clustered.h"
#include "environment/renderer_fog.h"
#include "function/render/renderer_rd/storage/mesh_storage.h"
#include "function/render/rendering_system/rendering_system_globals.h"
#include "storage/light_storage.h"
using namespace lain;
using namespace lain::RendererSceneRenderImplementation;
RenderForwardClustered* RenderForwardClustered::singleton = nullptr;

RenderForwardClustered::RenderForwardClustered() {
  singleton = this;

  /* SCENE SHADER */

  {
    String defines;
    defines += "\n#define MAX_ROUGHNESS_LOD " + itos(get_roughness_layers() - 1) + ".0\n";
    if (is_using_radiance_cubemap_array()) {
      defines += "\n#define USE_RADIANCE_CUBEMAP_ARRAY \n";
    }
    defines += "\n#define SDFGI_OCT_SIZE " + itos(gi.sdfgi_get_lightprobe_octahedron_size()) + "\n";
    defines += "\n#define MAX_DIRECTIONAL_LIGHT_DATA_STRUCTS " + itos(MAX_DIRECTIONAL_LIGHTS) + "\n";

    {
      //lightmaps
      scene_state.max_lightmaps = MAX_LIGHTMAPS;
      defines += "\n#define MAX_LIGHTMAP_TEXTURES " + itos(scene_state.max_lightmaps) + "\n";
      defines += "\n#define MAX_LIGHTMAPS " + itos(scene_state.max_lightmaps) + "\n";

      scene_state.lightmap_buffer = RD::get_singleton()->storage_buffer_create(sizeof(LightmapData) * scene_state.max_lightmaps);
    }
    {
      //captures
      scene_state.max_lightmap_captures = 2048;
      scene_state.lightmap_captures = memnew_arr(LightmapCaptureData, scene_state.max_lightmap_captures);
      scene_state.lightmap_capture_buffer = RD::get_singleton()->storage_buffer_create(sizeof(LightmapCaptureData) * scene_state.max_lightmap_captures);
    }
    { defines += "\n#define MATERIAL_UNIFORM_SET " + itos(MATERIAL_UNIFORM_SET) + "\n"; }
#ifdef REAL_T_IS_DOUBLE
    { defines += "\n#define USE_DOUBLE_PRECISION \n"; }
#endif

    scene_shader.init(defines);
  }
}

void lain::RendererSceneRenderImplementation::RenderForwardClustered::setup_render_buffer_data(Ref<RenderSceneBuffersRD> p_render_buffers) {
  Ref<RenderBufferDataForwardClustered> data;
  data.instantiate();
  p_render_buffers->set_custom_data(RB_SCOPE_FORWARD_CLUSTERED, data);

  // Ref<RendererRD::GI::RenderBuffersGI> rbgi;
  // rbgi.instantiate();
  // p_render_buffers->set_custom_data(RB_SCOPE_GI, rbgi);
}

// 这里需要的数据都在父类型 renderer_scene_renderer_rd.h 中
void lain::RendererSceneRenderImplementation::RenderForwardClustered::_update_shader_quality_settings() {
  Vector<RD::PipelineSpecializationConstant> spec_constants;

  RD::PipelineSpecializationConstant sc;
  sc.type = RD::PIPELINE_SPECIALIZATION_CONSTANT_TYPE_INT;

  sc.constant_id = SPEC_CONSTANT_SOFT_SHADOW_SAMPLES;
  sc.int_value = soft_shadow_samples_get();

  spec_constants.push_back(sc);

  sc.constant_id = SPEC_CONSTANT_PENUMBRA_SHADOW_SAMPLES;
  sc.int_value = penumbra_shadow_samples_get();

  spec_constants.push_back(sc);

  sc.constant_id = SPEC_CONSTANT_DIRECTIONAL_SOFT_SHADOW_SAMPLES;
  sc.int_value = directional_soft_shadow_samples_get();

  spec_constants.push_back(sc);

  sc.constant_id = SPEC_CONSTANT_DIRECTIONAL_PENUMBRA_SHADOW_SAMPLES;
  sc.int_value = directional_penumbra_shadow_samples_get();

  spec_constants.push_back(sc);

  sc.type = RD::PIPELINE_SPECIALIZATION_CONSTANT_TYPE_BOOL;
  sc.constant_id = SPEC_CONSTANT_DECAL_FILTER;
  sc.bool_value = decals_get_filter() == RS::DECAL_FILTER_NEAREST_MIPMAPS || decals_get_filter() == RS::DECAL_FILTER_LINEAR_MIPMAPS ||
                  decals_get_filter() == RS::DECAL_FILTER_NEAREST_MIPMAPS_ANISOTROPIC || decals_get_filter() == RS::DECAL_FILTER_LINEAR_MIPMAPS_ANISOTROPIC;

  spec_constants.push_back(sc);

  sc.constant_id = SPEC_CONSTANT_PROJECTOR_FILTER;
  sc.bool_value = light_projectors_get_filter() == RS::LIGHT_PROJECTOR_FILTER_NEAREST_MIPMAPS || light_projectors_get_filter() == RS::LIGHT_PROJECTOR_FILTER_LINEAR_MIPMAPS ||
                  light_projectors_get_filter() == RS::LIGHT_PROJECTOR_FILTER_NEAREST_MIPMAPS_ANISOTROPIC ||
                  light_projectors_get_filter() == RS::LIGHT_PROJECTOR_FILTER_LINEAR_MIPMAPS_ANISOTROPIC;

  spec_constants.push_back(sc);

  scene_shader.set_default_specialization_constants(spec_constants);

  base_uniforms_changed();  //also need this
}

void lain::RendererSceneRenderImplementation::RenderForwardClustered::base_uniforms_changed() {
  if (!render_base_uniform_set.is_null() && RD::get_singleton()->uniform_set_is_valid(render_base_uniform_set)) {
    RD::get_singleton()->free(render_base_uniform_set);
  }
  render_base_uniform_set = RID();
}

// normal texture 不知道为什么在 customdata里面
// velocity 在 render buffers 里面 @？

RID RenderForwardClustered::_render_buffers_get_normal_texture(Ref<RenderSceneBuffersRD> p_render_buffers) {
  Ref<RenderBufferDataForwardClustered> rb_data = p_render_buffers->get_custom_data(RB_SCOPE_FORWARD_CLUSTERED);
  ERR_FAIL_COND_V(rb_data.is_null(), RID());
  return rb_data->get_normal_roughness();
}

RID lain::RendererSceneRenderImplementation::RenderForwardClustered::_render_buffers_get_velocity_texture(Ref<RenderSceneBuffersRD> p_render_buffers) {
  return p_render_buffers->get_velocity_buffer(false);
}

void lain::RendererSceneRenderImplementation::RenderForwardClustered::_render_scene(RenderDataRD* p_render_data, const Color& p_default_bg_color) {
  RendererRD::LightStorage* light_storage = RendererRD::LightStorage::get_singleton();

  ERR_FAIL_NULL(p_render_data);

  Ref<RenderSceneBuffersRD> rb = p_render_data->render_buffers;
  ERR_FAIL_COND(rb.is_null());
  Ref<RenderBufferDataForwardClustered> rb_data;
  if (rb->has_custom_data(RB_SCOPE_FORWARD_CLUSTERED)) {
    // Our forward clustered custom data buffer will only be available when we're rendering our normal view.
    // This will not be available when rendering reflection probes.
    rb_data = rb->get_custom_data(RB_SCOPE_FORWARD_CLUSTERED);
  }
  bool is_reflection_probe = p_render_data->reflection_probe.is_valid();
  static const int texture_multisamples[RS::VIEWPORT_MSAA_MAX] = {1, 2, 4, 8};
  //first of all, make a new render pass
  //fill up ubo

  RENDER_TIMESTAMP("Prepare 3D Scene");
  // get info about our rendering effects
  bool ce_needs_motion_vectors = _compositor_effects_has_flag(p_render_data, RS::COMPOSITOR_EFFECT_FLAG_NEEDS_MOTION_VECTORS);
  bool ce_needs_normal_roughness = _compositor_effects_has_flag(p_render_data, RS::COMPOSITOR_EFFECT_FLAG_NEEDS_ROUGHNESS);
  bool ce_needs_separate_specular = _compositor_effects_has_flag(p_render_data, RS::COMPOSITOR_EFFECT_FLAG_NEEDS_SEPARATE_SPECULAR);

  RENDER_TIMESTAMP("Setup 3D Scene");
  bool using_debug_mvs = get_debug_draw_mode() == RS::VIEWPORT_DEBUG_DRAW_MOTION_VECTORS;
  bool using_taa = rb->get_use_taa();
  bool using_fsr2 = rb->get_scaling_3d_mode() == RS::VIEWPORT_SCALING_3D_MODE_FSR2;
  // check if we need motion vectors
  bool motion_vectors_required;
  if (using_debug_mvs) {
    motion_vectors_required = true;
  } else if (ce_needs_motion_vectors) {
    motion_vectors_required = true;
  } else if (!is_reflection_probe && using_taa) {
    motion_vectors_required = true;
  } else if (!is_reflection_probe && using_fsr2) {
    motion_vectors_required = true;
  } else {
    motion_vectors_required = false;
  }

  RENDER_TIMESTAMP("Render 3D Transparent Pass");
}

RenderGeometryInstance* lain::RendererSceneRenderImplementation::RenderForwardClustered::geometry_instance_create(RID p_base) {
  RS::InstanceType type = RSG::utilities->get_base_type(p_base);
  ERR_FAIL_COND_V(!((1 << type) & RS::INSTANCE_GEOMETRY_MASK), nullptr);

  GeometryInstanceForwardClustered* ginstance = geometry_instance_alloc.alloc();
  ginstance->data = memnew(GeometryInstanceForwardClustered::Data);

  ginstance->data->base = p_base;
  ginstance->data->base_type = type;
  ginstance->data->dependency_tracker.userdata = ginstance;
  ginstance->data->dependency_tracker.changed_callback = _geometry_instance_dependency_changed;
  ginstance->data->dependency_tracker.deleted_callback = _geometry_instance_dependency_deleted;

  ginstance->_mark_dirty();

  return ginstance;
}

void RenderForwardClustered::geometry_instance_free(RenderGeometryInstance* p_geometry_instance) {
  GeometryInstanceForwardClustered* ginstance = static_cast<GeometryInstanceForwardClustered*>(p_geometry_instance);
  ERR_FAIL_NULL(ginstance);
  if (ginstance->lightmap_sh != nullptr) {
    geometry_instance_lightmap_sh.free(ginstance->lightmap_sh);
  }
  GeometryInstanceSurfaceDataCache* surf = ginstance->surface_caches;
  while (surf) {
    GeometryInstanceSurfaceDataCache* next = surf->next;
    geometry_instance_surface_alloc.free(surf);
    surf = next;
  }
  memdelete(ginstance->data);
  geometry_instance_alloc.free(ginstance);
}

void lain::RendererSceneRenderImplementation::RenderForwardClustered::_geometry_instance_dependency_changed(Dependency::DependencyChangedNotification p_notification,
                                                                                                            DependencyTracker* p_tracker) {
  switch (p_notification) {
    case Dependency::DEPENDENCY_CHANGED_MATERIAL:
    case Dependency::DEPENDENCY_CHANGED_MESH:
    case Dependency::DEPENDENCY_CHANGED_PARTICLES:
    case Dependency::DEPENDENCY_CHANGED_PARTICLES_INSTANCES:
    case Dependency::DEPENDENCY_CHANGED_MULTIMESH:
    case Dependency::DEPENDENCY_CHANGED_SKELETON_DATA: {
      static_cast<RenderGeometryInstance*>(p_tracker->userdata)->_mark_dirty();
      static_cast<GeometryInstanceForwardClustered*>(p_tracker->userdata)->data->dirty_dependencies = true;
    } break;
    case Dependency::DEPENDENCY_CHANGED_MULTIMESH_VISIBLE_INSTANCES: {
      GeometryInstanceForwardClustered* ginstance = static_cast<GeometryInstanceForwardClustered*>(p_tracker->userdata);
      if (ginstance->data->base_type == RS::INSTANCE_MULTIMESH) {
        // ginstance->instance_count = RendererRD::MeshStorage::get_singleton()->multimesh_get_instances_to_draw(ginstance->data->base);
      }
    } break;
    default: {
      //rest of notifications of no interest
    } break;
  }
}

void RenderForwardClustered::_geometry_instance_dependency_deleted(const RID& p_dependency, DependencyTracker* p_tracker) {
  static_cast<RenderGeometryInstance*>(p_tracker->userdata)->_mark_dirty();
  static_cast<GeometryInstanceForwardClustered*>(p_tracker->userdata)->data->dirty_dependencies = true;
}

void RenderForwardClustered::RenderBufferDataForwardClustered::configure(RenderSceneBuffersRD* p_render_buffers) {
  if (render_buffers) {
    // JIC
    free_data();
  }

  render_buffers = p_render_buffers;
  ERR_FAIL_NULL(render_buffers);

  // if (cluster_builder == nullptr) {
  // 	cluster_builder = memnew(ClusterBuilderRD);
  // }
  // cluster_builder->set_shared(RenderForwardClustered::get_singleton()->get_cluster_builder_shared());

  // RID sampler = RendererRD::MaterialStorage::get_singleton()->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED);
  // cluster_builder->setup(p_render_buffers->get_internal_size(), p_render_buffers->get_max_cluster_elements(), p_render_buffers->get_depth_texture(), sampler, p_render_buffers->get_internal_texture());
}
void RenderForwardClustered::RenderBufferDataForwardClustered::free_data() {
  // JIC, should already have been cleared
  if (render_buffers) {
    render_buffers->clear_context(RB_SCOPE_FORWARD_CLUSTERED);
    // render_buffers->clear_context(RB_SCOPE_SSDS);
    // render_buffers->clear_context(RB_SCOPE_SSIL);
    // render_buffers->clear_context(RB_SCOPE_SSAO);
    // render_buffers->clear_context(RB_SCOPE_SSR);
  }

  // if (cluster_builder) {
  // 	memdelete(cluster_builder);
  // 	cluster_builder = nullptr;
  // }

  // if (fsr2_context) {
  // 	memdelete(fsr2_context);
  // 	fsr2_context = nullptr;
  // }

  // if (!render_sdfgi_uniform_set.is_null() && RD::get_singleton()->uniform_set_is_valid(render_sdfgi_uniform_set)) {
  // 	RD::get_singleton()->free(render_sdfgi_uniform_set);
  // }
}

void RenderForwardClustered::GeometryInstanceForwardClustered::_mark_dirty() {
  if (dirty_list_element.in_list()) {
    return;
  }

  //clear surface caches
  GeometryInstanceSurfaceDataCache* surf = surface_caches;

  while (surf) {
    GeometryInstanceSurfaceDataCache* next = surf->next;
    RenderForwardClustered::get_singleton()->geometry_instance_surface_alloc.free(surf);
    surf = next;
  }

  surface_caches = nullptr;

  RenderForwardClustered::get_singleton()->geometry_instance_dirty_list.add(&dirty_list_element);
}
void RenderForwardClustered::GeometryInstanceForwardClustered::set_transform(const Transform3D& p_transform, const AABB& p_aabb, const AABB& p_transformed_aabb) {
  uint64_t frame = RSG::rasterizer->get_frame_number();
  if (frame != prev_transform_change_frame) {
    prev_transform = transform;
    prev_transform_change_frame = frame;
    prev_transform_dirty = true;
  }

  RenderGeometryInstanceBase::set_transform(p_transform, p_aabb, p_transformed_aabb);
}

void RenderForwardClustered::GeometryInstanceForwardClustered::set_use_lightmap(RID p_lightmap_instance, const Rect2& p_lightmap_uv_scale, int p_lightmap_slice_index) {
  lightmap_instance = p_lightmap_instance;
  lightmap_uv_scale = p_lightmap_uv_scale;
  lightmap_slice_index = p_lightmap_slice_index;

  _mark_dirty();
}
void RenderForwardClustered::GeometryInstanceForwardClustered::set_lightmap_capture(const Color* p_sh9) {
  if (p_sh9) {
    if (lightmap_sh == nullptr) {
      lightmap_sh = RenderForwardClustered::get_singleton()->geometry_instance_lightmap_sh.alloc();
    }

    memcpy(lightmap_sh->sh, p_sh9, sizeof(Color) * 9);
  } else {
    if (lightmap_sh != nullptr) {
      RenderForwardClustered::get_singleton()->geometry_instance_lightmap_sh.free(lightmap_sh);
      lightmap_sh = nullptr;
    }
  }
  _mark_dirty();
}

void RenderForwardClustered::GeometryInstanceForwardClustered::pair_voxel_gi_instances(const RID* p_voxel_gi_instances, uint32_t p_voxel_gi_instance_count) {
  if (p_voxel_gi_instance_count > 0) {
    voxel_gi_instances[0] = p_voxel_gi_instances[0];
  } else {
    voxel_gi_instances[0] = RID();
  }

  if (p_voxel_gi_instance_count > 1) {
    voxel_gi_instances[1] = p_voxel_gi_instances[1];
  } else {
    voxel_gi_instances[1] = RID();
  }
}

void RenderForwardClustered::GeometryInstanceForwardClustered::set_softshadow_projector_pairing(bool p_softshadow, bool p_projector) {
  using_projectors = p_projector;
  using_softshadows = p_softshadow;
  _mark_dirty();
}
uint32_t RenderForwardClustered::geometry_instance_get_pair_mask() {
  return (1 << RS::INSTANCE_VOXEL_GI);
}
bool RenderForwardClustered::free(RID p_rid) {
  if (RendererSceneRenderRD::free(p_rid)) {
    return true;
  }
  return false;
}

void RenderForwardClustered::_setup_environment(const RenderDataRD* p_render_data, bool p_no_fog, const Size2i& p_screen_size, const Color& p_default_bg_color,
                                                bool p_opaque_render_buffers, bool p_apply_alpha_multiplier, bool p_pancake_shadows, int p_index) {
  RendererRD::LightStorage* light_storage = RendererRD::LightStorage::get_singleton();

  Ref<RenderSceneBuffersRD> rd = p_render_data->render_buffers;
  RID env = is_environment(p_render_data->environment) ? p_render_data->environment : RID();
  // RID reflection_probe_instance = p_render_data->reflection_probe.is_valid() ? light_storage->reflection_probe_instance_get_probe(p_render_data->reflection_probe) : RID();

  // May do this earlier in RenderSceneRenderRD::render_scene
  if (p_index >= (int)scene_state.uniform_buffers.size()) {
    uint32_t from = scene_state.uniform_buffers.size();
    scene_state.uniform_buffers.resize(p_index + 1);
    for (uint32_t i = from; i < scene_state.uniform_buffers.size(); i++) {
      scene_state.uniform_buffers[i] = p_render_data->scene_data->create_uniform_buffer();
    }
  }
  //
  RID reflection_probe_instance = RID();
  //
  p_render_data->scene_data->update_ubo(scene_state.uniform_buffers[p_index], get_debug_draw_mode(), env, reflection_probe_instance, p_render_data->camera_attributes,
                                        p_pancake_shadows, p_screen_size, p_default_bg_color, 1.0, p_opaque_render_buffers, p_apply_alpha_multiplier);

  // now do implementation UBO

  scene_state.ubo.cluster_shift = get_shift_from_power_of_2(p_render_data->cluster_size);
  scene_state.ubo.max_cluster_element_count_div_32 = p_render_data->cluster_max_elements / 32;
  {
    uint32_t cluster_screen_width = Math::division_round_up((uint32_t)p_screen_size.width(), p_render_data->cluster_size);
    uint32_t cluster_screen_height = Math::division_round_up((uint32_t)p_screen_size.height(), p_render_data->cluster_size);
    scene_state.ubo.cluster_type_size = cluster_screen_width * cluster_screen_height * (scene_state.ubo.max_cluster_element_count_div_32 + 32);
    scene_state.ubo.cluster_width = cluster_screen_width;
  }

  scene_state.ubo.gi_upscale_for_msaa = false;
  scene_state.ubo.volumetric_fog_enabled = false;

  if (rd.is_valid()) {
    if (rd->get_msaa_3d() != RS::VIEWPORT_MSAA_DISABLED) {
      scene_state.ubo.gi_upscale_for_msaa = true;
    }

    // if (rd->has_custom_data(RB_SCOPE_FOG)) {
    // 	Ref<RendererRD::Fog::VolumetricFog> fog = rd->get_custom_data(RB_SCOPE_FOG);

    // 	scene_state.ubo.volumetric_fog_enabled = true;
    // 	float fog_end = fog->length;
    // 	if (fog_end > 0.0) {
    // 		scene_state.ubo.volumetric_fog_inv_length = 1.0 / fog_end;
    // 	} else {
    // 		scene_state.ubo.volumetric_fog_inv_length = 1.0;
    // 	}

    // 	float fog_detail_spread = fog->spread; //reverse lookup
    // 	if (fog_detail_spread > 0.0) {
    // 		scene_state.ubo.volumetric_fog_detail_spread = 1.0 / fog_detail_spread;
    // 	} else {
    // 		scene_state.ubo.volumetric_fog_detail_spread = 1.0;
    // 	}
    // }
  }

  if (get_debug_draw_mode() == RS::VIEWPORT_DEBUG_DRAW_UNSHADED) {
    scene_state.ubo.ss_effects_flags = 0;
  } else if (p_render_data->reflection_probe.is_null() && is_environment(p_render_data->environment)) {
    scene_state.ubo.ssao_ao_affect = environment_get_ssao_ao_channel_affect(p_render_data->environment);
    scene_state.ubo.ssao_light_affect = environment_get_ssao_direct_light_affect(p_render_data->environment);
    uint32_t ss_flags = 0;
    if (p_opaque_render_buffers) {
      ss_flags |= environment_get_ssao_enabled(p_render_data->environment) ? 1 : 0;
      ss_flags |= environment_get_ssil_enabled(p_render_data->environment) ? 2 : 0;
    }
    scene_state.ubo.ss_effects_flags = ss_flags;
  } else {
    scene_state.ubo.ss_effects_flags = 0;
  }

  if (p_index >= (int)scene_state.implementation_uniform_buffers.size()) {
    uint32_t from = scene_state.implementation_uniform_buffers.size();
    scene_state.implementation_uniform_buffers.resize(p_index + 1);
    for (uint32_t i = from; i < scene_state.implementation_uniform_buffers.size(); i++) {
      scene_state.implementation_uniform_buffers[i] = RD::get_singleton()->uniform_buffer_create(sizeof(SceneState::UBO));
    }
  }

  RD::get_singleton()->buffer_update(scene_state.implementation_uniform_buffers[p_index], 0, sizeof(SceneState::UBO), &scene_state.ubo);
}
