#ifndef PRIMITIVE_MESHES_H
#define PRIMITIVE_MESHES_H
#include "mesh.h"
namespace lain{
class PrimitiveMesh : public Mesh {
	LCLASS(PrimitiveMesh, Mesh);
  private:
	RID mesh;
	mutable AABB aabb;
	AABB custom_aabb;	
  mutable int array_len = 0;
	mutable int index_array_len = 0;
  Ref<Material> material;
	bool flip_faces = false;

	bool add_uv2 = false;
	float uv2_padding = 2.0;
	void _update() const;
	mutable bool pending_request = true; // dirty, need update

protected:
  Mesh::PrimitiveType primitive_type = Mesh::PRIMITIVE_TRIANGLES;
	Vector2 get_uv2_scale(Vector2 p_margin_scale = Vector2(1.0, 1.0)) const;
	virtual void _create_mesh_array(Array &p_arr) const {}
	float get_lightmap_texel_size() const;
	virtual void _update_lightmap_size(){};

public:
	virtual int get_surface_count() const override;
	virtual int surface_get_array_len(int p_idx) const override;
	virtual int surface_get_array_index_len(int p_idx) const override;
	virtual Array surface_get_arrays(int p_surface) const override;
	// virtual TypedArray<Array> surface_get_blend_shape_arrays(int p_surface) const override;
	virtual Dictionary surface_get_lods(int p_surface) const override;
	virtual BitField<ArrayFormat> surface_get_format(int p_idx) const override;
	virtual Mesh::PrimitiveType surface_get_primitive_type(int p_idx) const override;
	virtual void surface_set_material(int p_idx, const Ref<Material> &p_material) override;
	virtual Ref<Material> surface_get_material(int p_idx) const override;
	virtual int get_blend_shape_count() const override;
	virtual StringName get_blend_shape_name(int p_index) const override;
	virtual void set_blend_shape_name(int p_index, const StringName &p_name) override;
	virtual AABB get_aabb() const override;
	virtual RID GetRID() const override;

  bool get_add_uv2() const { return add_uv2; }
	float get_uv2_padding() const { return uv2_padding; }

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;
	void request_update();

  
	PrimitiveMesh();
	~PrimitiveMesh();

};

class CapsuleMesh : public PrimitiveMesh {
	LCLASS(CapsuleMesh, PrimitiveMesh);

private:
	float radius = 0.5;
	float height = 2.0;
	int radial_segments = 64;
	int rings = 8;

protected:
	static void _bind_methods();
	// virtual void _create_mesh_array(Array &p_arr) const override;

	virtual void _update_lightmap_size() override;
	virtual void _create_mesh_array(Array &p_arr) const override;

public:
	static void create_mesh_array(Array &p_arr, float radius, float height, int radial_segments = 64, int rings = 8, bool p_add_uv2 = false, const float p_uv2_padding = 1.0);

	void set_radius(const float p_radius);
	float get_radius() const;

	void set_height(const float p_height);
	float get_height() const;

	void set_radial_segments(const int p_segments);
	int get_radial_segments() const;

	void set_rings(const int p_rings);
	int get_rings() const;

	CapsuleMesh() {}

};

}
#endif