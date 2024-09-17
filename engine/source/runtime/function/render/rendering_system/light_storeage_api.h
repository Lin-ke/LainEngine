#ifndef LIGHT_STORAGE_API_H
#define LIGHT_STORAGE_API_H
#include "rendering_system.h"

namespace lain{

class RendererLightStorage {
  
	virtual RID directional_light_allocate() = 0;
	virtual void directional_light_initialize(RID p_rid) = 0;

	virtual RID omni_light_allocate() = 0;
	virtual void omni_light_initialize(RID p_rid) = 0;

	virtual RID spot_light_allocate() = 0;
	virtual void spot_light_initialize(RID p_rid) = 0;

	virtual void light_free(RID p_rid) = 0;

	virtual void light_set_color(RID p_light, const Color &p_color) = 0;

};

}
#endif
