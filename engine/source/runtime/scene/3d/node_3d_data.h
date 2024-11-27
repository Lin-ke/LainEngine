#ifndef NODE_3D_DATA_H
#define NODE_3D_DATA_H
#include "core/meta/reflection/reflection_marcos.h"
#define DATA_STRUCT(m_class, m_inherit) \
  struct m_class##Data : m_inherit {    \
    META(Fields)

#define DATA_STRUCT(m_class) \
  struct m_class##Data {     \
    META(Fields)

#define DATA_STRUCT_END() \
  }                       \
  ;

#define DATA_STRUCT_DEFINE(type, name) \
  type name;                           \
  bool is_##name##_used = false;

namespace lain {
DATA_STRUCT(GObject3D)
DATA_STRUCT_DEFINE(Transform3D, transform)
DATA_STRUCT_END()

}  // namespace lain
#endif