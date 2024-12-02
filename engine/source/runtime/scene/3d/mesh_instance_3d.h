#ifndef MESH_INSTANCE_3D_H
#define MESH_INSTANCE_3D_H
#include "scene/3d/visual_instance_3d.h"
#include "scene/resources/common/mesh.h"
namespace lain{

class MeshInstance3D : public GeometryInstance3D {
  LCLASS(MeshInstance3D, GeometryInstance3D);
  protected:
  Ref<Mesh> mesh;
  public:
  void set_mesh(const Ref<Mesh> &p_mesh);
  Ref<Mesh> get_mesh() const;
  MeshInstance3D();
  ~MeshInstance3D();

  virtual AABB get_aabb() const override;
};
}
#endif // MESH_INSTANCE_3D_H