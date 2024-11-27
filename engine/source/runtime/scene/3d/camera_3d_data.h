#ifndef CAMERA_3D_DATA_H
#define CAMERA_3D_DATA_H
#include "node_3d_data.h"
namespace lain{
  struct Camera3DData : GObject3DData {
    META(Fields)
    float fov = 10;
    bool is_fov_used = false;
  };
}
#endif