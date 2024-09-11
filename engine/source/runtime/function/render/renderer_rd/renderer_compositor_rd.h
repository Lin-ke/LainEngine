
#ifndef RENDERER_COMPOSITOR_RD_H
#define RENDERER_COMPOSITOR_RD_H
#include "function/render/rendering_system/renderer_compositor.h"
#include "storage/material_storage.h"
#include "storage/mesh_storage.h"

namespace lain {
class RendererCompositorRD : public RendererCompositor {
 protected:
  RendererRD::MaterialStorage* material_storage = nullptr;
  RendererRD::MeshStorage* mesh_storage = nullptr;
static RendererCompositorRD *singleton;

 public:
  RendererMaterialStorage* get_material_storage() override { return material_storage; };
  RendererMeshStorage* get_mesh_storage() override { return mesh_storage; }
static RendererCompositorRD *get_singleton() { return singleton; }
RendererCompositorRD();
~RendererCompositorRD() override;

};
}  // namespace lain
#endif