#ifndef RENDERER_SCENE_RENDER_H
#define RENDERER_SCENE_RENDER_H
#include "core/templates/paged_array.h"
#include "renderer_geometry_instance_api.h"
namespace lain{
class RendererSceneRender {
  public:
  enum {
		MAX_DIRECTIONAL_LIGHTS = 8, // 8有向光，层级4级别
		MAX_DIRECTIONAL_LIGHT_CASCADES = 4,
		MAX_RENDER_VIEWS = 2
	};
  struct RenderShadowData {
		RID light;
		int pass = 0; // ?
		PagedArray<RenderGeometryInstance *> instances;
	};

  struct RenderSDFGIData {
		int region = 0;
		PagedArray<RenderGeometryInstance *> instances;
	};

	virtual void update() = 0;
};
}
#endif