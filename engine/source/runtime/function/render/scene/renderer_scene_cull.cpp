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

RID RendererSceneCull::scenario_allocate() {
  return scenario_owner.allocate_rid();
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

RendererSceneCull::Instance::Instance():
update_item(this) {
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

void lain::RendererSceneCull::instance_set_scenario(RID p_instance, RID p_scenario) {}

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
void RendererSceneCull::_instance_update_mesh_instance(Instance *p_instance) {
}