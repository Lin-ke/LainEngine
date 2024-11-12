#ifndef RENDERER_VIEWPORT_API_H
#define RENDERER_VIEWPORT_API_H
#include "rendering_system.h"
#include "rendering_method_api.h"
#include "../scene/renderer_scene_renderer_api.h"
#include "../scene/render_scene_buffers_api.h"
namespace lain {
class RendererViewport {
 public:
  struct Viewport {
    RID self;
    RID parent;

    Size2i size;
    uint32_t view_count = 1;
    RID camera;
    RID scenario;

    RID render_target;
    RID render_target_texture;
    Size2i internal_size = Size2i(0, 0);  // 是render size ( target_size * scale, For FSR etc. )
		RenderingMethod::RenderInfo render_info;
		RS::ViewportEnvironmentMode disable_environment = RS::VIEWPORT_ENVIRONMENT_DISABLED;

    RID shadow_atlas;
    int shadow_atlas_size = 2048;
    bool shadow_atlas_16_bits = true;

    WindowSystem::WindowID viewport_to_screen;
    Rect2 viewport_to_screen_rect;
    bool viewport_render_direct_to_screen;

    bool disable_3d = false;
    bool disable_2d = false;

    RS::ViewportMSAA msaa_3d = RS::VIEWPORT_MSAA_DISABLED;
    RS::ViewportMSAA msaa_2d = RS::VIEWPORT_MSAA_DISABLED;

    RS::ViewportScreenSpaceAA screen_space_aa = RS::VIEWPORT_SCREEN_SPACE_AA_DISABLED;
    RS::ViewportUpdateMode update_mode = RS::VIEWPORT_UPDATE_WHEN_VISIBLE;

    // camera data
		uint64_t prev_camera_data_frame = 0;
    RendererSceneRender::CameraData prev_camera_data;

    bool use_debanding = false;  // 使用快速后处理滤镜使条带不明显
    float fsr_sharpness = 0.0;  // 确定使用 FSR 放大模式时放大图像的清晰度。每出现一个整数，清晰度就会减半。值从 0.0（最锐利）到 2.0。高于 2.0 的值不会产生明显的差异。

    bool occlusion_buffer_dirty = true;

    Ref<RenderSceneBuffers> render_buffers;
    float scaling_3d_scale = 1.0;  // 可用于调节分辨率 scale
    RS::ViewportScaling3DMode scaling_3d_mode = RS::VIEWPORT_SCALING_3D_MODE_BILINEAR;
    bool fsr_enabled = false;  // 允许fsr
    bool use_taa = true;
    int jitter_phase_count = 0;
    float texture_mipmap_bias = 0.0f;
    RS::ViewportClearMode clear_mode = RS::VIEWPORT_CLEAR_ALWAYS;  // 绘制前始终清楚

    bool use_occlusion_culling = false;
		
		bool transparent_bg = false; // 渲染的透明背景

		int last_pass = -1; // 记录此pass时是否visible （当=draw_viewports_pass意味着此vp可见）
		float mesh_lod_threshold = 1.0;
		RS::ViewportDebugDraw debug_draw = RS::VIEWPORT_DEBUG_DRAW_DISABLED; // debug draw

    // statistics
		bool measure_render_time = true;
    uint64_t time_cpu_begin = 0;
    uint64_t time_cpu_end = 0;

    uint64_t time_gpu_begin = 0;
    uint64_t time_gpu_end = 0;
  };
  mutable RID_Owner<Viewport, true> viewport_owner;

  Vector<Viewport*> active_viewports;
  Vector<Viewport*> sorted_active_viewports;
	bool sorted_active_viewports_dirty = false; // true则需要重新排序
	int num_viewports_with_motion_vectors = 0;


  HashMap<String, RID> timestamp_vp_map;  // 时间戳到viewport的映射 (为什么不是相反的)

 public:
  RID viewport_allocate();
  void viewport_initialize(RID p_rid);

  void viewport_set_use_xr(RID p_viewport, bool p_use_xr);

  void viewport_set_size(RID p_viewport, int p_width, int p_height);

  void viewport_attach_to_screen(RID p_viewport, const Rect2& p_rect = Rect2(), WindowSystem::WindowID p_screen = WindowSystem::MAIN_WINDOW_ID);
  void viewport_set_render_direct_to_screen(RID p_viewport, bool p_enable);

  void viewport_set_active(RID p_viewport, bool p_active);
  void viewport_set_parent_viewport(RID p_viewport, RID p_parent_viewport);

  void viewport_set_scaling_3d_mode(RID p_viewport, RS::ViewportScaling3DMode p_mode);
  void viewport_set_scaling_3d_scale(RID p_viewport, float p_scaling_3d_scale);
  void viewport_set_fsr_sharpness(RID p_viewport, float p_sharpness);
  void viewport_set_texture_mipmap_bias(RID p_viewport, float p_mipmap_bias);

  void viewport_set_update_mode(RID p_viewport, RS::ViewportUpdateMode p_mode);
  RS::ViewportUpdateMode viewport_get_update_mode(RID p_viewport) const;
  void viewport_set_vflip(RID p_viewport, bool p_enable);

  void viewport_set_clear_mode(RID p_viewport, RS::ViewportClearMode p_clear_mode);

  RID viewport_get_render_target(RID p_viewport) const;
  RID viewport_get_texture(RID p_viewport) const;
  RID viewport_get_occluder_debug_texture(RID p_viewport) const;

