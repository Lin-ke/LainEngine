#ifndef GEOMETRY_INSTANCE_DATA
#define GEOMETRY_INSTANCE_DATA
#include "node_3d_data.h"
namespace lain{
DATA_STRUCT(GeometryInstance3D, GObject3D)
DATA_STRUCT_DEFINE(float, lod_bias)
DATA_STRUCT_DEFINE(int, cast_shadow)
DATA_STRUCT_END()
}

#endif