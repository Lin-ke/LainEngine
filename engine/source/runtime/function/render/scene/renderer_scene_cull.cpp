#include "renderer_scene_cull.h"
#include "function/render/rendering_system/rendering_system_globals.h"
using namespace lain;
RID lain::RendererSceneCull::camera_allocate() {
  return camera_owner.allocate_rid();
}

void RendererSceneCull::camera_initialize(RID p_rid) {
  camera_owner.initialize_rid(p_rid);
}
void RendererSceneCull::camera_set_perspective(RID p_camera, float p_fovy_degrees, float p_z_near, float p_z_far) {
  Camera* camera = camera_owner.get_or_null(p_camera);
  ERR_FAIL_NULL(camera);
  camera->type = Camera::PERSPECTIVE;
  camera->fov = p_fovy_degrees;
  camera->znear = p_z_near;
  camera->zfar = p_z_far;
}

void RendererSceneCull::camera_set_orthogonal(RID p_camera, float p_size, float p_z_near, float p_z_far) {
  Camera* camera = camera_owner.get_or_null(p_camera);
  ERR_FAIL_NULL(camera);
  camera->type = Camera::ORTHOGONAL;
  camera->size = p_size;
  camera->znear = p_z_near;
  camera->zfar = p_z_far;
}

void RendererSceneCull::camera_set_frustum(RID p_camera, float p_size, Vector2 p_offset, float p_z_near, float p_z_far) {
  Camera* camera = camera_owner.get_or_null(p_camera);
  ERR_FAIL_NULL(camera);
  camera->type = Camera::FRUSTUM;
  camera->size = p_size;
  camera->offset = p_offset;
  camera->znear = p_z_near;
  camera->zfar = p_z_far;
}

void RendererSceneCull::camera_set_transform(RID p_camera, const Transform3D& p_transform) {
  Camera* camera = camera_owner.get_or_null(p_camera);
  ERR_FAIL_NULL(camera);
  camera->transform = p_transform.orthonormalized();
}

void RendererSceneCull::camera_set_cull_mask(RID p_camera, uint32_t p_layers) {
  Camera* camera = camera_owner.get_or_null(p_camera);
  ERR_FAIL_NULL(camera);

  camera->visible_layers = p_layers;
}

void RendererSceneCull::camera_set_environment(RID p_camera, RID p_env) {
  Camera* camera = camera_owner.get_or_null(p_camera);
  ERR_FAIL_NULL(camera);
  camera->env = p_env;
}

void RendererSceneCull::camera_set_camera_attributes(RID p_camera, RID p_attributes) {
  Camera* camera = camera_owner.get_or_null(p_camera);
  ERR_FAIL_NULL(camera);
  camera->attributes = p_attributes;
}

void RendererSceneCull::camera_set_compositor(RID p_camera, RID p_compositor) {
  Camera* camera = camera_owner.get_or_null(p_camera);
  ERR_FAIL_NULL(camera);
  camera->compositor = p_compositor;
}

void RendererSceneCull::camera_set_use_vertical_aspect(RID p_camera, bool p_enable) {
  Camera* camera = camera_owner.get_or_null(p_camera);
  ERR_FAIL_NULL(camera);
  camera->vaspect = p_enable;
}

bool lain::RendererSceneCull::is_camera(RID p_camera) const {
  return camera_owner.owns(p_camera);
}

RID lain::RendererSceneCull::occluder_allocate() {
  return RendererSceneOcclusionCull::get_singleton()->occluder_allocate();
}

void lain::RendererSceneCull::occluder_initialize(RID p_occluder) {
  RendererSceneOcclusionCull::get_singleton()->occluder_initialize(p_occluder);
}

void lain::RendererSceneCull::occluder_set_mesh(RID p_occluder, const PackedVector3Array& p_vertices, const PackedInt32Array& p_indices) {
  RendererSceneOcclusionCull::get_singleton()->occluder_set_mesh(p_occluder, p_vertices, p_indices);
}

RID RendererSceneCull::scenario_allocate() {
  return scenario_owner.allocate_rid();
}

void lain::RendererSceneCull::scenario_initialize(RID p_rid) {
  scenario_owner.initialize_rid(p_rid);
  Scenario* scenario = scenario_owner.get_or_null(p_rid);
  ERR_FAIL_NULL(scenario);  // @bug godot代码里少了这段
  scenario->self = p_rid;
  scenario->reflection_probe_shadow_atlas = RSG::light_storage->shadow_atlas_create();
  RSG::light_storage->shadow_atlas_set_size(scenario->reflection_probe_shadow_atlas, 1024);  //make enough shadows for close distance, don't bother with rest
  RSG::light_storage->shadow_atlas_set_quadrant_subdivision(scenario->reflection_probe_shadow_atlas, 0, 4);
  RSG::light_storage->shadow_atlas_set_quadrant_subdivision(scenario->reflection_probe_shadow_atlas, 1, 4);
  RSG::light_storage->shadow_atlas_set_quadrant_subdivision(scenario->reflection_probe_shadow_atlas, 2, 4);
  RSG::light_storage->shadow_atlas_set_quadrant_subdivision(scenario->reflection_probe_shadow_atlas, 3, 8);

  // scenario->reflection_atlas = RSG::light_storage->reflection_atlas_create();

  scenario->instance_aabbs.set_page_pool(&instance_aabb_page_pool);
  scenario->instance_data.set_page_pool(&instance_data_page_pool);
  scenario->instance_visibility.set_page_pool(&instance_visibility_data_page_pool);

  RendererSceneOcclusionCull::get_singleton()->add_scenario(p_rid);
}

void RendererSceneCull::scenario_set_environment(RID p_scenario, RID p_environment) {
  Scenario* scenario = scenario_owner.get_or_null(p_scenario);
  ERR_FAIL_NULL(scenario);
  scenario->environment = p_environment;
}

void RendererSceneCull::scenario_set_compositor(RID p_scenario, RID p_compositor) {
  Scenario* scenario = scenario_owner.get_or_null(p_scenario);
  ERR_FAIL_NULL(scenario);
  scenario->compositor = p_compositor;
}

void RendererSceneCull::scenario_set_fallback_environment(RID p_scenario, RID p_environment) {
  Scenario* scenario = scenario_owner.get_or_null(p_scenario);
  ERR_FAIL_NULL(scenario);
  scenario->fallback_environment = p_environment;
}

void RendererSceneCull::scenario_set_reflection_atlas_size(RID p_scenario, int p_reflection_size, int p_reflection_count) {
  Scenario* scenario = scenario_owner.get_or_null(p_scenario);
  ERR_FAIL_NULL(scenario);
  // RSG::light_storage->reflection_atlas_set_size(scenario->reflection_atlas, p_reflection_size, p_reflection_count);
}
bool RendererSceneCull::is_scenario(RID p_scenario) const {
  return scenario_owner.owns(p_scenario);
}

RID lain::RendererSceneCull::scenario_get_environment(RID p_scenario) {
  Scenario* scenario = scenario_owner.get_or_null(p_scenario);
  ERR_FAIL_NULL_V(scenario, RID());
  return scenario->environment;
}
void lain::RendererSceneCull::scenario_set_camera_attributes(RID p_scenario, RID p_camera_attributes) {
  Scenario* scenario = scenario_owner.get_or_null(p_scenario);
  ERR_FAIL_NULL(scenario);
  scenario->camera_attributes = p_camera_attributes;
}

void RendererSceneCull::scenario_add_viewport_visibility_mask(RID p_scenario, RID p_viewport) {
  Scenario* scenario = scenario_owner.get_or_null(p_scenario);
  ERR_FAIL_NULL(scenario);
  ERR_FAIL_COND(scenario->viewport_visibility_masks.has(p_viewport));

  uint64_t new_mask = 1;
  // 第一个可用的 （0）
  while (new_mask & scenario->used_viewport_visibility_bits) {
    new_mask <<= 1;
  }

  if (new_mask == 0) {
    ERR_PRINT("Only 64 viewports per scenario allowed when using visibility ranges.");
    new_mask = ((uint64_t)1) << 63;
  }

  scenario->viewport_visibility_masks[p_viewport] = new_mask;  // 哪一位
  scenario->used_viewport_visibility_bits |= new_mask;
}

void lain::RendererSceneCull::scenario_remove_viewport_visibility_mask(RID p_scenario, RID p_viewport) {
  Scenario* scenario = scenario_owner.get_or_null(p_scenario);
  ERR_FAIL_NULL(scenario);
  ERR_FAIL_COND(!scenario->viewport_visibility_masks.has(p_viewport));

  uint64_t mask = scenario->viewport_visibility_masks[p_viewport];
  scenario->viewport_visibility_masks.erase(p_viewport);
  scenario->used_viewport_visibility_bits &= ~mask;
}

