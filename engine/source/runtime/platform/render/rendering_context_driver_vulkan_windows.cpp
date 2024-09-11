#include "rendering_context_driver_vulkan_windows.h"
#include "core/os/os.h"
using namespace lain;
// 这里是于surface创建相关的代码
const char* RenderingContextDriverVulkanWindows::_get_platform_surface_extension() const {
	return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}

RenderingContextDriverVulkanWindows::RenderingContextDriverVulkanWindows() {
	// Workaround for Vulkan not working on setups with AMD integrated graphics + NVIDIA dedicated GPU (GH-57708).
	// This prevents using AMD integrated graphics with Vulkan entirely, but it allows the engine to start
	// even on outdated/broken driver setups.
	// @?
	OS::GetSingleton()->SetEnvironment("DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1", "1");
}

RenderingContextDriverVulkanWindows::~RenderingContextDriverVulkanWindows() {
	// Does nothing.
}

RenderingContextDriver::SurfaceID RenderingContextDriverVulkanWindows::surface_create(const void* p_platform_data) {
	const WindowPlatformData* wpd = (const WindowPlatformData*)(p_platform_data);

	VkWin32SurfaceCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hinstance = wpd->instance;
	create_info.hwnd = wpd->window;

	VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
	auto win32_create_surface = PFN_vkCreateWin32SurfaceKHR(vkGetInstanceProcAddr(instance_get(), "vkCreateWin32SurfaceKHR"));
	VkResult err = win32_create_surface(instance_get(), &create_info, nullptr, &vk_surface);
	ERR_FAIL_COND_V(err != VK_SUCCESS, SurfaceID());

	Surface* surface = memnew(Surface);
	surface->vk_surface = vk_surface;
	return SurfaceID(surface);
}

