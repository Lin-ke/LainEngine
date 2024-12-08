#ifndef NODE_3D_DATA_H
#define NODE_3D_DATA_H
#include "core/meta/reflection/reflection_marcos.h"
#define DATA_STRUCT(m_class, m_inherit) \
  struct m_class##Data : m_inherit##Data {    \
    META(Fields)

#define DATA_STRUCT_END() \
  }                       \
  ;

#define DATA_STRUCT_DEFINE(type, name) \
  type name;                           \
  bool is_##name##_used = false;

namespace lain {
struct GObject3DData {     \
    META(Fields)
    Transform3D transform;
    bool is_transform_used = false;
};

}  // namespace lain
#endif