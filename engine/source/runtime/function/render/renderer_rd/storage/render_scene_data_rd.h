#ifndef RENDER_SCENE_DATA_RD_H
#define RENDER_SCENE_DATA_RD_H
#include "function/render/rendering_device/rendering_device.h"
#include "function/render/rendering_system/rendering_system.h"
#include "function/render/scene/render_scene_data_api.h"
#include "function/render/scene/renderer_scene_renderer_api.h"
namespace lain {
class RenderSceneDataRD : public RenderSceneData {
  LCLASS(RenderSceneDataRD, RenderSceneData);
	RID uniform_buffer; // loaded into this uniform buffer (supplied externally)
  
	// This struct is loaded into Set 1 - Binding 0, populated at start of rendering a frame, must match with shader code
  struct UBO {
    float projection_matrix[16];
    float inv_projection_matrix[16];
    float inv_view_matrix[16];
    float view_matrix[16];

    float projection_matrix_view[RendererSceneRender::MAX_RENDER_VIEWS][16];
    float inv_projection_matrix_view[RendererSceneRender::MAX_RENDER_VIEWS][16];
    float eye_offset[RendererSceneRender::MAX_RENDER_VIEWS][4];

    float main_cam_inv_view_matrix[16];

    float viewport_size[2];
    float screen_pixel_size[2];

    float directional_penumbra_shadow_kernel[128];  //32 vec4s
    float directional_soft_shadow_kernel[128];
    float penumbra_shadow_kernel[128];
    float soft_shadow_kernel[128];

    float radiance_inverse_xform[12];

    float ambient_light_color_energy[4];

    float ambient_color_sky_mix;
    uint32_t use_ambient_light;
    uint32_t use_ambient_cubemap;
    uint32_t use_reflection_cubemap;

    float shadow_atlas_pixel_size[2];
    float directional_shadow_pixel_size[2];

    uint32_t directional_light_count;
    float dual_paraboloid_side;
    float z_far;
    float z_near;

    uint32_t roughness_limiter_enabled;
    float roughness_limiter_amount;
    float roughness_limiter_limit;
    float opaque_prepass_threshold;

    // Fog
    uint32_t fog_enabled;
    uint32_t fog_mode;
    float fog_density;
    float fog_height;

    float fog_height_density;
    float fog_depth_curve;
    float pad;
    float fog_depth_begin;

    float fog_light_color[3];
    float fog_depth_end;

    float fog_sun_scatter;
    float fog_aerial_perspective;
    float time;
    float reflection_multiplier;

    float taa_jitter[2];
    uint32_t material_uv2_mode;
    float emissive_exposure_normalization;  // Needed to normalize emissive when using physical units.

    float IBL_exposure_normalization;  // Adjusts for baked exposure.
    uint32_t pancake_shadows;
    uint32_t camera_visible_layers;
    float pass_alpha_multiplier;
  };

  struct UBODATA {
    UBO ubo;
    UBO prev_ubo;
  };
  // api
 public:
  bool calculate_motion_vectors = false;

  Transform3D cam_transform;
  Projection cam_projection;
  Vector2 taa_jitter;
  uint32_t camera_visible_layers;
  bool cam_orthogonal = false;
  bool flip_y = false;

  // For billboards to cast correct shadows.
  Transform3D main_cam_transform;

  // For stereo rendering
  uint32_t view_count = 1;
  Vector3 view_eye_offset[RendererSceneRender::MAX_RENDER_VIEWS];
  Projection view_projection[RendererSceneRender::MAX_RENDER_VIEWS];

  Transform3D prev_cam_transform;
  Projection prev_cam_projection;
  Vector2 prev_taa_jitter;
  Projection prev_view_projection[RendererSceneRender::MAX_RENDER_VIEWS];

  float z_near = 0.0;
  float z_far = 0.0;

  float lod_distance_multiplier = 0.0;
  float screen_mesh_lod_threshold = 0.0;

  uint32_t directional_light_count = 0;
  float dual_paraboloid_side = 0.0;
  float opaque_prepass_threshold = 0.0;
  bool material_uv2_mode = false;
  float emissive_exposure_normalization = 0.0;

  Size2 shadow_atlas_pixel_size;
  Size2 directional_shadow_pixel_size;

  float time;
  float time_step;

  virtual Transform3D get_cam_transform() const;
  virtual Projection get_cam_projection() const;

  virtual uint32_t get_view_count() const;
  virtual Vector3 get_view_eye_offset(uint32_t p_view) const;
  virtual Projection get_view_projection(uint32_t p_view) const;

  virtual RID get_uniform_buffer() const;

  RID create_uniform_buffer();
  void update_ubo(RID p_uniform_buffer, RS::ViewportDebugDraw p_debug_mode, RID p_env, RID p_reflection_probe_instance, RID p_camera_attributes, bool p_pancake_shadows,
                  const Size2i& p_screen_size, const Color& p_default_bg_color, float p_luminance_multiplier, bool p_opaque_render_buffers, bool p_apply_alpha_multiplier);
};
}  // namespace lain
#endif