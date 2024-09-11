#include "renderer_compositor_rd.h"
#include "storage/mesh_storage.h"
#include "storage/material_storage.h"
using namespace lain;
lain::RendererCompositorRD::RendererCompositorRD() {
    material_storage = new(RendererRD::MaterialStorage);

}