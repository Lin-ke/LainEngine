#include "node_3d.h"
#include "scene/main/viewport.h"
using namespace lain;

Ref<World3D> GObject3D::get_world_3d() const {
	ERR_READ_THREAD_GUARD_V(Ref<World3D>()); // World3D can only be set from main thread, so it's safe to obtain on threads.
	ERR_FAIL_COND_V(!is_inside_world(), Ref<World3D>());
	ERR_FAIL_NULL_V(data.viewport, Ref<World3D>());

	return data.viewport->find_world_3d();
}

void lain::GObject3D::set_notify_transform(bool p_enabled) {
		ERR_THREAD_GUARD;
	data.notify_transform = p_enabled;
}
void GObject3D::set_disable_scale(bool p_enabled) {
	ERR_THREAD_GUARD;
	data.disable_scale = p_enabled;
}