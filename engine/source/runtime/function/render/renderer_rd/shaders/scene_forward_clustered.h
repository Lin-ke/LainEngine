/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef SCENE_FORWARD_CLUSTERED_GLSL_GEN_H_RD
#define SCENE_FORWARD_CLUSTERED_GLSL_GEN_H_RD

#include "function/render/renderer_rd/shader_rd.h"

namespace lain{

class SceneForwardClusteredShaderRD : public ShaderRD {

 public:
  SceneForwardClusteredShaderRD() {
  Error err;
  Ref<FileAccess> file = FileAccess::open("1.txt", FileAccess::READ, &err);
  DEV_ASSERT(err == OK); 
  static const String _vertex_code = file->get_as_utf8_string();
  static const String _fragment_code = String((const char*) (blob.ptr()));
  setup(_vertex_code, _fragment_code, String(), "SceneForwardClusteredShaderRD");
  }
};
}

#endif
