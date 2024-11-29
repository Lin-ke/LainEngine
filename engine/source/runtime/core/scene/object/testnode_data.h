#ifndef TESTNODE_DATA_H
#define TESTNODE_DATA_H

#include "core/meta/reflection/reflection_marcos.h"
namespace lain{
  struct TestNodeData{
    META(Fields)
    int a;
};
}
#endif