void lain::RendererSceneCull::InstanceCullResult::clear() {
  geometry_instances.clear();
  lights.clear();
  light_instances.clear();
  lightmaps.clear();
  reflections.clear();
  decals.clear();
  voxel_gi_instances.clear();
  mesh_instances.clear();
  fog_volumes.clear();
  // directional_shadows
  for (int i = 0; i < RendererSceneRender::MAX_DIRECTIONAL_LIGHTS; i++) {
    for (int j = 0; j < RendererSceneRender::MAX_DIRECTIONAL_LIGHT_CASCADES; j++) {
      directional_shadows[i].cascade_geometry_instances[j].clear();
    }
  }

  for (int i = 0; i < SDFGI_MAX_CASCADES * SDFGI_MAX_REGIONS_PER_CASCADE; i++) {
    sdfgi_region_geometry_instances[i].clear();
  }

  for (int i = 0; i < SDFGI_MAX_CASCADES; i++) {
    sdfgi_cascade_lights[i].clear();
  }
}
void lain::RendererSceneCull::InstanceCullResult::reset() {
  geometry_instances.reset();
  lights.reset();
  light_instances.reset();
  lightmaps.reset();
  reflections.reset();
  decals.reset();
  voxel_gi_instances.reset();
  mesh_instances.reset();
  fog_volumes.reset();
  for (int i = 0; i < RendererSceneRender::MAX_DIRECTIONAL_LIGHTS; i++) {
    for (int j = 0; j < RendererSceneRender::MAX_DIRECTIONAL_LIGHT_CASCADES; j++) {
      directional_shadows[i].cascade_geometry_instances[j].reset();
    }
  }

  for (int i = 0; i < SDFGI_MAX_CASCADES * SDFGI_MAX_REGIONS_PER_CASCADE; i++) {
    sdfgi_region_geometry_instances[i].reset();
  }

  for (int i = 0; i < SDFGI_MAX_CASCADES; i++) {
    sdfgi_cascade_lights[i].reset();
  }
}

void lain::RendererSceneCull::InstanceCullResult::append_from(InstanceCullResult& p_cull_result) {

  geometry_instances.merge_unordered(p_cull_result.geometry_instances);
  lights.merge_unordered(p_cull_result.lights);
  light_instances.merge_unordered(p_cull_result.light_instances);
  lightmaps.merge_unordered(p_cull_result.lightmaps);
  reflections.merge_unordered(p_cull_result.reflections);
  decals.merge_unordered(p_cull_result.decals);
  voxel_gi_instances.merge_unordered(p_cull_result.voxel_gi_instances);
  mesh_instances.merge_unordered(p_cull_result.mesh_instances);
  fog_volumes.merge_unordered(p_cull_result.fog_volumes);

  for (int i = 0; i < RendererSceneRender::MAX_DIRECTIONAL_LIGHTS; i++) {
    for (int j = 0; j < RendererSceneRender::MAX_DIRECTIONAL_LIGHT_CASCADES; j++) {
      directional_shadows[i].cascade_geometry_instances[j].merge_unordered(p_cull_result.directional_shadows[i].cascade_geometry_instances[j]);
    }
  }

  for (int i = 0; i < SDFGI_MAX_CASCADES * SDFGI_MAX_REGIONS_PER_CASCADE; i++) {
    sdfgi_region_geometry_instances[i].merge_unordered(p_cull_result.sdfgi_region_geometry_instances[i]);
  }

  for (int i = 0; i < SDFGI_MAX_CASCADES; i++) {
    sdfgi_cascade_lights[i].merge_unordered(p_cull_result.sdfgi_cascade_lights[i]);
  }
}

void lain::RendererSceneCull::InstanceCullResult::init(PagedArrayPool<RID>* p_rid_pool, PagedArrayPool<RenderGeometryInstance*>* p_geometry_instance_pool,
                                                       PagedArrayPool<Instance*>* p_instance_pool) {
  geometry_instances.set_page_pool(p_geometry_instance_pool);
  light_instances.set_page_pool(p_rid_pool);
  lights.set_page_pool(p_instance_pool);
  lightmaps.set_page_pool(p_rid_pool);
  reflections.set_page_pool(p_rid_pool);
  decals.set_page_pool(p_rid_pool);
  voxel_gi_instances.set_page_pool(p_rid_pool);
  mesh_instances.set_page_pool(p_rid_pool);
  fog_volumes.set_page_pool(p_rid_pool);
  for (int i = 0; i < RendererSceneRender::MAX_DIRECTIONAL_LIGHTS; i++) {
    for (int j = 0; j < RendererSceneRender::MAX_DIRECTIONAL_LIGHT_CASCADES; j++) {
      directional_shadows[i].cascade_geometry_instances[j].set_page_pool(p_geometry_instance_pool);
    }
  }

  for (int i = 0; i < SDFGI_MAX_CASCADES * SDFGI_MAX_REGIONS_PER_CASCADE; i++) {
    sdfgi_region_geometry_instances[i].set_page_pool(p_geometry_instance_pool);
  }

  for (int i = 0; i < SDFGI_MAX_CASCADES; i++) {
    sdfgi_cascade_lights[i].set_page_pool(p_rid_pool);
  }
}

RendererSceneCull::Instance::Instance() : update_item(this) {
  dependency_tracker.userdata = this;
}
RID lain::RendererSceneCull::instance_allocate() {
  return instance_owner.allocate_rid();
}

void lain::RendererSceneCull::instance_initialize(RID p_rid) {
  instance_owner.initialize_rid(p_rid);
  Instance* instance = instance_owner.get_or_null(p_rid);
  instance->self = p_rid;
}

void lain::RendererSceneCull::instance_set_base(RID p_instance, RID p_base) {
  Instance* instance = instance_owner.get_or_null(p_instance);  // self 是 instance 的RID，
  ERR_FAIL_NULL(instance);

  Scenario* scenario = instance->scenario;
  // already set
  if (instance->base_type != RS::INSTANCE_NONE) {
    return;
  }
  instance->base_type = RS::INSTANCE_NONE;
  instance->base = RID();

  if (!p_base.is_valid()) {
    _instance_queue_update(instance, true, true);
    return;
  }
  instance->base_type = RSG::utilities->get_base_type(p_base);
  // fix up a specific malfunctioning case before the switch, so it can be handled
  // if (instance->base_type == RS::INSTANCE_NONE && RendererSceneOcclusionCull::get_singleton()->is_occluder(p_base)) {
  // 	instance->base_type = RS::INSTANCE_OCCLUDER;
  // }
  // new 并 填充basedata, 以及
  switch (instance->base_type) {
    case RS::INSTANCE_NONE: {
      ERR_PRINT_ONCE("unimplemented base type encountered in renderer scene cull");
      return;
    }
    case RS::INSTANCE_LIGHT: {
      InstanceLightData* light = memnew(InstanceLightData);
      instance->base_data = light;
    } break;
    case RS::INSTANCE_MESH:
    case RS::INSTANCE_MULTIMESH:
    case RS::INSTANCE_PARTICLES: {
      InstanceGeometryData* geom = memnew(InstanceGeometryData);
      instance->base_data = geom;

    } break;
    case RS::INSTANCE_PARTICLES_COLLISION: {
      InstanceParticlesCollisionData* collision = memnew(InstanceParticlesCollisionData);
      instance->base_data = collision;
    } break;
    case RS::INSTANCE_FOG_VOLUME: {
      InstanceFogVolumeData* fog = memnew(InstanceFogVolumeData);
      instance->base_data = fog;
    } break;
    case RS::INSTANCE_DECAL: {
      InstanceDecalData* decal = memnew(InstanceDecalData);
      instance->base_data = decal;
    } break;
    case RS::INSTANCE_REFLECTION_PROBE: {
      InstanceReflectionProbeData* reflection = memnew(InstanceReflectionProbeData);
      instance->base_data = reflection;
    } break;
    case RS::INSTANCE_VOXEL_GI: {
      InstanceVoxelGIData* voxel_gi = memnew(InstanceVoxelGIData);
      instance->base_data = voxel_gi;
    } break;
    case RS::INSTANCE_LIGHTMAP: {
      InstanceLightmapData* lightmap = memnew(InstanceLightmapData);
      instance->base_data = lightmap;
    } break;
    case RS::INSTANCE_VISIBLITY_NOTIFIER: {
      InstanceVisibilityNotifierData* visibility_notifier = memnew(InstanceVisibilityNotifierData);
      instance->base_data = visibility_notifier;
    } break;
    case RS::INSTANCE_OCCLUDER: {
      InstanceOccluderData* occluder = memnew(InstanceOccluderData);
      instance->base_data = occluder;
    } break;
    default: {
    }
  }
  instance->base = p_base;
  if (instance->base_type == RS::INSTANCE_MESH) {
    _instance_update_mesh_instance(instance);
  }
  RSG::utilities->base_update_dependency(p_base, &instance->dependency_tracker);
  _instance_queue_update(instance, true, true);
}

