#ifndef RENDERER_SkyRD_RD_H
#define RENDERER_SkyRD_RD_H
#include "core/io/rid_owner.h"
namespace lain::RendererRD{
  class SkyRD {
 private:
  static SkyRD* p_singleton;
  public:
  ~SkyRD();
  SkyRD();
  static SkyRD* get_singleton() { return p_singleton;}
	float sky_get_baked_exposure(RID p_sky) const;
	int roughness_layers;
  bool sky_use_cubemap_array;
  struct Sky{
		float baked_exposure = 1.0;
  };
	mutable RID_Owner<Sky, true> sky_owner;
  void init();
  };
}
#endif