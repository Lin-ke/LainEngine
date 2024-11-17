#include "function/render/renderer_rd/pipeline_cache_rd.h"
#include "function/render/renderer_rd/shaders/copy.glsl.gen.h"
#include "function/render/renderer_rd/shaders/copy_to_fb.glsl.gen.h"
#include "function/render/rendering_system/rendering_system.h"
namespace lain::RendererRD {
// 几个shader 的组合：
// copy
// copy to fb
// blur
class CopyEffects {
  static CopyEffects* singleton;
  bool prefer_raster_effects;

 public:
  static CopyEffects* get_singleton();
  struct CopyPushConstant {
    int32_t section[4];
    int32_t target[2];
    uint32_t flags;
    uint32_t pad;
    // Glow.
    float glow_strength;
    float glow_bloom;
    float glow_hdr_threshold;
    float glow_hdr_scale;

    float glow_exposure;
    float glow_white;
    float glow_luminance_cap;
    float glow_auto_exposure_scale;
    // DOF.
    float camera_z_far;
    float camera_z_near;
    uint32_t pad2[2];
    //SET color
    float set_color[4];
  };

  // 一些flag
  enum {
    COPY_FLAG_HORIZONTAL = (1 << 0),
    COPY_FLAG_USE_COPY_SECTION = (1 << 1),
    COPY_FLAG_USE_ORTHOGONAL_PROJECTION = (1 << 2),
    COPY_FLAG_DOF_NEAR_FIRST_TAP = (1 << 3),
    COPY_FLAG_GLOW_FIRST_PASS = (1 << 4),
    COPY_FLAG_FLIP_Y = (1 << 5),
    COPY_FLAG_FORCE_LUMINANCE = (1 << 6),
    COPY_FLAG_ALL_SOURCE = (1 << 7),
    COPY_FLAG_ALPHA_TO_ONE = (1 << 8),
  };

  enum CopyMode {
    COPY_MODE_GAUSSIAN_COPY,
    COPY_MODE_GAUSSIAN_COPY_8BIT,
    COPY_MODE_GAUSSIAN_GLOW,
    COPY_MODE_GAUSSIAN_GLOW_AUTO_EXPOSURE,
    COPY_MODE_SIMPLY_COPY,
    COPY_MODE_SIMPLY_COPY_8BIT,
    COPY_MODE_SIMPLY_COPY_DEPTH,
    COPY_MODE_SET_COLOR,
    COPY_MODE_SET_COLOR_8BIT,
    COPY_MODE_MIPMAP,
    COPY_MODE_LINEARIZE_DEPTH,
    COPY_MODE_CUBE_TO_PANORAMA,
    COPY_MODE_CUBE_ARRAY_TO_PANORAMA,
    COPY_MODE_MAX,

  };

  struct Copy {
    CopyPushConstant push_constant;
    CopyShaderRD shader;
    RID shader_version;
    RID pipelines[COPY_MODE_MAX];

  } copy;

  // Copy to FB shader

  enum CopyToFBMode {
    COPY_TO_FB_COPY,
    COPY_TO_FB_COPY_PANORAMA_TO_DP,
    COPY_TO_FB_COPY2,
    COPY_TO_FB_SET_COLOR,

    // These variants are disabled unless XR shaders are enabled.
    // They should be listed last.
    COPY_TO_FB_MULTIVIEW,
    COPY_TO_FB_MULTIVIEW_WITH_DEPTH,

    COPY_TO_FB_MAX,
  };

  enum CopyToFBFlags {
    COPY_TO_FB_FLAG_FLIP_Y = (1 << 0),
    COPY_TO_FB_FLAG_USE_SECTION = (1 << 1),
    COPY_TO_FB_FLAG_FORCE_LUMINANCE = (1 << 2),
    COPY_TO_FB_FLAG_ALPHA_TO_ZERO = (1 << 3),
    COPY_TO_FB_FLAG_SRGB = (1 << 4),
    COPY_TO_FB_FLAG_ALPHA_TO_ONE = (1 << 5),
    COPY_TO_FB_FLAG_LINEAR = (1 << 6),
    COPY_TO_FB_FLAG_NORMAL = (1 << 7),
    COPY_TO_FB_FLAG_USE_SRC_SECTION = (1 << 8),
  };

  struct CopyToFbPushConstant {
    float section[4];
    float pixel_size[2];
    float luminance_multiplier;
    uint32_t flags;

    float set_color[4];
  };

  struct CopyToFb {
    CopyToFbPushConstant push_constant;
    CopyToFbShaderRD shader;
    RID shader_version;
    PipelineCacheRD pipelines[COPY_TO_FB_MAX];

  } copy_to_fb;

  CopyEffects(bool p_prefer_raster_effects);
  ~CopyEffects();

  void copy_to_rect(RID p_source_rd_texture, RID p_dest_texture, const Rect2i& p_rect, bool p_flip_y = false, bool p_force_luminance = false, bool p_all_source = false,
                    bool p_8_bit_dst = false, bool p_alpha_to_one = false);
  void copy_to_fb_rect(RID p_source_rd_texture, RID p_dest_framebuffer, const Rect2i& p_rect, bool p_flip_y = false, bool p_force_luminance = false,
                       bool p_alpha_to_zero = false, bool p_srgb = false, RID p_secondary = RID(), bool p_multiview = false, bool alpha_to_one = false, bool p_linear = false,
                       bool p_normal = false, const Rect2& p_src_rect = Rect2());
};
}  // namespace lain::RendererRD
