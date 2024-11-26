#include "viewport.h"

using namespace lain;

Rect2 lain::Viewport::get_visible_rect() const {
	ERR_READ_THREAD_GUARD_V(Rect2());
	Rect2 r;

	if (size == Size2()) {
		r = Rect2(Point2(), WindowSystem::GetSingleton()->window_get_size());
	} else {
		r = Rect2(Point2(), size);
	}

	if (size_2d_override != Size2i()) {
		r.size = size_2d_override;
	}

	return r;
}

Ref<World3D> Viewport::get_world_3d() const {
  ERR_READ_THREAD_GUARD_V(Ref<World3D>());
  return world_3d;
}
