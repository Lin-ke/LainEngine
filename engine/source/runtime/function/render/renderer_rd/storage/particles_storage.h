#ifndef PARTICAL_STORAGE_RD_H
#define PARTICAL_STORAGE_RD_H
#include "function/render/rendering_system/particles_storage_api.h"
namespace lain::RendererRD{
class ParticlesStorage : public RendererParticlesStorage {
  static ParticlesStorage* singleton;
   public:
   ParticlesStorage();
    ~ParticlesStorage();
  static ParticlesStorage* get_singleton() {return singleton;}
  bool owns_particles(RID p_rid) {return false;}
  bool owns_particles_collision(RID p_rid){return false;}// particle的碰撞形状

};
}
#endif 