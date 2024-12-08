#include "geometry_instance_3d_data.h"
#include "scene/resources/common/mesh.h"
namespace lain{
  DATA_STRUCT(MeshInstance3D, GeometryInstance3D)
  DATA_STRUCT_DEFINE(Mesh, mesh)
  DATA_STRUCT_END()
}