void lain::RendererSceneCull::instance_set_scenario(RID p_instance, RID p_scenario) {
  Instance* instance = instance_owner.get_or_null(p_instance);
  ERR_FAIL_NULL(instance);
  if (p_scenario.is_valid()) {
    Scenario* scenario = scenario_owner.get_or_null(p_scenario);
    ERR_FAIL_NULL(scenario);
    // 如果已经在 scenario 中，修改
    if (instance->scenario) {
      instance->scenario->instances.remove(&instance->scenario_item);
      if (instance->indexer_id.is_valid()) {
        _unpair_instance(instance);
      }
      switch (instance->base_type) {
        case RS::INSTANCE_LIGHT: {
          InstanceLightData* light = static_cast<InstanceLightData*>(instance->base_data);
          if (instance->visible && RSG::light_storage->light_get_type(instance->base) != RS::LIGHT_DIRECTIONAL && light->bake_mode == RS::LIGHT_BAKE_DYNAMIC) {
            instance->scenario->dynamic_lights.erase(light->instance);
          }
          if (light->D) {
            instance->scenario->directional_lights.erase(light->D);
            light->D = nullptr;
          }
        }

        break;
        case RS::INSTANCE_REFLECTION_PROBE: {
          InstanceReflectionProbeData* reflection_probe = static_cast<InstanceReflectionProbeData*>(instance->base_data);
          // RSG::light_storage->reflection_probe_release_atlas_index(reflection_probe->instance);

        } break;
        case RS::INSTANCE_VOXEL_GI: {
          InstanceVoxelGIData* voxel_gi = static_cast<InstanceVoxelGIData*>(instance->base_data);
          if (voxel_gi->update_element.in_list()) {
            voxel_gi_update_list.remove(&voxel_gi->update_element);
          }
        } break;
        default:
          break;
      }
      instance->scenario = nullptr;
    }

    instance->scenario = scenario;

    scenario->instances.add(&instance->scenario_item);
    // 需要特殊处理的情况
    switch (instance->base_type) {
      case RS::INSTANCE_LIGHT: {
        InstanceLightData* light = static_cast<InstanceLightData*>(instance->base_data);

        if (RSG::light_storage->light_get_type(instance->base) == RS::LIGHT_DIRECTIONAL) {
          light->D = scenario->directional_lights.push_back(instance);
        }
      } break;
      case RS::INSTANCE_VOXEL_GI: {
        InstanceVoxelGIData* voxel_gi = static_cast<InstanceVoxelGIData*>(instance->base_data);
        if (!voxel_gi->update_element.in_list()) {
          voxel_gi_update_list.add(&voxel_gi->update_element);
        }
      } break;
      case RS::INSTANCE_OCCLUDER: {
        RendererSceneOcclusionCull::get_singleton()->scenario_set_instance(scenario->self, p_instance, instance->base, instance->transform, instance->visible);
      } break;
      default: {
      }
    }

    _instance_queue_update(instance, true, true);
  }
}

void lain::RendererSceneCull::instance_set_layer_mask(RID p_instance, uint32_t p_mask) {
  Instance* instance = instance_owner.get_or_null(p_instance);
  ERR_FAIL_NULL(instance);

  if (instance->layer_mask == p_mask) {
    return;
  }

  instance->layer_mask = p_mask;
  if (instance->scenario && instance->array_index >= 0) {
    instance->scenario->instance_data[instance->array_index].layer_mask = p_mask;
  }

  if ((1 << instance->base_type) & RS::INSTANCE_GEOMETRY_MASK && instance->base_data) {
    InstanceGeometryData* geom = static_cast<InstanceGeometryData*>(instance->base_data);
    ERR_FAIL_NULL(geom->geometry_instance);
    geom->geometry_instance->set_layer_mask(p_mask);

    if (geom->can_cast_shadows) {
      for (HashSet<RendererSceneCull::Instance*>::Iterator I = geom->lights.begin(); I != geom->lights.end(); ++I) {
        InstanceLightData* light = static_cast<InstanceLightData*>((*I)->base_data);
        light->make_shadow_dirty();
      }
    }
  }
}
// /设置排序偏移量，并在使用包围框还是实例原点进行深度排序之间进行切换。
void RendererSceneCull::instance_set_pivot_data(RID p_instance, float p_sorting_offset, bool p_use_aabb_center) {
  Instance* instance = instance_owner.get_or_null(p_instance);
  ERR_FAIL_NULL(instance);

  instance->sorting_offset = p_sorting_offset;
  instance->use_aabb_center = p_use_aabb_center;

  if ((1 << instance->base_type) & RS::INSTANCE_GEOMETRY_MASK && instance->base_data) {
    InstanceGeometryData* geom = static_cast<InstanceGeometryData*>(instance->base_data);
    ERR_FAIL_NULL(geom->geometry_instance);
    geom->geometry_instance->set_pivot_data(p_sorting_offset, p_use_aabb_center);
  } else if (instance->base_type == RS::INSTANCE_DECAL && instance->base_data) {
    InstanceDecalData* decal = static_cast<InstanceDecalData*>(instance->base_data);
    // RSG::texture_storage->decal_instance_set_sorting_offset(decal->instance, instance->sorting_offset);
  }
}

void RendererSceneCull::instance_set_transform(RID p_instance, const Transform3D& p_transform) {
  Instance* instance = instance_owner.get_or_null(p_instance);
  ERR_FAIL_NULL(instance);

  if (instance->transform == p_transform) {
    return;  //must be checked to avoid worst evil
  }

#ifdef DEBUG_ENABLED

  for (int i = 0; i < 4; i++) {
    const Vector3& v = i < 3 ? p_transform.basis.rows[i] : p_transform.origin;
    ERR_FAIL_COND(!v.is_finite());
  }

#endif
  instance->transform = p_transform;
  _instance_queue_update(instance, true);
}
void RendererSceneCull::_instance_queue_update(Instance* p_instance, bool p_update_aabb, bool p_update_dependencies) {
  if (p_update_aabb) {
    p_instance->update_aabb = true;
  }
  if (p_update_dependencies) {
    p_instance->update_dependencies = true;
  }

  if (p_instance->update_item.in_list()) {
    return;
  }
  _instance_update_list.add(&p_instance->update_item);
}

void RendererSceneCull::instance_attach_object_instance_id(RID p_instance, ObjectID p_id) {
  Instance* instance = instance_owner.get_or_null(p_instance);
  ERR_FAIL_NULL(instance);

  instance->object_id = p_id;
}
void RendererSceneCull::instance_set_blend_shape_weight(RID p_instance, int p_shape, float p_weight) {
  Instance* instance = instance_owner.get_or_null(p_instance);
  ERR_FAIL_NULL(instance);

  if (instance->update_item.in_list()) {
    _update_dirty_instance(instance);
  }

  if (instance->mesh_instance.is_valid()) {
    RSG::mesh_storage->mesh_instance_set_blend_shape_weight(instance->mesh_instance, p_shape, p_weight);
  }

  _instance_queue_update(instance, false, false);
}

void RendererSceneCull::instance_set_surface_override_material(RID p_instance, int p_surface, RID p_material) {
  Instance* instance = instance_owner.get_or_null(p_instance);
  ERR_FAIL_NULL(instance);

  if (instance->base_type == RS::INSTANCE_MESH) {
    //may not have been updated yet, may also have not been set yet. When updated will be correcte, worst case
    instance->materials.resize(MAX(p_surface + 1, RSG::mesh_storage->mesh_get_surface_count(instance->base)));
  }

  ERR_FAIL_INDEX(p_surface, instance->materials.size());

  instance->materials.write[p_surface] = p_material;

  _instance_queue_update(instance, false, true);
}
void RendererSceneCull::instance_set_visible(RID p_instance, bool p_visible) {
  Instance* instance = instance_owner.get_or_null(p_instance);
  ERR_FAIL_NULL(instance);

  if (instance->visible == p_visible) {
    return;
  }

  instance->visible = p_visible;

  if (p_visible) {
    if (instance->scenario != nullptr) {
      _instance_queue_update(instance, true, false);
    }
  } else if (instance->indexer_id.is_valid()) {
    _unpair_instance(instance);
  }

  if (instance->base_type == RS::INSTANCE_LIGHT) {
    InstanceLightData* light = static_cast<InstanceLightData*>(instance->base_data);
    if (instance->scenario && RSG::light_storage->light_get_type(instance->base) != RS::LIGHT_DIRECTIONAL && light->bake_mode == RS::LIGHT_BAKE_DYNAMIC) {
      if (p_visible) {
        instance->scenario->dynamic_lights.push_back(light->instance);
      } else {
        instance->scenario->dynamic_lights.erase(light->instance);
      }
    }
  }

  if (instance->base_type == RS::INSTANCE_PARTICLES_COLLISION) {
    InstanceParticlesCollisionData* collision = static_cast<InstanceParticlesCollisionData*>(instance->base_data);
    // RSG::particles_storage->particles_collision_instance_set_active(collision->instance, p_visible);
  }

  if (instance->base_type == RS::INSTANCE_FOG_VOLUME) {
    InstanceFogVolumeData* volume = static_cast<InstanceFogVolumeData*>(instance->base_data);
    // scene_render->fog_volume_instance_set_active(volume->instance, p_visible);
  }

  if (instance->base_type == RS::INSTANCE_OCCLUDER) {
    if (instance->scenario) {
      RendererSceneOcclusionCull::get_singleton()->scenario_set_instance(instance->scenario->self, p_instance, instance->base, instance->transform, p_visible);
    }
  }
}

