#ifndef RENDERER_GEOMETRY_INSTANCE_API_H
#define RENDERER_GEOMETRY_INSTANCE_API_H
#include "core/math/rect2.h"
#include "core/math/transform3d.h"
#include "core/math/vector3.h"
#include "core/io/rid.h"
#include "function/render/rendering_system/utilities.h"
namespace lain {
class RenderGeometryInstance {
	virtual ~RenderGeometryInstance() {}
	virtual void _mark_dirty() = 0;
	public:
  virtual void set_skeleton(RID p_skeleton) = 0;
	virtual void set_material_override(RID p_override) = 0;
	virtual void set_material_overlay(RID p_overlay) = 0;
	virtual void set_surface_materials(const Vector<RID> &p_materials) = 0;
	virtual void set_mesh_instance(RID p_mesh_instance) = 0;
	virtual void set_transform(const Transform3D &p_transform, const AABB &p_aabb, const AABB &p_transformed_aabb) = 0;
	virtual void set_pivot_data(float p_sorting_offset, bool p_use_aabb_center) = 0;
	virtual void set_lod_bias(float p_lod_bias) = 0;
	virtual void set_layer_mask(uint32_t p_layer_mask) = 0;
	virtual void set_fade_range(bool p_enable_near, float p_near_begin, float p_near_end, bool p_enable_far, float p_far_begin, float p_far_end) = 0;
	virtual void set_parent_fade_alpha(float p_alpha) = 0;
	virtual void set_transparency(float p_transparency) = 0;
	virtual void set_use_baked_light(bool p_enable) = 0;
	virtual void set_use_dynamic_gi(bool p_enable) = 0;
	virtual void set_use_lightmap(RID p_lightmap_instance, const Rect2 &p_lightmap_uv_scale, int p_lightmap_slice_index) = 0;
	virtual void set_lightmap_capture(const Color *p_sh9) = 0;
	virtual void set_instance_shader_uniforms_offset(int32_t p_offset) = 0;
	virtual void set_cast_double_sided_shadows(bool p_enable) = 0;

	virtual Transform3D get_transform() = 0;
	virtual AABB get_aabb() = 0;
};
} // namespace lain
#endif