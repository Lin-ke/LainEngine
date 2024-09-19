#ifndef RENDERER_GI_RD_H
#define RENDERER_GI_RD_H
#include "function/render/rendering_system/renderer_gi_api.h"

namespace lain::RendererRD{
  class GI : public RendererGI {
 private:
  static GI* p_singleton;
  public:
  ~GI();
  GI();
  static GI* get_singleton() { return p_singleton;}
  bool owns_voxel_gi(RID p_rid);

  };
}
#endif