#ifndef COMPOSITOR_STORAGE_H
#define COMPOSITOR_STORAGE_H
#include "rendering_system.h"
#include "core/io/rid_owner.h"
class RendererCompositorStorage {
private:
	static RendererCompositorStorage *singleton;
	int num_compositor_effects_with_motion_vectors = 0;
public:
  L_INLINE static RendererCompositorStorage* get_singleton() {return singleton;}
  L_INLINE int get_num_compositor_effects_with_motion_vectors() {return num_compositor_effects_with_motion_vectors;}
};
#endif