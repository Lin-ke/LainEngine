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
  void _from_data(void* p_data);
  //收到， 下一步形成一个整体框架，将整体技术路线概括进去；然后再增加引入
  // 的内容来详细说明意义；增加对实验的解释和实用性的实验，说明设置和意义 ；
  // 突出增量的介绍
};
}
#endif // MESH_INSTANCE_3D_H