#ifndef MESH_STORAGE_API_H
#define MESH_STORAGE_API_H
#include  "rendering_system.h"

namespace lain{

 class RendererMeshStorage {
    virtual ~RendererMeshStorage() {}

	/* MESH API */

	virtual RID mesh_allocate() = 0;
	virtual void mesh_initialize(RID p_rid) = 0;
	virtual void mesh_free(RID p_rid) = 0;

	virtual void mesh_set_blend_shape_count(RID p_mesh, int p_blend_shape_count) = 0;

	/// Returns stride
	virtual void mesh_add_surface(RID p_mesh, const RS::SurfaceData &p_surface) = 0;

	virtual int mesh_get_blend_shape_count(RID p_mesh) const = 0;

	virtual void mesh_set_blend_shape_mode(RID p_mesh, RS::BlendShapeMode p_mode) = 0;
	virtual RS::BlendShapeMode mesh_get_blend_shape_mode(RID p_mesh) const = 0;

	virtual void mesh_surface_update_vertex_region(RID p_mesh, int p_surface, int p_offset, const Vector<uint8_t> &p_data) = 0;
	virtual void mesh_surface_update_attribute_region(RID p_mesh, int p_surface, int p_offset, const Vector<uint8_t> &p_data) = 0;
	virtual void mesh_surface_update_skin_region(RID p_mesh, int p_surface, int p_offset, const Vector<uint8_t> &p_data) = 0;

	virtual void mesh_surface_set_material(RID p_mesh, int p_surface, RID p_material) = 0;
	virtual RID mesh_surface_get_material(RID p_mesh, int p_surface) const = 0;

	virtual RS::SurfaceData mesh_get_surface(RID p_mesh, int p_surface) const = 0;

	virtual int mesh_get_surface_count(RID p_mesh) const = 0;

	virtual void mesh_set_custom_aabb(RID p_mesh, const AABB &p_aabb) = 0;
	virtual AABB mesh_get_custom_aabb(RID p_mesh) const = 0;
	virtual AABB mesh_get_aabb(RID p_mesh, RID p_skeleton = RID()) = 0;

	virtual void mesh_set_path(RID p_mesh, const String &p_path) = 0;
	virtual String mesh_get_path(RID p_mesh) const = 0;

	virtual void mesh_set_shadow_mesh(RID p_mesh, RID p_shadow_mesh) = 0;

	virtual void mesh_clear(RID p_mesh) = 0;

	virtual bool mesh_needs_instance(RID p_mesh, bool p_has_skeleton) = 0;

	/* MESH INSTANCE */

	virtual RID mesh_instance_create(RID p_base) = 0;
	virtual void mesh_instance_free(RID p_rid) = 0;
	virtual void mesh_instance_set_skeleton(RID p_mesh_instance, RID p_skeleton) = 0;
	virtual void mesh_instance_set_blend_shape_weight(RID p_mesh_instance, int p_shape, float p_weight) = 0;
	virtual void mesh_instance_check_for_update(RID p_mesh_instance) = 0;
	virtual void mesh_instance_set_canvas_item_transform(RID p_mesh_instance, const Transform2D &p_transform) = 0;
	virtual void update_mesh_instances() = 0;

};
}; // namespace lain
#endif // MESH_STORAGE_API_H