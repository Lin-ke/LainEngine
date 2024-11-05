#ifndef RenderBufferCustomDataRD_H
#define RenderBufferCustomDataRD_H
#include "core/object/refcounted.h"
namespace lain::RendererRD {
  class RenderSceneBuffersRD;
  class RenderBufferCustomDataRD : public RefCounted {
    LCLASS(RenderBufferCustomDataRD, RefCounted);

  public:
    virtual void configure(RenderSceneBuffersRD *p_render_buffers) = 0;
    virtual void free_data() = 0; // called on cleanup

  private:
  };
}

#endif // !1