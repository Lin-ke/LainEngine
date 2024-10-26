
#ifndef RENDERER_COMPOSITOR_RD_H
#define RENDERER_COMPOSITOR_RD_H
#include "function/render/rendering_system/renderer_compositor_api.h"
#include "storage/material_storage.h"
#include "storage/mesh_storage.h"
#include "storage/light_storage.h"
#include "storage/texture_storage.h"
#include "renderer_scene_render_rd.h"
#include "utilities_rd.h"

namespace lain {
class RendererCompositorRD : public RendererCompositor {
 protected:
  RendererRD::MaterialStorage* material_storage = nullptr;
  RendererRD::MeshStorage* mesh_storage = nullptr;
  RendererRD::TextureStorage* texture_storage = nullptr;
  // light
  RendererRD::LightStorage* light_storage = nullptr;
	RendererRD::RendererSceneRenderRD *scene = nullptr;
  RendererRD::Utilities* utilities = nullptr;

  static RendererCompositorRD* singleton;
  enum BlitMode { BLIT_MODE_NORMAL, BLIT_MODE_USE_LAYER, BLIT_MODE_LENS, BLIT_MODE_NORMAL_ALPHA, BLIT_MODE_MAX };
	double time = 0.0;
	double delta = 0.0;
	static uint64_t frame;
 public:
  virtual RendererMaterialStorage* get_material_storage() override { return material_storage; };
  virtual  RendererMeshStorage* get_mesh_storage() override { return mesh_storage; }
  virtual RendererLightStorage *get_light_storage() override { return light_storage; }
  virtual RendererTextureStorage *get_texture_storage() override { return texture_storage; }
  virtual RendererSceneRender *get_scene() override { return scene; }
  virtual RendererUtilities *get_utilities() override { return utilities; }
  static RendererCompositorRD* get_singleton() { return singleton; }

  void set_boot_image(const Ref<Image>& p_image, const Color& p_color, bool p_scale, bool p_use_filter);

  void initialize();
  void begin_frame(double frame_step);
  void blit_render_targets_to_screen(WindowSystem::WindowID p_screen, const BlitToScreen* p_render_targets, int p_amount);

  void gl_end_frame(bool p_swap_buffers) {}
  void end_frame(bool p_swap_buffers);
  void finalize();

  _ALWAYS_INLINE_ uint64_t get_frame_number() const { return frame; }
  _ALWAYS_INLINE_ double get_frame_delta_time() const { return delta; }
  _ALWAYS_INLINE_ double get_total_time() const { return time; }

  static Error is_viable() { return OK; }

  static RendererCompositor* _create_current() { return memnew(RendererCompositorRD); }

  static void make_current() {
    _create_func = _create_current;
    low_end = false;
  }
  RendererCompositorRD();
  ~RendererCompositorRD() override { singleton = nullptr; }
};
}  // namespace lain
#endif