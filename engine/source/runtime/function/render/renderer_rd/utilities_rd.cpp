#include "utilities_rd.h"
#include "storage/material_storage.h"
#include "storage/mesh_storage.h"
#include "storage/light_storage.h"
#include "storage/particles_storage.h"
#include "storage/texture_storage.h"
#include "environment/renderer_gi.h"
#include "environment/renderer_fog.h"
using namespace lain;

using namespace lain::RendererRD;
Utilities::Utilities() {
  singleton = this;
}
Utilities::~Utilities() {
  singleton = nullptr;
}

RS::InstanceType Utilities::get_base_type(RID p_rid) const {
  if (RendererRD::MeshStorage::get_singleton()->owns_mesh(p_rid)) {
    return RS::INSTANCE_MESH;
  }
  if (RendererRD::MeshStorage::get_singleton()->owns_multimesh(p_rid)) {
    return RS::INSTANCE_MULTIMESH;
  }
  if (RendererRD::LightStorage::get_singleton()->owns_reflection_probe(p_rid)) {
    return RS::INSTANCE_REFLECTION_PROBE;
  }
  if (RendererRD::TextureStorage::get_singleton()->owns_decal(p_rid)) {
    return RS::INSTANCE_DECAL;
  }
  if (RendererRD::GI::get_singleton()->owns_voxel_gi(p_rid)) {
    return RS::INSTANCE_VOXEL_GI;
  }
  if (RendererRD::LightStorage::get_singleton()->owns_light(p_rid)) {
    return RS::INSTANCE_LIGHT;
  }
  if (RendererRD::LightStorage::get_singleton()->owns_lightmap(p_rid)) {
    return RS::INSTANCE_LIGHTMAP;
  }
  if (RendererRD::ParticlesStorage::get_singleton()->owns_particles(p_rid)) {
    return RS::INSTANCE_PARTICLES;
  }
  if (RendererRD::ParticlesStorage::get_singleton()->owns_particles_collision(p_rid)) {
    return RS::INSTANCE_PARTICLES_COLLISION;
  }
  if (RendererRD::Fog::get_singleton()->owns_fog_volume(p_rid)) {
    return RS::INSTANCE_FOG_VOLUME;
  }
  if (owns_visibility_notifier(p_rid)) {
    return RS::INSTANCE_VISIBLITY_NOTIFIER;
  }

  return RS::INSTANCE_NONE;
}

void lain::RendererRD::Utilities::update_dirty_resources() {
  // @todo
}
