#ifndef RENDERER_SCENE_RENDER_H
#define RENDERER_SCENE_RENDER_H
#include "core/templates/paged_array.h"
#include "renderer_geometry_instance_api.h"
#include "function/render/rendering_system/render_scene_buffers_api.h"
#include "function/render/rendering_system/environment_storage.h"
namespace lain{
class RendererSceneRender {
	private:
	RendererEnvironmentStorage environment_storage;
	// RendererCompositorStorage compositor_storage;

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
	
	/* ENVIRONMENT API */
	RID environment_allocate();
	void environment_initialize(RID p_rid);
	void environment_free(RID p_rid);

	bool is_environment(RID p_env) const;

	RS::EnvironmentBG environment_get_background(RID p_env) const;
	int environment_get_canvas_max_layer(RID p_env) const;

	/* COMPOSITOR API*/
	RID compositor_allocate();
	void compositor_initialize(RID p_rid);
	void compositor_free(RID p_rid);

	bool is_compositor(RID p_compositor) const;

	// 虚接口
	virtual Ref<RenderSceneBuffers> render_buffers_create() = 0;

	virtual void update() = 0;
	
	struct CameraData {
		// flags
		uint32_t view_count;
		bool is_orthogonal;
		uint32_t visible_layers;
		bool vaspect;

		// Main/center projection
		Transform3D main_transform;
		Projection main_projection;

		Transform3D view_offset[RendererSceneRender::MAX_RENDER_VIEWS];
		Projection view_projection[RendererSceneRender::MAX_RENDER_VIEWS];
		Vector2 taa_jitter;

		void set_camera(const Transform3D p_transform, const Projection p_projection, bool p_is_orthogonal, bool p_vaspect, const Vector2 &p_taa_jitter = Vector2(), uint32_t p_visible_layers = 0xFFFFFFFF);
		void set_multiview_camera(uint32_t p_view_count, const Transform3D *p_transforms, const Projection *p_projections, bool p_is_orthogonal, bool p_vaspect);

	};
	virtual void set_scene_pass(uint64_t p_pass) = 0;
	

};
}
#endif