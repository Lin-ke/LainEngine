#include "rendering_context_driver.h"
namespace lain {
RenderingContextDriver::~RenderingContextDriver() {
}


Error RenderingContextDriver::window_create(WindowSystem::WindowID p_window, const void* p_platform_data) {
	SurfaceID surface = surface_create(p_platform_data);
	if (surface != 0) {
		window_surface_map[p_window] = surface;
		return OK;
	}
	else {
		return ERR_CANT_CREATE;
	}
}

void RenderingContextDriver::window_set_size(WindowSystem::WindowID p_window, uint32_t p_width, uint32_t p_height) {
	SurfaceID surface = surface_get_from_window(p_window);
	if (surface) {
		surface_set_size(surface, p_width, p_height);
	}
}

void RenderingContextDriver::window_set_vsync_mode(WindowSystem::WindowID p_window,  WindowSystem::VSyncMode p_vsync_mode) {
	SurfaceID surface = surface_get_from_window(p_window);
	if (surface) {
		surface_set_vsync_mode(surface, p_vsync_mode);
	}
}

WindowSystem::VSyncMode RenderingContextDriver::window_get_vsync_mode(WindowSystem::WindowID p_window) const {
	SurfaceID surface = surface_get_from_window(p_window);
	if (surface) {
		return surface_get_vsync_mode(surface);
	}
	else {
		return WindowSystem::VSYNC_DISABLED;
	}
}

void RenderingContextDriver::window_destroy(WindowSystem::WindowID p_window) {
	SurfaceID surface = surface_get_from_window(p_window);
	if (surface) {
		surface_destroy(surface);
	}

	window_surface_map.erase(p_window);
}



RenderingContextDriver::SurfaceID RenderingContextDriver::surface_get_from_window(WindowSystem::WindowID p_window) const {
	HashMap<WindowSystem::WindowID, SurfaceID>::ConstIterator it = window_surface_map.find(p_window);
	if (it != window_surface_map.end()) {
		return it->value;
	}
	else {
		return SurfaceID(); // return 0;
	}


}


}