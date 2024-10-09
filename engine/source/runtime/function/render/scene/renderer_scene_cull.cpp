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
        _update_instance_shader_uniforms_from_material(isparams, p_instance->instance_shader_uniforms, p_instance->material_override); // 使用material_override
      } else { // no override
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
        } // end of mesh
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
        } // end of multimesh
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
        } else { // !instance_allocated_shader_uniforms
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
void RendererSceneCull::_update_instance_shader_uniforms_from_material(HashMap<StringName, Instance::InstanceShaderParameter> &isparams, const HashMap<StringName, Instance::InstanceShaderParameter> &existing_isparams, RID p_material) {
	List<RendererMaterialStorage::InstanceShaderParam> plist;
	RSG::material_storage->material_get_instance_shader_parameters(p_material, &plist);
	for (const RendererMaterialStorage::InstanceShaderParam &E : plist) {
		StringName name = E.info.name;
		if (isparams.has(name)) {
			if (isparams[name].info.type != E.info.type) {
				WARN_PRINT("More than one material in instance export the same instance shader uniform '" + E.info.name + "', but they do it with different data types. Only the first one (in order) will display correctly.");
			}
			if (isparams[name].index != E.index) {
				WARN_PRINT("More than one material in instance export the same instance shader uniform '" + E.info.name + "', but they do it with different indices. Only the first one (in order) will display correctly.");
			}
			continue; //first one found always has priority
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