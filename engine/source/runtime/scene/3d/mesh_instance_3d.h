#ifndef MESH_INSTANCE_3D_H
#define MESH_INSTANCE_3D_H
#include "scene/3d/visual_instance_3d.h"
#include "scene/resources/common/mesh.h"
namespace lain{

class MeshInstance3D : public GeometryInstance3D {
  LCLASS(MeshInstance3D, GeometryInstance3D);
  protected:
  Ref<Mesh> mesh;
  static void _bind_methods();
  public:
  void set_mesh(const Ref<Mesh> &p_mesh);
  Ref<Mesh> get_mesh() const;
  MeshInstance3D();
  ~MeshInstance3D();

  virtual AABB get_aabb() const override;
  private:
  void _mesh_changed();
	Vector<Ref<Material>> surface_override_materials;
	LocalVector<float> blend_shape_tracks;
	HashMap<StringName, int> blend_shape_properties;
  public:
	void set_blend_shape_value(int p_blend_shape, float p_value);
	float get_blend_shape_value(int p_blend_shape) const;
	void set_surface_override_material(int p_surface, const Ref<Material> &p_material);
	int get_surface_override_material_count() const;
  Ref<Material> get_surface_override_material(int p_surface) const ;

  
};
}
#endif // MESH_INSTANCE_3D_H