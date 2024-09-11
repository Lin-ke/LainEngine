
#ifndef RENDERING_SYSTEM_GLOBAL_H
#define RENDERING_SYSTEM_GLOBAL_H
#include "mesh_storage_api.h"
#include "material_storage_api.h"
namespace lain {

class RenderingServerGlobals {
 public:
  static bool threaded;
// 全要通过这种方式（API）来访问
// 为了兼容性
//   static RendererUtilities* utilities;
//   static RendererLightStorage* light_storage;
static RendererMaterialStorage* material_storage;
static RendererMeshStorage* mesh_storage;
//   static RendererParticlesStorage* particles_storage;
//   static RendererTextureStorage* texture_storage;
//   static RendererGI* gi;
//   static RendererFog* fog;
//   static RendererCameraAttributes* camera_attributes;
//   static RendererCanvasRender* canvas_render;
//   static RendererCompositor* rasterizer;

//   static RendererCanvasCull* canvas;
//   static RendererViewport* viewport;
//   static RenderingMethod* scene;
};
}  // namespace lain
#define RSG RenderingServerGlobals
#endif
