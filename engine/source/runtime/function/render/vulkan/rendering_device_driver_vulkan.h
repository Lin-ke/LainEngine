
#ifndef RENDERING_DEVICE_DRIVER_VULKAN_H
#define RENDERING_DEVICE_DRIVER_VULKAN_H
#include "vulkan_header.h"

#include "core/templates/hash_map.h"
#include "core/templates/paged_allocator.h"
#include "function/render/common/rendering_device_driver.h"
#include "rendering_context_driver_vulkan.h"
#include "core/templates/hash_set.h"


namespace lain::graphics {

// Design principles:
// - Vulkan structs are zero-initialized and fields not requiring a non-zero value are omitted (except in cases where expresivity reasons apply).
class RenderingDeviceDriverVulkan : public RenderingDeviceDriver {
	/// ---apis
	struct CommandQueue;
	struct SwapChain;
public:

	/*****************/
	/**** TEXTURE ****/
	/*****************/

	struct TextureInfo {
		VkImageView vk_view = VK_NULL_HANDLE;
		DataFormat rd_format = DATA_FORMAT_MAX;
		VkImageCreateInfo vk_create_info = {};
		VkImageViewCreateInfo vk_view_create_info = {};
		struct {
			VmaAllocation handle = nullptr;
			VmaAllocationInfo info = {};
		} allocation; // All 0/null if just a view.
	};
	virtual BitField<TextureUsageBits> texture_get_usages_supported_by_format(DataFormat p_format, bool p_cpu_readable) override;
	virtual uint64_t texture_get_allocation_size(TextureID p_texture) override final;

	virtual TextureID texture_create(const TextureFormat& p_format, const TextureView& p_view) override final;

private:
	VkSampleCountFlagBits _ensure_supported_sample_count(TextureSamples p_requested_sample_count);
public:
	/****************/
	/**** FENCES ****/
	/****************/

private:
	struct Fence {
		VkFence vk_fence = VK_NULL_HANDLE;
		CommandQueue* queue_signaled_from = nullptr;
	};

public:
	virtual FenceID fence_create() override final;
	virtual Error fence_wait(FenceID p_fence) override final;
	virtual void fence_free(FenceID p_fence) override final;

	/********************/
	/**** SEMAPHORES ****/
	/********************/

	virtual SemaphoreID semaphore_create() override final;
	virtual void semaphore_free(SemaphoreID p_semaphore) override final;
	/******************/
	/**** COMMANDS ****/
	/******************/

	// ----- QUEUE FAMILY -----

	 virtual CommandQueueFamilyID command_queue_family_get(BitField<CommandQueueFamilyBits> p_cmd_queue_family_bits, RenderingContextDriver::SurfaceID p_surface = 0) override final;

	 // ----- QUEUE -----
private:
	struct CommandQueue {
		LocalVector<VkSemaphore> present_semaphores;
		LocalVector<VkSemaphore> image_semaphores;
		LocalVector<SwapChain*> image_semaphores_swap_chains; // --- image semaphore的index 到 swap chain
		LocalVector<uint32_t> pending_semaphores_for_execute;
		LocalVector<uint32_t> pending_semaphores_for_fence; // 需要给fence信号的信号量
		LocalVector<uint32_t> free_image_semaphores; // --- 等待清理
		LocalVector<Pair<Fence*, uint32_t>> image_semaphores_for_fences; // fence to image_semaphores index，需要给fence的信号量加入这里
		uint32_t queue_family = 0; // 通过这个索引到vkqueue
		uint32_t queue_index = 0;
		uint32_t present_semaphore_index = 0;
	};
//
//public:
	virtual CommandQueueID command_queue_create(CommandQueueFamilyID p_cmd_queue_family, bool p_identify_as_main_queue) override final;
	virtual Error command_queue_execute_and_present(CommandQueueID p_cmd_queue, VectorView<SemaphoreID> p_wait_semaphores, VectorView<CommandBufferID> p_cmd_buffers, VectorView<SemaphoreID> p_cmd_semaphores, FenceID p_cmd_fence, VectorView<SwapChainID> p_swap_chains) override final;
	virtual void command_queue_free(CommandQueueID p_cmd_queue) override final;
// 
// 		
	/********************/
	/**** SWAP CHAIN ****/
	/********************/

private:
	// FrameBuffer和VKImage都是屏幕所用的
	// FrameBuffer hold the Images, Renderpass use the format
	struct SwapChain {
		VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE;
		RenderingContextDriver::SurfaceID surface = RenderingContextDriver::SurfaceID();
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		TightLocalVector<VkImage> images;
		TightLocalVector<VkImageView> image_views;
		TightLocalVector<FramebufferID> framebuffers;
		LocalVector<CommandQueue*> command_queues_acquired; //
		LocalVector<uint32_t> command_queues_acquired_semaphores; // semaphore in command queue index
		uint32_t image_index = 0;
		RenderPassID render_pass;
		VkPresentModeKHR active_present_mode = VK_PRESENT_MODE_FIFO_KHR;
	};
public:
	virtual SwapChainID swap_chain_create(RenderingContextDriver::SurfaceID p_surface) override final;
	virtual Error swap_chain_resize(CommandQueueID p_cmd_queue, SwapChainID p_swap_chain, uint32_t p_desired_framebuffer_count) override final;
	/*********************/
	/**** FRAMEBUFFER ****/
	/*********************/