void RendererSceneCull::instance_geometry_set_transparency(RID p_instance, float p_transparency) {
  Instance* instance = instance_owner.get_or_null(p_instance);
  ERR_FAIL_NULL(instance);

  instance->transparency = p_transparency;

  if ((1 << instance->base_type) & RS::INSTANCE_GEOMETRY_MASK && instance->base_data) {
    InstanceGeometryData* geom = static_cast<InstanceGeometryData*>(instance->base_data);
    ERR_FAIL_NULL(geom->geometry_instance);
    geom->geometry_instance->set_transparency(p_transparency);
  }
}

void RendererSceneCull::instance_set_custom_aabb(RID p_instance, AABB p_aabb) {
  Instance* instance = instance_owner.get_or_null(p_instance);
  ERR_FAIL_NULL(instance);

  if (p_aabb != AABB()) {
    // Set custom AABB
    if (instance->custom_aabb == nullptr) {
      instance->custom_aabb = memnew(AABB);
    }
    *instance->custom_aabb = p_aabb;

  } else {
    // Clear custom AABB
    if (instance->custom_aabb != nullptr) {
      memdelete(instance->custom_aabb);
      instance->custom_aabb = nullptr;
    }
  }

  if (instance->scenario) {
    _instance_queue_update(instance, true, false);
  }
}
//设置将对象从视锥中剔除时为 AABB 增加的边距大小。这样就可以避免剔除落在视锥外的对象
void RendererSceneCull::instance_set_extra_visibility_margin(RID p_instance, real_t p_margin) {
  Instance* instance = instance_owner.get_or_null(p_instance);
  ERR_FAIL_NULL(instance);

  instance->extra_margin = p_margin;
  _instance_queue_update(instance, true, false);
}

void RendererSceneCull::instance_set_ignore_culling(RID p_instance, bool p_enabled) {
  Instance* instance = instance_owner.get_or_null(p_instance);
  ERR_FAIL_NULL(instance);
  instance->ignore_all_culling = p_enabled;

  if (instance->scenario && instance->array_index >= 0) {
    InstanceData& idata = instance->scenario->instance_data[instance->array_index];
    if (instance->ignore_all_culling) {
      idata.flags |= InstanceData::FLAG_IGNORE_ALL_CULLING;
    } else {
      idata.flags &= ~uint32_t(InstanceData::FLAG_IGNORE_ALL_CULLING);
    }
  }
}

void RendererSceneCull::_instance_update_mesh_instance(Instance* p_instance) {}

void RendererSceneCull::update() {
  //optimize bvhs

  uint32_t rid_count = scenario_owner.get_rid_count();
  RID* rids = (RID*)alloca(sizeof(RID) * rid_count);
  scenario_owner.fill_owned_buffer(rids);
  for (uint32_t i = 0; i < rid_count; i++) {
    Scenario* s = scenario_owner.get_or_null(rids[i]);
    s->indexers[Scenario::INDEXER_GEOMETRY].optimize_incremental(indexer_update_iterations);
    s->indexers[Scenario::INDEXER_VOLUMES].optimize_incremental(indexer_update_iterations);
  }
  scene_render->update();
  update_dirty_instances();
  render_particle_colliders();
}

void RendererSceneCull::instance_attach_skeleton(RID p_instance, RID p_skeleton) {
  Instance* instance = instance_owner.get_or_null(p_instance);
  ERR_FAIL_NULL(instance);

  if (instance->skeleton == p_skeleton) {
    return;
  }

  instance->skeleton = p_skeleton;

  if (p_skeleton.is_valid()) {
    //update the dependency now, so if cleared, we remove it
    RSG::mesh_storage->skeleton_update_dependency(p_skeleton, &instance->dependency_tracker);
  }

  _instance_queue_update(instance, true, true);

  if ((1 << instance->base_type) & RS::INSTANCE_GEOMETRY_MASK && instance->base_data) {
    _instance_update_mesh_instance(instance);

    InstanceGeometryData* geom = static_cast<InstanceGeometryData*>(instance->base_data);
    ERR_FAIL_NULL(geom->geometry_instance);
    geom->geometry_instance->set_skeleton(p_skeleton);
  }
}
void RendererSceneCull::instance_set_visibility_parent(RID p_instance, RID p_parent_instance) {
  Instance* instance = instance_owner.get_or_null(p_instance);
  ERR_FAIL_NULL(instance);

  Instance* old_parent = instance->visibility_parent;
  if (old_parent) {
    old_parent->visibility_dependencies.erase(instance);
    instance->visibility_parent = nullptr;
    _update_instance_visibility_depth(old_parent);
  }

  Instance* parent = instance_owner.get_or_null(p_parent_instance);
  ERR_FAIL_COND(p_parent_instance.is_valid() && !parent);

  if (parent) {
    parent->visibility_dependencies.insert(instance);
    instance->visibility_parent = parent;

    bool cycle_detected = _update_instance_visibility_depth(parent);
    if (cycle_detected) {
      ERR_PRINT("Cycle detected in the visibility dependencies tree. The latest change to visibility_parent will have no effect.");
      parent->visibility_dependencies.erase(instance);
      instance->visibility_parent = nullptr;
    }
  }

  _update_instance_visibility_dependencies(instance);
}

bool RendererSceneCull::_update_instance_visibility_depth(Instance* p_instance) {
  bool cycle_detected = false;
  HashSet<Instance*> traversed_nodes;

  {
    Instance* instance = p_instance;
    while (instance) {
      if (!instance->visibility_dependencies.is_empty()) {
        uint32_t depth = 0;
        for (const Instance* E : instance->visibility_dependencies) {
          depth = MAX(depth, E->visibility_dependencies_depth);
        }
        instance->visibility_dependencies_depth = depth + 1;
      } else {
        instance->visibility_dependencies_depth = 0;
      }

      if (instance->scenario && instance->visibility_index != -1) {
        instance->scenario->instance_visibility.move(instance->visibility_index, instance->visibility_dependencies_depth);
      }

      traversed_nodes.insert(instance);

      instance = instance->visibility_parent;
      if (traversed_nodes.has(instance)) {
        cycle_detected = true;
        break;
      }
    }
  }

  return cycle_detected;
}

void RendererSceneCull::_update_instance_visibility_dependencies(Instance* p_instance) {
  bool is_geometry_instance = ((1 << p_instance->base_type) & RS::INSTANCE_GEOMETRY_MASK) && p_instance->base_data;
  bool has_visibility_range = p_instance->visibility_range_begin > 0.0 || p_instance->visibility_range_end > 0.0;
  bool needs_visibility_cull = has_visibility_range && is_geometry_instance && p_instance->array_index != -1;

  if (!needs_visibility_cull && p_instance->visibility_index != -1) {
    p_instance->scenario->instance_visibility.remove_at(p_instance->visibility_index);
    p_instance->visibility_index = -1;
  } else if (needs_visibility_cull && p_instance->visibility_index == -1) {
    InstanceVisibilityData vd;
    vd.instance = p_instance;
    vd.range_begin = p_instance->visibility_range_begin;
    vd.range_end = p_instance->visibility_range_end;
    vd.range_begin_margin = p_instance->visibility_range_begin_margin;
    vd.range_end_margin = p_instance->visibility_range_end_margin;
    vd.position = p_instance->transformed_aabb.get_center();
    vd.array_index = p_instance->array_index;
    vd.fade_mode = p_instance->visibility_range_fade_mode;

    p_instance->scenario->instance_visibility.insert(vd, p_instance->visibility_dependencies_depth);
  }

  if (p_instance->scenario && p_instance->array_index != -1) {
    InstanceData& idata = p_instance->scenario->instance_data[p_instance->array_index];
    idata.visibility_index = p_instance->visibility_index;

    if (is_geometry_instance) {
      if (has_visibility_range && p_instance->visibility_range_fade_mode == RS::VISIBILITY_RANGE_FADE_SELF) {
        bool begin_enabled = p_instance->visibility_range_begin > 0.0f;
        float begin_min = p_instance->visibility_range_begin - p_instance->visibility_range_begin_margin;
        float begin_max = p_instance->visibility_range_begin + p_instance->visibility_range_begin_margin;
        bool end_enabled = p_instance->visibility_range_end > 0.0f;
        float end_min = p_instance->visibility_range_end - p_instance->visibility_range_end_margin;
        float end_max = p_instance->visibility_range_end + p_instance->visibility_range_end_margin;
        idata.instance_geometry->set_fade_range(begin_enabled, begin_min, begin_max, end_enabled, end_min, end_max);
      } else {
        idata.instance_geometry->set_fade_range(false, 0.0f, 0.0f, false, 0.0f, 0.0f);
      }
    }

    if ((has_visibility_range || p_instance->visibility_parent) && (p_instance->visibility_index == -1 || p_instance->visibility_dependencies_depth == 0)) {
      idata.flags |= InstanceData::FLAG_VISIBILITY_DEPENDENCY_NEEDS_CHECK;
    } else {
      idata.flags &= ~InstanceData::FLAG_VISIBILITY_DEPENDENCY_NEEDS_CHECK;
    }

    if (p_instance->visibility_parent) {
      idata.vis_parent_array_index = p_instance->visibility_parent->array_index;
    } else {
      idata.vis_parent_array_index = -1;
      if (is_geometry_instance) {
        idata.instance_geometry->set_parent_fade_alpha(1.0f);
      }
    }
  }
}

