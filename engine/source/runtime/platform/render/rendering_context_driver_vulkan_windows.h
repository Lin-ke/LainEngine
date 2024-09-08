#pragma once

#ifndef RENDERING_CONTEXT_DRIVER_VULKAN_WINDOWS_H
#define RENDERING_CONTEXT_DRIVER_VULKAN_WINDOWS_H


#include "function/render/vulkan/rendering_context_driver_vulkan.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
namespace lain {

	class RenderingContextDriverVulkanWindows : public RenderingContextDriverVulkan {
	private:
		const char* _get_platform_surface_extension() const override final;

	protected:
		SurfaceID surface_create(const void* p_platform_data) override final;

	public:
		struct WindowPlatformData {
			HWND window;
			HINSTANCE instance;
		};

		RenderingContextDriverVulkanWindows();
		~RenderingContextDriverVulkanWindows() override;
	};
}

#endif
