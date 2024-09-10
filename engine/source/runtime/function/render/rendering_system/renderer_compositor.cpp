#include "renderer_compositor.h"
using namespace lain;
RendererCompositor *RendererCompositor::singleton = nullptr;
RendererCompositor *RendererCompositor::create() {
	return _create_func();
}

bool RendererCompositor::is_xr_enabled() const {
	return xr_enabled;
}

lain::RendererCompositor::RendererCompositor() {
	singleton = this;
}

RendererCompositor::~RendererCompositor() {
	singleton = nullptr;
}
