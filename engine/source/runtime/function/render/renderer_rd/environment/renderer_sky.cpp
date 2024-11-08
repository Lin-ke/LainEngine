#include "renderer_sky.h"

float lain::RendererRD::SkyRD::sky_get_baked_exposure(RID p_sky) const {
  Sky* sky = sky_owner.get_or_null(p_sky);
  ERR_FAIL_NULL_V(sky, 1.0);
  return sky->baked_exposure;
}