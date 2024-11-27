#include "compositor.h"
using namespace lain;
CompositorEffect::CompositorEffect() {
  RenderingSystem *rs = RenderingSystem::get_singleton();
	// if (rs != nullptr) {
		rid = rs->compositor_effect_create();
	// 	rs->compositor_effect_set_callback(rid, RenderingServer::CompositorEffectCallbackType(effect_callback_type), Callable(this, "_render_callback"));
	// }
}

lain::CompositorEffect::~CompositorEffect()
{
  RenderingSystem *rs = RenderingSystem::get_singleton();
	if (rs != nullptr && rid.is_valid()) {
		rs->free(rid);
	}
}


Compositor::Compositor() {
	RenderingSystem *rs = RenderingSystem::get_singleton();
	if (rs != nullptr) {
		compositor = rs->compositor_create();
	}
}

lain::Compositor::~Compositor() {
	RS *rs = RS::get_singleton();
	if (rs != nullptr && compositor.is_valid()) {
		rs->free(compositor);
	}
}
