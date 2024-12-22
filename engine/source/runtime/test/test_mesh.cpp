#include "test_mesh.h"
#include "scene/resources/io/resource_importer_obj.cpp"
using namespace lain::test;

void lain::test::test_mesh() {
  List<Ref<ImporterMesh>> meshes;
  Error err = _parse_obj("res://robot.obj", meshes, true, true, false, {}, {}, true, {});
  L_PRINT(err);

  Ref<Resource> res = ResourceLoader::load("res://robot.obj");
  
  }