  void viewport_set_prev_camera_data(RID p_viewport, const RendererSceneRender::CameraData *p_camera_data);
  const RendererSceneRender::CameraData *viewport_get_prev_camera_data(RID p_viewport);

  void viewport_set_disable_2d(RID p_viewport, bool p_disable);
  void viewport_set_environment_mode(RID p_viewport, RS::ViewportEnvironmentMode p_mode);
  void viewport_set_disable_3d(RID p_viewport, bool p_disable);

  bool viewport_is_environment_disabled(Viewport* viewport);

  void viewport_attach_camera(RID p_viewport, RID p_camera);
  void viewport_set_scenario(RID p_viewport, RID p_scenario);
  void viewport_attach_canvas(RID p_viewport, RID p_canvas);
  void viewport_remove_canvas(RID p_viewport, RID p_canvas);
  void viewport_set_canvas_transform(RID p_viewport, RID p_canvas, const Transform2D& p_offset);
  void viewport_set_transparent_background(RID p_viewport, bool p_enabled);
  void viewport_set_use_hdr_2d(RID p_viewport, bool p_use_hdr_2d);

  void viewport_set_global_canvas_transform(RID p_viewport, const Transform2D& p_transform);
  void viewport_set_canvas_stacking(RID p_viewport, RID p_canvas, int p_layer, int p_sublayer);

  void viewport_set_canvas_cull_mask(RID p_viewport, uint32_t p_canvas_cull_mask);

  void viewport_set_positional_shadow_atlas_size(RID p_viewport, int p_size, bool p_16_bits = true);
  void viewport_set_positional_shadow_atlas_quadrant_subdivision(RID p_viewport, int p_quadrant, int p_subdiv);

  void viewport_set_msaa_2d(RID p_viewport, RS::ViewportMSAA p_msaa);
  void viewport_set_msaa_3d(RID p_viewport, RS::ViewportMSAA p_msaa);
  void viewport_set_screen_space_aa(RID p_viewport, RS::ViewportScreenSpaceAA p_mode);
  void viewport_set_use_taa(RID p_viewport, bool p_use_taa);
  void viewport_set_use_debanding(RID p_viewport, bool p_use_debanding);
  void viewport_set_use_occlusion_culling(RID p_viewport, bool p_use_occlusion_culling);
  void viewport_set_occlusion_rays_per_thread(int p_rays_per_thread);
  void viewport_set_occlusion_culling_build_quality(RS::ViewportOcclusionCullingBuildQuality p_quality);
  void viewport_set_mesh_lod_threshold(RID p_viewport, float p_pixels);

  virtual int viewport_get_render_info(RID p_viewport, RS::ViewportRenderInfoType p_type, RS::ViewportRenderInfo p_info);
  virtual void viewport_set_debug_draw(RID p_viewport, RS::ViewportDebugDraw p_draw);

  void viewport_set_measure_render_time(RID p_viewport, bool p_enable);
  float viewport_get_measured_render_time_cpu(RID p_viewport) const;
  float viewport_get_measured_render_time_gpu(RID p_viewport) const;

  void viewport_set_snap_2d_transforms_to_pixel(RID p_viewport, bool p_enabled);
  void viewport_set_snap_2d_vertices_to_pixel(RID p_viewport, bool p_enabled);

  // void viewport_set_default_canvas_item_texture_filter(RID p_viewport, RS::CanvasItemTextureFilter p_filter);
  // void viewport_set_default_canvas_item_texture_repeat(RID p_viewport, RS::CanvasItemTextureRepeat p_repeat);

  // void viewport_set_sdf_oversize_and_scale(RID p_viewport, RS::ViewportSDFOversize p_over_size, RS::ViewportSDFScale p_scale);

  // virtual RID viewport_find_from_screen_attachment(DisplayServer::WindowID p_id = DisplayServer::MAIN_WINDOW_ID) const;

  // void viewport_set_vrs_mode(RID p_viewport, RS::ViewportVRSMode p_mode);
  // void viewport_set_vrs_update_mode(RID p_viewport, RS::ViewportVRSUpdateMode p_mode);
  void viewport_set_vrs_texture(RID p_viewport, RID p_texture);

  void handle_timestamp(String p_timestamp, uint64_t p_cpu_time, uint64_t p_gpu_time);

  void draw_viewports(bool p_swap_buffers);

  bool free(RID p_rid);
  private:
  int total_objects_drawn = 0;
	int total_vertices_drawn = 0;
	int total_draw_calls_used = 0;
  public:
  int get_total_objects_drawn() const;
  int get_total_primitives_drawn() const;
  int get_total_draw_calls_used() const;
  int get_num_viewports_with_motion_vectors() const;

  // Workaround for setting this on thread.
  // void call_set_vsync_mode(DisplayServer::VSyncMode p_mode, DisplayServer::WindowID p_window);

  RendererViewport();
  int draw_viewports_pass = 0;
  virtual ~RendererViewport() {}

 private:
	Vector<Viewport *> _sort_active_viewports();

  void _viewport_set_size(Viewport* p_viewport, int p_width, int p_height, uint32_t p_view_count);
  void _configure_3d_render_buffers(Viewport* p_viewport);
	void _draw_viewport(Viewport *p_viewport);
  void _draw_3d(Viewport* p_viewport);
  bool _viewport_requires_motion_vectors(Viewport* p_viewport);
	int occlusion_rays_per_thread = 512;

};

}  // namespace lain
#endif
