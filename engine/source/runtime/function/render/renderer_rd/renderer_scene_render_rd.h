#include "function/render/scene/renderer_scene_renderer_api.h"
namespace lain::RendererRD {
  // Implementaion 3D scene rendering using Rendering Device
  class RendererSceneRenderRD : public RendererSceneRender {
    static RendererSceneRenderRD* singleton;
    public:
    static RendererSceneRenderRD* get_singleton() {return singleton;}
    RendererSceneRenderRD();
    ~RendererSceneRenderRD();
    void cull_scene();
    uint64_t get_scene_pass();

    virtual void update() override;
  };
}
