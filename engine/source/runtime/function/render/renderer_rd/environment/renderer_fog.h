#ifndef RENDERER_FOG_RD_H
#define RENDERER_FOG_RD_H
#include "function/render/rendering_system/renderer_fog_api.h"
#define RB_SCOPE_FOG SNAME("Fog")

namespace lain::RendererRD{
  class Fog : public RendererFog {
 private:
  static Fog* p_singleton;
  public:
  ~Fog();
  Fog();
  static Fog* get_singleton() { return p_singleton;}
  bool owns_fog_volume(RID p_rid) {return false;}

  };
}
#endif