
#ifndef RENDERER_COMPOSITOR_RD_H
#define RENDERER_COMPOSITOR_RD_H
#include "function/render/rendering_system/renderer_compositor.h"
#include "storage/material_storage.h"
#include "storage/mesh_storage.h"

namespace lain {
class RendererCompositorRD : public RendererCompositor {
    protected:
    RendererRD::MaterialStorage *material_storage = nullptr;
    RendererRD::MeshStorage *mesh_storage = nullptr;

    public:
	virtual RendererMaterialStorage *get_material_storage() {

    };

};
}
#endif