	virtual FramebufferID framebuffer_create(RenderPassID p_render_pass, VectorView<TextureID> p_attachments, uint32_t p_width, uint32_t p_height) override final;
	virtual void framebuffer_free(FramebufferID p_framebuffer) override final;


//	/*********************/
	/**** UNIFORM SET ****/
	/*********************/

	// Descriptor sets require allocation from a pool.
	// The documentation on how to use pools properly
	// is scarce, and the documentation is strange.
	//
	// Basically, you can mix and match pools as you
	// like, but you'll run into fragmentation issues.
	// Because of this, the recommended approach is to
	// create a pool for every descriptor set type, as
	// this prevents fragmentation.
	//
	// This is implemented here as a having a list of
	// pools (each can contain up to 64 sets) for each
	// set layout. The amount of sets for each type
	// is used as the key.
private:
	static const uint32_t MAX_UNIFORM_POOL_ELEMENT = 65535;
	uint32_t max_descriptor_sets_per_pool = 0;

	/******************/
	/**** PIPELINE ****/
	/******************/
private:
	struct PipelineCacheHeader {
		uint32_t magic = 0;
		uint32_t data_size = 0;
		uint64_t data_hash = 0;
		uint32_t vendor_id = 0;
		uint32_t device_id = 0;
		uint32_t driver_version = 0;
		uint8_t uuid[VK_UUID_SIZE] = {};
		uint8_t driver_abi = 0;
	};

	struct PipelineCache {
		String file_path;
		size_t current_size = 0;
		Vector<uint8_t> buffer; // Header then data.
		VkPipelineCache vk_cache = VK_NULL_HANDLE;
	};

	static int caching_instance_count;
	PipelineCache pipelines_cache;
	String pipeline_cache_id;

	/*****************/
	/**** GENERIC ****/
	/*****************/
private:
	struct Queue {
		VkQueue queue = VK_NULL_HANDLE;
		uint32_t virtual_count = 0; // 这个队列被使用的次数 ，暂不用
		BinaryMutex submit_mutex;
	};
	struct ShaderCapabilities {
		bool shader_float16_is_supported = false;
		bool shader_int8_is_supported = false;
	};
	struct VRSCapabilities {
		bool pipeline_vrs_supported = false; // We can specify our fragment rate on a pipeline level.
		bool primitive_vrs_supported = false; // We can specify our fragment rate on each drawcall.
		bool attachment_vrs_supported = false; // We can provide a density map attachment on our framebuffer.

		Size2i min_texel_size;
		Size2i max_texel_size;

		Size2i texel_size; // The texel size we'll use
	};
	struct StorageBufferCapabilities {
		bool storage_buffer_16_bit_access_is_supported = false;
		bool uniform_and_storage_buffer_16_bit_access_is_supported = false;
		bool storage_push_constant_16_is_supported = false;
		bool storage_input_output_16 = false;
	};
	struct MultiviewCapabilities {
		bool is_supported = false;
		bool geometry_shader_is_supported = false;
		bool tessellation_shader_is_supported = false;
		uint32_t max_view_count = 0;
		uint32_t max_instance_count = 0;
	};

	struct SubgroupCapabilities {
		uint32_t size = 0;
		uint32_t min_size = 0;
		uint32_t max_size = 0;
		VkShaderStageFlags supported_stages = 0;
		VkSubgroupFeatureFlags supported_operations = 0;
		VkBool32 quad_operations_in_all_stages = false;
		bool size_control_is_supported = false;

