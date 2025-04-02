
#ifndef RENDERER_COMPOSITOR_RD_H
#define RENDERER_COMPOSITOR_RD_H
#include "function/render/rendering_system/renderer_compositor_api.h"
#include "renderer_scene_render_rd.h"
#include "storage/light_storage.h"
#include "storage/material_storage.h"
#include "storage/mesh_storage.h"
#include "storage/texture_storage.h"
#include "storage/particles_storage.h"
#include "environment/renderer_fog.h"
#include "environment/renderer_gi.h"

#include "utilities_rd.h"

#include "shaders/blit.glsl.gen.h"
#include "storage/framebuffer_cache_rd.h"
#include "uniform_set_cache_rd.h" // 这个也应该放到那里
namespace lain {
class RendererCompositorRD : public RendererCompositor {
 protected:
   UniformSetCacheRD *uniform_set_cache = nullptr;
	FramebufferCacheRD *framebuffer_cache = nullptr;
  RendererSceneRenderRD* scene = nullptr;
  RendererRD::MaterialStorage* material_storage = nullptr;
  RendererRD::MeshStorage* mesh_storage = nullptr;
  RendererRD::TextureStorage* texture_storage = nullptr;
  // light
  RendererRD::LightStorage* light_storage = nullptr;
  RendererRD::Utilities* utilities = nullptr;
  RendererRD::Fog* fog= nullptr;
// GI 在 renderer_scene_renderer里
  RendererRD::ParticlesStorage* particles_storage = nullptr;


  

  static RendererCompositorRD* singleton;
  enum BlitMode { BLIT_MODE_NORMAL, BLIT_MODE_USE_LAYER, BLIT_MODE_LENS, BLIT_MODE_NORMAL_ALPHA, BLIT_MODE_MAX };
  double time = 0.0;
  double delta = 0.0;
  static uint64_t frame;
  // uniform set里是这个 texture的uniform 和 sampler
  // 那么问题来了，如果这个texture释放之后，这个uniform set会自动释放吗

  HashMap<RID, RID> render_target_descriptors;  // rd_texture -> uniform set

  // blit会有len distortion的特效，所以提供 aspect_ratio 等等
  // ref : blit.glsl
  struct BlitPushConstant {
    float src_rect[4];
    float dst_rect[4];

    float eye_center[2];
    float k1;
    float k2;

    float upscale;
    float aspect_ratio;
    uint32_t layer;
    uint32_t convert_to_srgb;
  };

  struct Blit { // array， sampler 都是固定的，只需要编译shader 的变体
    BlitPushConstant push_constant;
    BlitShaderRD shader;
    RID shader_version;
    RID pipelines[BLIT_MODE_MAX];
    RID index_buffer;
    RID array;
    RID sampler;
  } blit;

 public:
  virtual RendererMaterialStorage* get_material_storage() override { return material_storage; };
  virtual RendererMeshStorage* get_mesh_storage() override { return mesh_storage; }
  virtual RendererLightStorage* get_light_storage() override { return light_storage; }
  virtual RendererTextureStorage* get_texture_storage() override { return texture_storage; }
  virtual RendererSceneRender* get_scene() override { return scene; }
  virtual RendererUtilities* get_utilities() override { return utilities; }
  virtual RendererGI* get_gi() override { 
		ERR_FAIL_NULL_V(scene, nullptr);
		return scene->get_gi(); // sky 和gi在scene里
  }
  virtual RendererFog* get_fog() override { return fog; }
  virtual RendererParticlesStorage* get_particles_storage() override { return particles_storage; }
  static RendererCompositorRD* get_singleton() { return singleton; }



  // void set_boot_image(const Ref<Image>& p_image, const Color& p_color, bool p_scale, bool p_use_filter);

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

  static void make_current() { // 在这里初始化了 _create_func
    _create_func = _create_current;
    low_end = false;
  }
  RendererCompositorRD();
  ~RendererCompositorRD() override { singleton = nullptr; }
};
}  // namespace lain
#endif