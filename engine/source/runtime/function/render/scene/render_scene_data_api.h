#ifndef RENDER_SCENE_DATA_H
#define RENDER_SCENE_DATA_H
#include "core/object/object.h"
namespace lain {
  // Abstract render data object, holds scene data related to rendering a single frame of a viewport.
class RenderSceneData : public Object {
  LCLASS(RenderSceneData, Object);

 public:
  virtual Transform3D get_cam_transform() const = 0;
  virtual Projection get_cam_projection() const = 0;

  virtual uint32_t get_view_count() const = 0;
  virtual Vector3 get_view_eye_offset(uint32_t p_view) const = 0;
  virtual Projection get_view_projection(uint32_t p_view) const = 0;

  virtual RID get_uniform_buffer() const = 0;
};

}  // namespace lain
#endif