		uint32_t supported_stages_flags_rd() const;
		String supported_stages_desc() const;
		uint32_t supported_operations_flags_rd() const;
		String supported_operations_desc() const;
	};
	struct PipelineCacheControlCapabilities {
		bool is_supported;
	};

	struct DeviceFunctions {
		PFN_vkCreateSwapchainKHR CreateSwapchainKHR = nullptr;
		PFN_vkDestroySwapchainKHR DestroySwapchainKHR = nullptr;
		PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR = nullptr;
		PFN_vkAcquireNextImageKHR AcquireNextImageKHR = nullptr;
		PFN_vkQueuePresentKHR QueuePresentKHR = nullptr;
		PFN_vkCreateRenderPass2KHR CreateRenderPass2KHR = nullptr;
	};
	/// --- device ---
	VkDevice vk_device = VK_NULL_HANDLE;
	RenderingContextDriverVulkan* context_driver = nullptr;
	RenderingContextDriver::Device context_device = {};
	VkPhysicalDevice physical_device = {};
	VkPhysicalDeviceFeatures physical_device_features = {};
	VkPhysicalDeviceFeatures requested_device_features = {};
	RDD::Capabilities device_capabilities;
	SubgroupCapabilities subgroup_capabilities;
	MultiviewCapabilities multiview_capabilities;
	VRSCapabilities vrs_capabilities;
	StorageBufferCapabilities storage_buffer_capabilities;
	ShaderCapabilities shader_capabilities;
	PipelineCacheControlCapabilities pipeline_cache_control_capabilities;
	DeviceFunctions device_functions;
	/// --- memory ---
	VmaAllocator allocator = nullptr;
	HashMap<uint32_t, VmaPool> small_allocs_pools; // index to pool
	VmaPool _find_or_create_small_allocs_pool(uint32_t p_mem_type_index);

	/// --extensions ---
	VkPhysicalDeviceProperties physical_device_properties = {};
	HashSet<CharString> enabled_device_extension_names;
	HashMap<CharString, bool> requested_device_extensions;

	// 我觉得这个形式比较好
	struct QueueFamilyInfo {
		TightLocalVector<Queue> physical_queue;
		VkQueueFamilyProperties properties;
	};
	/// --- queue ---
	TightLocalVector<TightLocalVector<Queue>> queue_families; // --- each queue family may have more than one queue.
	TightLocalVector<VkQueueFamilyProperties> queue_family_properties;

	/// --- frame ---
	uint32_t frame_count = 1;
	uint32_t present_frame_latency = 0;


	virtual Error initialize(uint32_t p_device_index, uint32_t p_frame_count);
	void finatialize();

	

	virtual void set_object_name(ObjectType p_type, ID p_driver_id, const String& p_name) override;
	

private:
	/// ---tools function ---
	Error _initialize_device_extensions();
	Error _check_device_features();
	Error _check_device_capabilities();
	void _register_requested_device_extension(const CharString&, bool);
	/// --- queue create ---
	Error _add_queue_create_info(LocalVector<VkDeviceQueueCreateInfo>& r_queue_create_info);
	/// --- device---
	Error _initialize_device(const LocalVector<VkDeviceQueueCreateInfo>& p_queue_create_info);
	Error _initialize_allocator();
	Error _initialize_pipeline_cache();
	void _set_object_name(VkObjectType p_object_type, uint64_t p_object_handle, String p_object_name);
	void _swap_chain_release(SwapChain* swap_chain);
	VkResult _create_render_pass(VkDevice p_device, const VkRenderPassCreateInfo2* p_create_info, const VkAllocationCallbacks* p_allocator, VkRenderPass* p_render_pass);
	// used in swap_chain recreate.
	bool _release_image_semaphore(CommandQueue* p_command_queue, uint32_t p_semaphore_index, bool p_release_on_swap_chain);
	bool _recreate_image_semaphore(CommandQueue* p_command_queue, uint32_t p_semaphore_index, bool p_release_on_swap_chain);

	public:
		RenderingDeviceDriverVulkan(RenderingContextDriverVulkan* p_context_driver);
		virtual ~RenderingDeviceDriverVulkan();

};


}

#endif // RENDERING_DEVICE_DRIVER_VULKAN_H