void RendererSceneCull::update_dirty_instances() {
  while (_instance_update_list.first()) {
    _update_dirty_instance(_instance_update_list.first()->self());
  }

  // Update dirty resources after dirty instances as instance updates may affect resources.
  RSG::utilities->update_dirty_resources();
}

void RendererSceneCull::_update_dirty_instance(Instance* p_instance) {
  if (p_instance->update_aabb) {
    _update_instance_aabb(p_instance);  // 根据实例类型调用不同的update AABB
  }
  if (p_instance->update_dependencies) {
    p_instance->dependency_tracker.update_begin();

    if (p_instance->base.is_valid()) {
      RSG::utilities->base_update_dependency(p_instance->base, &p_instance->dependency_tracker);
    }

    if (p_instance->material_override.is_valid()) {
      RSG::material_storage->material_update_dependency(p_instance->material_override, &p_instance->dependency_tracker);
    }

    if (p_instance->material_overlay.is_valid()) {
      RSG::material_storage->material_update_dependency(p_instance->material_overlay, &p_instance->dependency_tracker);
    }

    if (p_instance->base_type == RS::INSTANCE_MESH) {
      //remove materials no longer used and un-own them

      int new_mat_count = RSG::mesh_storage->mesh_get_surface_count(p_instance->base);
      p_instance->materials.resize(new_mat_count);

      _instance_update_mesh_instance(p_instance);
    }

    // if (p_instance->base_type == RS::INSTANCE_PARTICLES) {
    //   // update the process material dependency

    //   RID particle_material = RSG::particles_storage->particles_get_process_material(p_instance->base);
    //   if (particle_material.is_valid()) {
    //     RSG::material_storage->material_update_dependency(particle_material, &p_instance->dependency_tracker);
    //   }
    // }

    if ((1 << p_instance->base_type) & RS::INSTANCE_GEOMETRY_MASK) {
      InstanceGeometryData* geom = static_cast<InstanceGeometryData*>(p_instance->base_data);

      bool can_cast_shadows = true;
      bool is_animated = false;
      HashMap<StringName, Instance::InstanceShaderParameter> isparams;
      // 下面更新can_cast_shadows, is_animated, isparams
      if (p_instance->cast_shadows == RS::SHADOW_CASTING_SETTING_OFF) {
        can_cast_shadows = false;
      }

      if (p_instance->material_override.is_valid()) {
        if (!RSG::material_storage->material_casts_shadows(p_instance->material_override)) {
          can_cast_shadows = false;
        }
        is_animated = RSG::material_storage->material_is_animated(p_instance->material_override);
        _update_instance_shader_uniforms_from_material(isparams, p_instance->instance_shader_uniforms, p_instance->material_override);  // 使用material_override
      } else {                                                                                                                          // no override
        if (p_instance->base_type == RS::INSTANCE_MESH) {
          RID mesh = p_instance->base;

          if (mesh.is_valid()) {
            bool cast_shadows = false;

            for (int i = 0; i < p_instance->materials.size(); i++) {
              // material 可以通过 本地的 materials 或者 mesh_surface_get_material 获取
              // 需要保证顺序是一致的 @潜在的bug (mesh_surface_get_material[i] != materials[i])
              RID mat = p_instance->materials[i].is_valid() ? p_instance->materials[i] : RSG::mesh_storage->mesh_surface_get_material(mesh, i);

              if (!mat.is_valid()) {
                cast_shadows = true;
              } else {
                if (RSG::material_storage->material_casts_shadows(mat)) {
                  cast_shadows = true;
                }

                if (RSG::material_storage->material_is_animated(mat)) {
                  is_animated = true;
                }

                _update_instance_shader_uniforms_from_material(isparams, p_instance->instance_shader_uniforms, mat);

                RSG::material_storage->material_update_dependency(mat, &p_instance->dependency_tracker);
              }
            }

            if (!cast_shadows) {
              can_cast_shadows = false;
            }
          }
        }  // end of mesh
        else if (p_instance->base_type == RS::INSTANCE_MULTIMESH) {
          RID mesh = RSG::mesh_storage->multimesh_get_mesh(p_instance->base);
          if (mesh.is_valid()) {
            bool cast_shadows = false;

            int sc = RSG::mesh_storage->mesh_get_surface_count(mesh);
            for (int i = 0; i < sc; i++) {
              RID mat = RSG::mesh_storage->mesh_surface_get_material(mesh, i);

              if (!mat.is_valid()) {
                cast_shadows = true;

              } else {
                if (RSG::material_storage->material_casts_shadows(mat)) {
                  cast_shadows = true;
                }
                if (RSG::material_storage->material_is_animated(mat)) {
                  is_animated = true;
                }

                _update_instance_shader_uniforms_from_material(isparams, p_instance->instance_shader_uniforms, mat);

                RSG::material_storage->material_update_dependency(mat, &p_instance->dependency_tracker);
              }
            }

            if (!cast_shadows) {
              can_cast_shadows = false;
            }

            RSG::utilities->base_update_dependency(mesh, &p_instance->dependency_tracker);
          }
        }  // end of multimesh
        // } else if (p_instance->base_type == RS::INSTANCE_PARTICLES) {
        //   bool cast_shadows = false;

        //   int dp = RSG::particles_storage->particles_get_draw_passes(p_instance->base);

        //   for (int i = 0; i < dp; i++) {
        //     RID mesh = RSG::particles_storage->particles_get_draw_pass_mesh(p_instance->base, i);
        //     if (!mesh.is_valid()) {
        //       continue;
        //     }

        //     int sc = RSG::mesh_storage->mesh_get_surface_count(mesh);
        //     for (int j = 0; j < sc; j++) {
        //       RID mat = RSG::mesh_storage->mesh_surface_get_material(mesh, j);

        //       if (!mat.is_valid()) {
        //         cast_shadows = true;
        //       } else {
        //         if (RSG::material_storage->material_casts_shadows(mat)) {
        //           cast_shadows = true;
        //         }

        //         if (RSG::material_storage->material_is_animated(mat)) {
        //           is_animated = true;
        //         }

        //         _update_instance_shader_uniforms_from_material(isparams, p_instance->instance_shader_uniforms, mat);

        //         RSG::material_storage->material_update_dependency(mat, &p_instance->dependency_tracker);
        //       }
        //     }
        //   }

        //   if (!cast_shadows) {
        //     can_cast_shadows = false;
        //   }
        // }
      }

      if (p_instance->material_overlay.is_valid()) {
        can_cast_shadows = can_cast_shadows && RSG::material_storage->material_casts_shadows(p_instance->material_overlay);
        is_animated = is_animated || RSG::material_storage->material_is_animated(p_instance->material_overlay);
        _update_instance_shader_uniforms_from_material(isparams, p_instance->instance_shader_uniforms, p_instance->material_overlay);
      }

      if (can_cast_shadows != geom->can_cast_shadows) {
        //ability to cast shadows change, let lights now
        for (const Instance* E : geom->lights) {
          InstanceLightData* light = static_cast<InstanceLightData*>(E->base_data);
          light->make_shadow_dirty();
        }

        geom->can_cast_shadows = can_cast_shadows;
      }

      geom->material_is_animated = is_animated;
      p_instance->instance_shader_uniforms = isparams;

      if (p_instance->instance_allocated_shader_uniforms != (p_instance->instance_shader_uniforms.size() > 0)) {
        p_instance->instance_allocated_shader_uniforms = (p_instance->instance_shader_uniforms.size() > 0);
        if (p_instance->instance_allocated_shader_uniforms) {
          p_instance->instance_allocated_shader_uniforms_offset = RSG::material_storage->global_shader_parameters_instance_allocate(p_instance->self);
          ERR_FAIL_NULL(geom->geometry_instance);
          geom->geometry_instance->set_instance_shader_uniforms_offset(p_instance->instance_allocated_shader_uniforms_offset);

          for (const KeyValue<StringName, Instance::InstanceShaderParameter>& E : p_instance->instance_shader_uniforms) {
            if (E.value.value.get_type() != Variant::NIL) {
              int flags_count = 0;
              if (E.value.info.hint == PROPERTY_HINT_FLAGS) {
                // A small hack to detect boolean flags count and prevent overhead.
                switch (E.value.info.hint_string.length()) {
                  case 3:  // "x,y"
                    flags_count = 1;
                    break;
                  case 5:  // "x,y,z"
                    flags_count = 2;
                    break;
                  case 7:  // "x,y,z,w"
                    flags_count = 3;
                    break;
                }
              }
              // uniform的更新
              RSG::material_storage->global_shader_parameters_instance_update(p_instance->self, E.value.index, E.value.value, flags_count);
            }
          }
        } else {  // !instance_allocated_shader_uniforms
          RSG::material_storage->global_shader_parameters_instance_free(p_instance->self);
          p_instance->instance_allocated_shader_uniforms_offset = -1;
          ERR_FAIL_NULL(geom->geometry_instance);
          geom->geometry_instance->set_instance_shader_uniforms_offset(-1);
        }
      }
    }

    if (p_instance->skeleton.is_valid()) {
      RSG::mesh_storage->skeleton_update_dependency(p_instance->skeleton, &p_instance->dependency_tracker);
    }

    p_instance->dependency_tracker.update_end();

    if ((1 << p_instance->base_type) & RS::INSTANCE_GEOMETRY_MASK) {
      InstanceGeometryData* geom = static_cast<InstanceGeometryData*>(p_instance->base_data);
      ERR_FAIL_NULL(geom->geometry_instance);
      geom->geometry_instance->set_surface_materials(p_instance->materials);
    }
  }
}

