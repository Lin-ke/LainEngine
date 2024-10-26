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
  static RendererSceneOcclusionCull* get_singleton() { return singleton; }
  virtual bool is_occluder(RID p_rid) { return false; }
	virtual RID occluder_allocate() { return RID(); }
	virtual void occluder_initialize(RID p_occluder) {}
	virtual void free_occluder(RID p_occluder) { }
	virtual void buffer_set_size(RID p_buffer, const Vector2i &p_size) { }
virtual void occluder_set_mesh(RID p_occluder, const PackedVector3Array &p_vertices, const PackedInt32Array &p_indices) {}
	virtual void add_scenario(RID p_scenario) {}
		virtual void scenario_set_instance(RID p_scenario, RID p_instance, RID p_occluder, const Transform3D &p_xform, bool p_enabled) {  }
	virtual void buffer_update(RID p_buffer, const Transform3D &p_cam_transform, const Projection &p_cam_projection, bool p_cam_orthogonal) {}


};
}
#endif