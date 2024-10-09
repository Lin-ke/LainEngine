#ifndef RENDERER_VIEWPORT_API_H
#define RENDERER_VIEWPORT_API_H
#include "rendering_system.h"
namespace lain {
class RendererViewport {
 public:
  struct ViewPort {
    RID self;
    RID parent;

    Size2i size;
    uint32_t view_count = 1;
    RID camera;
    RID scenario;

    WindowSystem::WindowID viewport_to_screen;
    Rect2 viewport_to_screen_rect;
    bool viewport_render_direct_to_screen;

    // statistics
    uint64_t time_cpu_begin = 0;
    uint64_t time_cpu_end = 0;

    uint64_t time_gpu_begin = 0;
    uint64_t time_gpu_end = 0;
  };
  mutable RID_Owner<RendererViewport::ViewPort, true> viewport_owner;

  Vector<Viewport*> active_viewports;
  Vector<Viewport*> sorted_active_viewports;
  RID viewport_allocate();
  void viewport_initialize(RID p_rid);
  int32_t get_num_viewports_with_motion_vectors() const;
  void handle_timestamp(String p_timestamp, uint64_t p_cpu_time, uint64_t p_gpu_time);
};

}  // namespace lain
#endif