// 根据 material 那边提供的shader 参数列表进行更新传入的isparam。
void RendererSceneCull::_update_instance_shader_uniforms_from_material(HashMap<StringName, Instance::InstanceShaderParameter>& isparams,
                                                                       const HashMap<StringName, Instance::InstanceShaderParameter>& existing_isparams, RID p_material) {
  List<RendererMaterialStorage::InstanceShaderParam> plist;
  RSG::material_storage->material_get_instance_shader_parameters(p_material, &plist);
  for (const RendererMaterialStorage::InstanceShaderParam& E : plist) {
    StringName name = E.info.name;
    if (isparams.has(name)) {
      if (isparams[name].info.type != E.info.type) {
        WARN_PRINT("More than one material in instance export the same instance shader uniform '" + E.info.name +
                   "', but they do it with different data types. Only the first one (in order) will display correctly.");
      }
      if (isparams[name].index != E.index) {
        WARN_PRINT("More than one material in instance export the same instance shader uniform '" + E.info.name +
                   "', but they do it with different indices. Only the first one (in order) will display correctly.");
      }
      continue;  //first one found always has priority
    }

    Instance::InstanceShaderParameter isp;
    isp.index = E.index;
    isp.info = E.info;
    isp.default_value = E.default_value;
    if (existing_isparams.has(name)) {
      isp.value = existing_isparams[name].value;
    } else {
      isp.value = E.default_value;
    }
    isparams[name] = isp;
  }
}
/* HALTON SEQUENCE */

#ifndef _3D_DISABLED
static float get_halton_value(int p_index, int p_base) {
  float f = 1;
  float r = 0;
  while (p_index > 0) {
    f = f / static_cast<float>(p_base);
    r = r + f * (p_index % p_base);
    p_index = p_index / p_base;
  }
  return r * 2.0f - 1.0f;
}
#endif  // _3D_DISABLED

void RendererSceneCull::render_camera(const Ref<RenderSceneBuffers>& p_render_buffers, RID p_camera, RID p_scenario, RID p_viewport, Size2 p_viewport_size,
                                      uint32_t p_jitter_phase_count, float p_screen_mesh_lod_threshold, RID p_shadow_atlas, RenderInfo* r_render_info) {

  Camera* camera = camera_owner.get_or_null(p_camera);
  ERR_FAIL_NULL(camera);

  Vector2 jitter;
  if (p_jitter_phase_count > 0) {
    uint32_t current_jitter_count = camera_jitter_array.size();
    if (p_jitter_phase_count != current_jitter_count) {
      // Resize the jitter array and fill it with the pre-computed Halton sequence.
      camera_jitter_array.resize(p_jitter_phase_count);

      for (uint32_t i = current_jitter_count; i < p_jitter_phase_count; i++) {
        camera_jitter_array[i].x = get_halton_value(i, 2);  //https://en.wikipedia.org/wiki/Halton_sequence
        camera_jitter_array[i].y = get_halton_value(i, 3);
      }
    }

    jitter = camera_jitter_array[RSG::rasterizer->get_frame_number() % p_jitter_phase_count] / p_viewport_size;
  }

  RendererSceneRender::CameraData camera_data;
  // Normal camera
  Transform3D transform = camera->transform;
  Projection projection;
  bool vaspect = camera->vaspect;
  bool is_orthogonal = false;

  switch (camera->type) {
    case Camera::ORTHOGONAL: {
      projection.set_orthogonal(camera->size, p_viewport_size.width() / (float)p_viewport_size.height(), camera->znear, camera->zfar, camera->vaspect);
      is_orthogonal = true;
    } break;
    case Camera::PERSPECTIVE: {
      projection.set_perspective(camera->fov, p_viewport_size.width() / (float)p_viewport_size.height(), camera->znear, camera->zfar, camera->vaspect);

    } break;
    case Camera::FRUSTUM: {
      projection.set_frustum(camera->size, p_viewport_size.width() / (float)p_viewport_size.height(), camera->offset, camera->znear, camera->zfar, camera->vaspect);
    } break;
  }

  camera_data.set_camera(transform, projection, is_orthogonal, vaspect, jitter, camera->visible_layers);
  RID environment = _render_get_environment(p_camera, p_scenario);
  RID compositor = _render_get_compositor(p_camera, p_scenario);

  RENDER_TIMESTAMP("Update Occlusion Buffer")
  // For now just cull on the first camera
  RendererSceneOcclusionCull::get_singleton()->buffer_update(p_viewport, camera_data.main_transform, camera_data.main_projection, camera_data.is_orthogonal);

  _render_scene(&camera_data, p_render_buffers, environment, camera->attributes, compositor, camera->visible_layers, p_scenario, p_viewport, p_shadow_atlas, RID(), -1,
                p_screen_mesh_lod_threshold, true, r_render_info);
}

RID RendererSceneCull::_render_get_compositor(RID p_camera, RID p_scenario) {
  Camera* camera = camera_owner.get_or_null(p_camera);
  if (camera && scene_render->is_compositor(camera->compositor)) {
    return camera->compositor;
  }

  Scenario* scenario = scenario_owner.get_or_null(p_scenario);
  if (scenario && scene_render->is_compositor(scenario->compositor)) {
    return scenario->compositor;
  }

  return RID();
}

void lain::RendererSceneCull::_render_scene(const RendererSceneRender::CameraData* p_camera_data, const Ref<RenderSceneBuffers>& p_render_buffers, RID p_environment,
                                            RID p_force_camera_attributes, RID p_compositor, uint32_t p_visible_layers, RID p_scenario, RID p_viewport, RID p_shadow_atlas,
                                            RID p_reflection_probe, int p_reflection_probe_pass, float p_screen_mesh_lod_threshold, bool p_using_shadows,
                                            RenderingMethod::RenderInfo* r_render_info) {
  // 填 camera 视锥体 的信息
  light_culler->prepare_camera(p_camera_data->main_transform, p_camera_data->main_projection);
  Scenario* scenario = scenario_owner.get_or_null(p_scenario);
  Vector3 camera_position = p_camera_data->main_transform.origin;

  ERR_FAIL_COND(p_render_buffers.is_null());

  render_pass++;
  scene_render->set_scene_pass(render_pass);

  RENDER_TIMESTAMP("Update Visibility Dependencies");
  if (scenario->instance_visibility.get_bin_count() > 0) {
    if (!scenario->viewport_visibility_masks.has(p_viewport)) {
      scenario_add_viewport_visibility_mask(scenario->self, p_viewport);  // 等价于 p_senario 吧
    }

    VisibilityCullData visibility_cull_data;
    visibility_cull_data.scenario = scenario;
    visibility_cull_data.viewport_mask = scenario->viewport_visibility_masks[p_viewport];
    visibility_cull_data.camera_position = camera_position;

    for (int i = scenario->instance_visibility.get_bin_count() - 1; i > 0; i--) {  // We skip bin 0
      visibility_cull_data.cull_offset = scenario->instance_visibility.get_bin_start(i);
      visibility_cull_data.cull_count = scenario->instance_visibility.get_bin_size(i);

      if (visibility_cull_data.cull_count == 0) {
        continue;
      }

      if (visibility_cull_data.cull_count > thread_cull_threshold) {
        WorkerThreadPool::GroupID group_task =
            WorkerThreadPool::get_singleton()->add_template_group_task(this, &RendererSceneCull::_visibility_cull_threaded, &visibility_cull_data,
                                                                       WorkerThreadPool::get_singleton()->get_thread_count(), -1, true, SNAME("VisibilityCullInstances"));
        WorkerThreadPool::get_singleton()->wait_for_group_task_completion(group_task);
      } else {
        _visibility_cull(visibility_cull_data, visibility_cull_data.cull_offset, visibility_cull_data.cull_offset + visibility_cull_data.cull_count);
      }
    }
  }
	RENDER_TIMESTAMP("Cull 3D Scene");
	/* STEP 2 - CULL */

  Vector<Plane> planes = p_camera_data->main_projection.get_projection_planes(p_camera_data->main_transform);
	cull.frustum = Frustum(planes);

	Vector<RID> directional_lights;
	// directional lights
	{
		cull.shadow_count = 0;

		Vector<Instance *> lights_with_shadow;

		for (Instance *E : scenario->directional_lights) {
			if (!E->visible) {
				continue;
			}

			if (directional_lights.size() > RendererSceneRender::MAX_DIRECTIONAL_LIGHTS) {
				break;
			}

			InstanceLightData *light = static_cast<InstanceLightData *>(E->base_data);

			//check shadow..

			if (light) {
				if (p_using_shadows && p_shadow_atlas.is_valid() && RSG::light_storage->light_has_shadow(E->base) && !(RSG::light_storage->light_get_type(E->base) == RS::LIGHT_DIRECTIONAL && RSG::light_storage->light_directional_get_sky_mode(E->base) == RS::LIGHT_DIRECTIONAL_SKY_MODE_SKY_ONLY)) {
					lights_with_shadow.push_back(E);
				}
				//add to list
				directional_lights.push_back(light->instance);
			}
		}

		RSG::light_storage->set_directional_shadow_count(lights_with_shadow.size());

		for (int i = 0; i < lights_with_shadow.size(); i++) {
			_light_instance_setup_directional_shadow(i, lights_with_shadow[i], p_camera_data->main_transform, p_camera_data->main_projection, p_camera_data->is_orthogonal, p_camera_data->vaspect);
		}
	}
}

