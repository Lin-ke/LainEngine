#pragma once
#ifndef RENDERING_CONTEXT_DRIVER_VULKAN_H
#define RENDERING_CONTEXT_DRIVER_VULKAN_H
#include "vulkan_header.h"
#include "function/render/common/rendering_context_driver.h"
#include "core/templates/local_vector.h"
#include "function/display/window_system.h"
#include "core/templates/hash_set.h"

namespace lain::graphics {
	/// --- physical device management ---///
	// 多GPU的情况下的设备管理和基础拓展的启用

class RenderingContextDriverVulkan : public RenderingContextDriver {
public:
	/// <summary>
	/// get physical device; surface;
	/// debug
	/// </summary>
	struct Functions {
		// Instance
		PFN_vkGetInstanceProcAddr GetInstanceProcAddr = nullptr;
		// Physical device.
		PFN_vkGetPhysicalDeviceFeatures2 GetPhysicalDeviceFeatures2 = nullptr;
		PFN_vkGetPhysicalDeviceProperties2 GetPhysicalDeviceProperties2 = nullptr;

		// Device.
		PFN_vkGetDeviceProcAddr GetDeviceProcAddr = nullptr;

		// Surfaces.
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR = nullptr;
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR = nullptr;
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR GetPhysicalDeviceSurfacePresentModesKHR = nullptr;

		// Debug utils.
		PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT = nullptr;
		PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT = nullptr;
		PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT = nullptr;
		PFN_vkCmdEndDebugUtilsLabelEXT CmdEndDebugUtilsLabelEXT = nullptr;
		PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT = nullptr;

		bool debug_util_functions_available() const {
			return CreateDebugUtilsMessengerEXT != nullptr &&
				DestroyDebugUtilsMessengerEXT != nullptr &&
				CmdBeginDebugUtilsLabelEXT != nullptr &&
				CmdEndDebugUtilsLabelEXT != nullptr &&
				SetDebugUtilsObjectNameEXT != nullptr;
		}


		// Debug report.
		PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallbackEXT = nullptr;
		PFN_vkDebugReportMessageEXT DebugReportMessageEXT = nullptr;
		PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallbackEXT = nullptr;

		bool debug_report_functions_available() const {
			return CreateDebugReportCallbackEXT != nullptr &&
				DebugReportMessageEXT != nullptr &&
				DestroyDebugReportCallbackEXT != nullptr;
		}
	};
	private:
	VkInstance instance = VK_NULL_HANDLE;
	uint32_t instance_api_version = VK_API_VERSION_1_3;
	VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
	VkDebugReportCallbackEXT debug_report = VK_NULL_HANDLE;
	Functions functions;
	/// --- device
	struct DeviceQueueFamilies {
		TightLocalVector<VkQueueFamilyProperties> properties;
	};
	TightLocalVector<Device> driver_devices;
	TightLocalVector<VkPhysicalDevice> physical_devices;
	TightLocalVector<DeviceQueueFamilies> device_queue_families;

	/// --- extensions
	HashMap<CharString, bool> requested_instance_extensions;
	HashSet<CharString> enabled_instance_extension_names;
	
	
	/// -- initial
	Error _initialize_vulkan_version();
	void _register_requested_instance_extension(const CharString& p_extension_name, bool p_required);
	Error _initialize_instance_extensions();
	Error _initialize_instance();
	Error _initialize_devices();

	Error _find_validation_layers(TightLocalVector<const char*>& r_layer_names) const;


	/// ---platform related
	virtual const char* _get_platform_surface_extension() const { return nullptr; }
	// Static callbacks.
	// Can be overridden by platform-specific drivers.
	virtual bool _use_validation_layers() const;
	virtual Error _create_vulkan_instance(const VkInstanceCreateInfo* p_create_info, VkInstance* r_instance);

	static VKAPI_ATTR VkBool32 VKAPI_CALL _debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT p_message_severity, VkDebugUtilsMessageTypeFlagsEXT p_message_type, const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data);
	static VKAPI_ATTR VkBool32 VKAPI_CALL _debug_report_callback(VkDebugReportFlagsEXT p_flags, VkDebugReportObjectTypeEXT p_object_type, uint64_t p_object, size_t p_location, int32_t p_message_code, const char* p_layer_prefix, const char* p_message, void* p_user_data);


public:
	virtual Error initialize() override;
	virtual bool is_debug_utils_enabled() const { return enabled_instance_extension_names.has(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); };
	// ---device
	virtual const Device& device_get(uint32_t p_device_index) const override;
	virtual uint32_t device_get_count() const override;
	virtual bool device_supports_present(uint32_t p_device_index, SurfaceID p_surface) const override;
	// ---driver
	virtual RenderingDeviceDriver* driver_create() override;
	virtual void driver_free(RenderingDeviceDriver* p_driver) override;
	// --- surface
	virtual void surface_set_size(SurfaceID p_surface, uint32_t p_width, uint32_t p_height) override;
	virtual void surface_set_vsync_mode(SurfaceID p_surface, WindowSystem::VSyncMode p_vsync_mode) override;
	virtual WindowSystem::VSyncMode surface_get_vsync_mode(SurfaceID p_surface) const override;
	virtual uint32_t surface_get_width(SurfaceID p_surface) const override;
	virtual uint32_t surface_get_height(SurfaceID p_surface) const override;
	virtual void surface_set_needs_resize(SurfaceID p_surface, bool p_needs_resize) override;
	virtual bool surface_get_needs_resize(SurfaceID p_surface) const override;
	virtual void surface_destroy(SurfaceID p_surface) override;
	// 应该在platform里创建
	virtual SurfaceID surface_create(const void* p_platform_data) override;
	
	// Vulkan-only methods.
	struct Surface {
		VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
		uint32_t width = 0;
		uint32_t height = 0;
		WindowSystem::VSyncMode vsync_mode = WindowSystem::VSYNC_ENABLED;
		bool needs_resize = false;
	};

	VkInstance instance_get() const;
	VkPhysicalDevice physical_device_get(uint32_t p_device_index) const;
	uint32_t queue_family_get_count(uint32_t p_device_index) const;
	VkQueueFamilyProperties queue_family_get(uint32_t p_device_index, uint32_t p_queue_family_index) const;
	bool queue_family_supports_present(VkPhysicalDevice p_physical_device, uint32_t p_queue_family_index, SurfaceID p_surface) const;
	const Functions& functions_get() const;

	RenderingContextDriverVulkan() {}
	virtual ~RenderingContextDriverVulkan();
};
}
#endif
