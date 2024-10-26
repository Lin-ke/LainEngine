#ifndef RENDERER_COMPOSITOR_H
#define RENDERER_COMPOSITOR_H
#include "mesh_storage_api.h"
#include "material_storage_api.h"
#include "core/io/image.h"
#include "function/render/scene/renderer_scene_renderer_api.h"
#include "texture_storage_api.h"
#include "light_storage_api.h"
namespace lain{
struct BlitToScreen {
	RID render_target;
	Rect2 src_rect = Rect2(0.0, 0.0, 1.0, 1.0);
	Rect2i dst_rect;

	struct {
		bool use_layer = false;
		uint32_t layer = 0;
	} multi_view;

	struct {
		//lens distorted parameters for VR
		bool apply = false;
		Vector2 eye_center;
		float k1 = 0.0;
		float k2 = 0.0;

		float upscale = 1.0;
		float aspect_ratio = 1.0;
	} lens_distortion;
};
class RendererCompositor {
private:
	bool xr_enabled = false;
	static RendererCompositor *singleton;

protected:
	static RendererCompositor *(*_create_func)();
	bool back_end = false;
	static bool low_end;

public:
	/// 这里的指针都指向 RD 或者 OpGL实现。
	// render_system 在 init时 查询这些指针 （要求create时创建好）
	static RendererCompositor *create();

	virtual RendererUtilities *get_utilities() = 0;
	virtual RendererLightStorage *get_light_storage() = 0;
	virtual RendererMaterialStorage *get_material_storage() = 0;
	virtual RendererMeshStorage *get_mesh_storage() = 0;
	// virtual RendererParticlesStorage *get_particles_storage() = 0;
	virtual RendererTextureStorage *get_texture_storage() = 0;
	// virtual RendererGI *get_gi() = 0;
	// virtual RendererFog *get_fog() = 0;
	// virtual RendererCanvasRender *get_canvas() = 0;
	virtual RendererSceneRender *get_scene() = 0;

	virtual void set_boot_image(const Ref<Image> &p_image, const Color &p_color, bool p_scale, bool p_use_filter = true) = 0;

	virtual void initialize() = 0;
	virtual void begin_frame(double frame_step) = 0;

	virtual void blit_render_targets_to_screen(WindowSystem::WindowID p_screen, const BlitToScreen *p_render_targets, int p_amount) = 0;

	virtual void gl_end_frame(bool p_swap_buffers) = 0;
	virtual void end_frame(bool p_swap_buffers) = 0;
	virtual void finalize() = 0;
	virtual uint64_t get_frame_number() const = 0;
	virtual double get_frame_delta_time() const = 0;
	virtual double get_total_time() const = 0;

	static bool is_low_end() { return low_end; };
	virtual bool is_xr_enabled() const;

	static RendererCompositor *get_singleton() { return singleton; }
	RendererCompositor();
	virtual ~RendererCompositor();
};

}
#endif