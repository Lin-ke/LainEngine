#ifndef RENDERER_SCENE_OCCLUSION_CULL_H
#define RENDERER_SCENE_OCCLUSION_CULL_H

#include "core/math/projection.h"
#include "core/templates/local_vector.h"
#include "function/render/rendering_system/rendering_system.h"
namespace lain{
  class RendererSceneOcclusionCull {
protected:
	static RendererSceneOcclusionCull *singleton;
  
public:
  RendererSceneOcclusionCull* get_singleton() { return singleton; }
  virtual bool is_occluder(RID p_rid) { return false; }
	virtual RID occluder_allocate() { return RID(); }
	virtual void occluder_initialize(RID p_occluder) {}
	virtual void free_occluder(RID p_occluder) { }
};
}
#endif