RID RendererSceneCull::_render_get_environment(RID p_camera, RID p_scenario) {
  Camera* camera = camera_owner.get_or_null(p_camera);
  if (camera && scene_render->is_environment(camera->env)) {
    return camera->env;
  }

  Scenario* scenario = scenario_owner.get_or_null(p_scenario);
  if (!scenario) {
    return RID();
  }
  if (scene_render->is_environment(scenario->environment)) {
    return scenario->environment;
  }

  if (scene_render->is_environment(scenario->fallback_environment)) {
    return scenario->fallback_environment;
  }

  return RID();
}

void RendererSceneCull::_visibility_cull_threaded(uint32_t p_thread, VisibilityCullData* cull_data) {
  uint32_t total_threads = WorkerThreadPool::get_singleton()->get_thread_count();
  uint32_t bin_from = p_thread * cull_data->cull_count / total_threads;
  uint32_t bin_to = (p_thread + 1 == total_threads) ? cull_data->cull_count : ((p_thread + 1) * cull_data->cull_count / total_threads);

  _visibility_cull(*cull_data, cull_data->cull_offset + bin_from, cull_data->cull_offset + bin_to);
}

void RendererSceneCull::_visibility_cull(const VisibilityCullData& cull_data, uint64_t p_from, uint64_t p_to) {
  Scenario* scenario = cull_data.scenario;
  for (unsigned int i = p_from; i < p_to; i++) {
    InstanceVisibilityData& vd = scenario->instance_visibility[i];
    InstanceData& idata = scenario->instance_data[vd.array_index];

    if (idata.vis_parent_array_index >= 0) {
      uint32_t parent_flags = scenario->instance_data[idata.vis_parent_array_index].flags;

      if ((parent_flags & InstanceData::FLAG_VISIBILITY_DEPENDENCY_HIDDEN) ||
          !(parent_flags & (InstanceData::FLAG_VISIBILITY_DEPENDENCY_HIDDEN_CLOSE_RANGE | InstanceData::FLAG_VISIBILITY_DEPENDENCY_FADE_CHILDREN))) {
        idata.flags |= InstanceData::FLAG_VISIBILITY_DEPENDENCY_HIDDEN;
        idata.flags &= ~InstanceData::FLAG_VISIBILITY_DEPENDENCY_HIDDEN_CLOSE_RANGE;
        idata.flags &= ~InstanceData::FLAG_VISIBILITY_DEPENDENCY_FADE_CHILDREN;
        continue;
      }
    }

    int range_check = _visibility_range_check<true>(vd, cull_data.camera_position, cull_data.viewport_mask);

    if (range_check == -1) {
      idata.flags |= InstanceData::FLAG_VISIBILITY_DEPENDENCY_HIDDEN;
      idata.flags &= ~InstanceData::FLAG_VISIBILITY_DEPENDENCY_HIDDEN_CLOSE_RANGE;
      idata.flags &= ~InstanceData::FLAG_VISIBILITY_DEPENDENCY_FADE_CHILDREN;
    } else if (range_check == 1) {
      idata.flags &= ~InstanceData::FLAG_VISIBILITY_DEPENDENCY_HIDDEN;
      idata.flags |= InstanceData::FLAG_VISIBILITY_DEPENDENCY_HIDDEN_CLOSE_RANGE;
      idata.flags &= ~InstanceData::FLAG_VISIBILITY_DEPENDENCY_FADE_CHILDREN;
    } else {
      idata.flags &= ~InstanceData::FLAG_VISIBILITY_DEPENDENCY_HIDDEN;
      idata.flags &= ~InstanceData::FLAG_VISIBILITY_DEPENDENCY_HIDDEN_CLOSE_RANGE;
      if (range_check == 2) {
        idata.flags |= InstanceData::FLAG_VISIBILITY_DEPENDENCY_FADE_CHILDREN;
      } else { // 0
        idata.flags &= ~InstanceData::FLAG_VISIBILITY_DEPENDENCY_FADE_CHILDREN;
      }
    }
  }
}

template <bool p_fade_check>
_FORCE_INLINE_ int RendererSceneCull::_visibility_range_check(InstanceVisibilityData& r_vis_data, const Vector3& p_camera_pos, uint64_t p_viewport_mask) {
  float dist = p_camera_pos.distance_to(r_vis_data.position);
  const RS::VisibilityRangeFadeMode& fade_mode = r_vis_data.fade_mode;

  float begin_offset = -r_vis_data.range_begin_margin;
  float end_offset = r_vis_data.range_end_margin;

  if (fade_mode == RS::VISIBILITY_RANGE_FADE_DISABLED && !(p_viewport_mask & r_vis_data.viewport_state)) {
    begin_offset = -begin_offset;
    end_offset = -end_offset;
  }

  if (r_vis_data.range_end > 0.0f && dist > r_vis_data.range_end + end_offset) {
    r_vis_data.viewport_state &= ~p_viewport_mask;
    return -1; // 太远
  } else if (r_vis_data.range_begin > 0.0f && dist < r_vis_data.range_begin + begin_offset) {
    r_vis_data.viewport_state &= ~p_viewport_mask;
    return 1; // 太近
  } else {
    r_vis_data.viewport_state |= p_viewport_mask;
    if (p_fade_check) {
      if (fade_mode != RS::VISIBILITY_RANGE_FADE_DISABLED) {
        r_vis_data.children_fade_alpha = 1.0f;
        if (r_vis_data.range_end > 0.0f && dist > r_vis_data.range_end - end_offset) {
          if (fade_mode == RS::VISIBILITY_RANGE_FADE_DEPENDENCIES) {
            r_vis_data.children_fade_alpha = MIN(1.0f, (dist - (r_vis_data.range_end - end_offset)) / (2.0f * r_vis_data.range_end_margin));
          }
          return 2;
        } else if (r_vis_data.range_begin > 0.0f && dist < r_vis_data.range_begin - begin_offset) {
          if (fade_mode == RS::VISIBILITY_RANGE_FADE_DEPENDENCIES) {
            r_vis_data.children_fade_alpha = MIN(1.0f, 1.0 - (dist - (r_vis_data.range_begin + begin_offset)) / (2.0f * r_vis_data.range_begin_margin));
          }
          return 2;
        }
      }
    }
    return 0;
  }
}

