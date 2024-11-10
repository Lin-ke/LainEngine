
#ifndef RENDERING_CONTEXT_DRIVER_H
#define RENDERING_CONTEXT_DRIVER_H
// 建立Display和抽象Surface的关系

#include "core/object/object.h"
#include "function/display/window_system.h"
#define NVIDIA_VENDOR_NAME "NVIDIA"
namespace lain {
class RenderingDeviceDriver;

class RenderingContextDriver {
public:
	typedef uint64_t SurfaceID;

private:
	HashMap<WindowSystem::WindowID, SurfaceID> window_surface_map;

public:
// vulkan surface通过 platform()创建
// swapchain在 renderingdevice里创建。
	SurfaceID surface_get_from_window(WindowSystem::WindowID p_window) const;
	Error window_create(WindowSystem::WindowID p_window, const void* p_platform_data);
	void window_set_size(WindowSystem::WindowID p_window, uint32_t p_width, uint32_t p_height);
	void window_set_vsync_mode(WindowSystem::WindowID p_window, WindowSystem::VSyncMode p_vsync_mode);
	WindowSystem::VSyncMode window_get_vsync_mode(WindowSystem::WindowID p_window) const;
	void window_destroy(WindowSystem::WindowID p_window);

public:
	//https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkVendorId.html
	enum Vendor {
		VENDOR_UNKNOWN = 0x0,
		VENDOR_AMD = 0x1002,
		VENDOR_IMGTEC = 0x1010,
		VENDOR_APPLE = 0x106B,
		VENDOR_NVIDIA = 0x10DE,
		VENDOR_ARM = 0x13B5,
		VENDOR_MICROSOFT = 0x1414,
		VENDOR_QUALCOMM = 0x5143,
		VENDOR_INTEL = 0x8086
	};

	enum DeviceType {
		DEVICE_TYPE_OTHER = 0x0,
		DEVICE_TYPE_INTEGRATED_GPU = 0x1,
		DEVICE_TYPE_DISCRETE_GPU = 0x2,
		DEVICE_TYPE_VIRTUAL_GPU = 0x3,
		DEVICE_TYPE_CPU = 0x4,
		DEVICE_TYPE_MAX = 0x5
	};

	struct Device {
		String name = "Unknown";
		Vendor vendor = VENDOR_UNKNOWN;
		DeviceType type = DEVICE_TYPE_OTHER;
	};

	virtual ~RenderingContextDriver();
	virtual Error initialize() = 0;
	virtual const Device& device_get(uint32_t p_device_index) const = 0;
	virtual uint32_t device_get_count() const = 0;
	virtual bool device_supports_present(uint32_t p_device_index, SurfaceID p_surface) const = 0;
	virtual RenderingDeviceDriver* driver_create() = 0;
	virtual void driver_free(RenderingDeviceDriver* p_driver) = 0;
	virtual SurfaceID surface_create(const void* p_platform_data) = 0;
	virtual void surface_set_size(SurfaceID p_surface, uint32_t p_width, uint32_t p_height) = 0;
	virtual void surface_set_vsync_mode(SurfaceID p_surface, WindowSystem::VSyncMode p_vsync_mode) = 0;
	virtual WindowSystem::VSyncMode surface_get_vsync_mode(SurfaceID p_surface) const = 0;
	virtual uint32_t surface_get_width(SurfaceID p_surface) const = 0;
	virtual uint32_t surface_get_height(SurfaceID p_surface) const = 0;
	virtual void surface_set_needs_resize(SurfaceID p_surface, bool p_needs_resize) = 0;
	virtual bool surface_get_needs_resize(SurfaceID p_surface) const = 0;
	virtual void surface_destroy(SurfaceID p_surface) = 0;
	virtual bool is_debug_utils_enabled() const = 0;
};
using RCD = RenderingContextDriver;
}

#endif // RENDERING_CONTEXT_DRIVER_H