void RendererSceneCull::_light_instance_setup_directional_shadow(int p_shadow_index, Instance *p_instance, const Transform3D p_cam_transform, const Projection &p_cam_projection, bool p_cam_orthogonal, bool p_cam_vaspect) {
	// For later tight culling, the light culler needs to know the details of the directional light.
	light_culler->prepare_directional_light(p_instance, p_shadow_index);

	InstanceLightData *light = static_cast<InstanceLightData *>(p_instance->base_data);

	Transform3D light_transform = p_instance->transform;
	light_transform.orthonormalize(); //scale does not count on lights

	real_t max_distance = p_cam_projection.get_z_far();
	real_t shadow_max = RSG::light_storage->light_get_param(p_instance->base, RS::LIGHT_PARAM_SHADOW_MAX_DISTANCE);
	if (shadow_max > 0 && !p_cam_orthogonal) { //its impractical (and leads to unwanted behaviors) to set max distance in orthogonal camera
		max_distance = MIN(shadow_max, max_distance);
	}
	max_distance = MAX(max_distance, p_cam_projection.get_z_near() + 0.001);
	real_t min_distance = MIN(p_cam_projection.get_z_near(), max_distance);

	real_t pancake_size = RSG::light_storage->light_get_param(p_instance->base, RS::LIGHT_PARAM_SHADOW_PANCAKE_SIZE);

	real_t range = max_distance - min_distance;

	int splits = 0;
	switch (RSG::light_storage->light_directional_get_shadow_mode(p_instance->base)) {
		case RS::LIGHT_DIRECTIONAL_SHADOW_ORTHOGONAL:
			splits = 1;
			break;
		case RS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_2_SPLITS:
			splits = 2;
			break;
		case RS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_4_SPLITS:
			splits = 4;
			break;
	}

	real_t distances[5];

	distances[0] = min_distance;
	for (int i = 0; i < splits; i++) {
		distances[i + 1] = min_distance + RSG::light_storage->light_get_param(p_instance->base, RS::LightParam(RS::LIGHT_PARAM_SHADOW_SPLIT_1_OFFSET + i)) * range;
	};

	distances[splits] = max_distance;

	real_t texture_size = RSG::light_storage->get_directional_light_shadow_size(light->instance);

	bool overlap = RSG::light_storage->light_directional_get_blend_splits(p_instance->base);

	cull.shadow_count = p_shadow_index + 1;
	cull.shadows[p_shadow_index].cascade_count = splits;
	cull.shadows[p_shadow_index].light_instance = light->instance;

	for (int i = 0; i < splits; i++) {
		RENDER_TIMESTAMP("Cull DirectionalLight3D, Split " + itos(i));

		// setup a camera matrix for that range!
		Projection camera_matrix;

		real_t aspect = p_cam_projection.get_aspect();

		if (p_cam_orthogonal) {
			Vector2 vp_he = p_cam_projection.get_viewport_half_extents();

			camera_matrix.set_orthogonal(vp_he.y * 2.0, aspect, distances[(i == 0 || !overlap) ? i : i - 1], distances[i + 1], false);
		} else {
			real_t fov = p_cam_projection.get_fov(); //this is actually yfov, because set aspect tries to keep it
			camera_matrix.set_perspective(fov, aspect, distances[(i == 0 || !overlap) ? i : i - 1], distances[i + 1], true);
		}

		//obtain the frustum endpoints

		Vector3 endpoints[8]; // frustum plane endpoints
		bool res = camera_matrix.get_endpoints(p_cam_transform, endpoints);
		ERR_CONTINUE(!res);

		// obtain the light frustum ranges (given endpoints)

		Transform3D transform = light_transform; //discard scale and stabilize light

		Vector3 x_vec = transform.basis.get_column(Vector3::AXIS_X).normalized();
		Vector3 y_vec = transform.basis.get_column(Vector3::AXIS_Y).normalized();
		Vector3 z_vec = transform.basis.get_column(Vector3::AXIS_Z).normalized();
		//z_vec points against the camera, like in default opengl

		real_t x_min = 0.f, x_max = 0.f;
		real_t y_min = 0.f, y_max = 0.f;
		real_t z_min = 0.f, z_max = 0.f;

		// FIXME: z_max_cam is defined, computed, but not used below when setting up
		// ortho_camera. Commented out for now to fix warnings but should be investigated.
		real_t x_min_cam = 0.f, x_max_cam = 0.f;
		real_t y_min_cam = 0.f, y_max_cam = 0.f;
		real_t z_min_cam = 0.f;
		//real_t z_max_cam = 0.f;

		//real_t bias_scale = 1.0;
		//real_t aspect_bias_scale = 1.0;

		//used for culling

		for (int j = 0; j < 8; j++) {
			real_t d_x = x_vec.dot(endpoints[j]);
			real_t d_y = y_vec.dot(endpoints[j]);
			real_t d_z = z_vec.dot(endpoints[j]);

			if (j == 0 || d_x < x_min) {
				x_min = d_x;
			}
			if (j == 0 || d_x > x_max) {
				x_max = d_x;
			}

			if (j == 0 || d_y < y_min) {
				y_min = d_y;
			}
			if (j == 0 || d_y > y_max) {
				y_max = d_y;
			}

			if (j == 0 || d_z < z_min) {
				z_min = d_z;
			}
			if (j == 0 || d_z > z_max) {
				z_max = d_z;
			}
		}

		real_t radius = 0;
		real_t soft_shadow_expand = 0;
		Vector3 center;

		{
			//camera viewport stuff

			for (int j = 0; j < 8; j++) {
				center += endpoints[j];
			}
			center /= 8.0;

			//center=x_vec*(x_max-x_min)*0.5 + y_vec*(y_max-y_min)*0.5 + z_vec*(z_max-z_min)*0.5;

			for (int j = 0; j < 8; j++) {
				real_t d = center.distance_to(endpoints[j]);
				if (d > radius) {
					radius = d;
				}
			}

			radius *= texture_size / (texture_size - 2.0); //add a texel by each side

			z_min_cam = z_vec.dot(center) - radius;

			{
				float soft_shadow_angle = RSG::light_storage->light_get_param(p_instance->base, RS::LIGHT_PARAM_SIZE);

				if (soft_shadow_angle > 0.0) {
					float z_range = (z_vec.dot(center) + radius + pancake_size) - z_min_cam;
					soft_shadow_expand = Math::tan(Math::deg_to_rad(soft_shadow_angle)) * z_range;

					x_max += soft_shadow_expand;
					y_max += soft_shadow_expand;

					x_min -= soft_shadow_expand;
					y_min -= soft_shadow_expand;
				}
			}

			// This trick here is what stabilizes the shadow (make potential jaggies to not move)
			// at the cost of some wasted resolution. Still, the quality increase is very well worth it.
			const real_t unit = (radius + soft_shadow_expand) * 4.0 / texture_size;
			x_max_cam = Math::snapped(x_vec.dot(center) + radius + soft_shadow_expand, unit);
			x_min_cam = Math::snapped(x_vec.dot(center) - radius - soft_shadow_expand, unit);
			y_max_cam = Math::snapped(y_vec.dot(center) + radius + soft_shadow_expand, unit);
			y_min_cam = Math::snapped(y_vec.dot(center) - radius - soft_shadow_expand, unit);
		}

		//now that we know all ranges, we can proceed to make the light frustum planes, for culling octree

		Vector<Plane> light_frustum_planes;
		light_frustum_planes.resize(6);

		//right/left
		light_frustum_planes.write[0] = Plane(x_vec, x_max);
		light_frustum_planes.write[1] = Plane(-x_vec, -x_min);
		//top/bottom
		light_frustum_planes.write[2] = Plane(y_vec, y_max);
		light_frustum_planes.write[3] = Plane(-y_vec, -y_min);
		//near/far
		light_frustum_planes.write[4] = Plane(z_vec, z_max + 1e6);
		light_frustum_planes.write[5] = Plane(-z_vec, -z_min); // z_min is ok, since casters further than far-light plane are not needed

		// a pre pass will need to be needed to determine the actual z-near to be used

		z_max = z_vec.dot(center) + radius + pancake_size;

		{
			Projection ortho_camera;
			real_t half_x = (x_max_cam - x_min_cam) * 0.5;
			real_t half_y = (y_max_cam - y_min_cam) * 0.5;

			ortho_camera.set_orthogonal(-half_x, half_x, -half_y, half_y, 0, (z_max - z_min_cam));

			Vector2 uv_scale(1.0 / (x_max_cam - x_min_cam), 1.0 / (y_max_cam - y_min_cam));

			Transform3D ortho_transform;
			ortho_transform.basis = transform.basis;
			ortho_transform.origin = x_vec * (x_min_cam + half_x) + y_vec * (y_min_cam + half_y) + z_vec * z_max;

			cull.shadows[p_shadow_index].cascades[i].frustum = Frustum(light_frustum_planes);
			cull.shadows[p_shadow_index].cascades[i].projection = ortho_camera;
			cull.shadows[p_shadow_index].cascades[i].transform = ortho_transform;
			cull.shadows[p_shadow_index].cascades[i].zfar = z_max - z_min_cam;
			cull.shadows[p_shadow_index].cascades[i].split = distances[i + 1];
			cull.shadows[p_shadow_index].cascades[i].shadow_texel_size = radius * 2.0 / texture_size;
			cull.shadows[p_shadow_index].cascades[i].bias_scale = (z_max - z_min_cam);
			cull.shadows[p_shadow_index].cascades[i].range_begin = z_max;
			cull.shadows[p_shadow_index].cascades[i].uv_scale = uv_scale;
		}
	}
}