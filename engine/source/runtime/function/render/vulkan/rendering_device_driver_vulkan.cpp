#include "rendering_device_driver_vulkan.h"
#include "core/config/project_settings.h"
#include "core/string/print_string.h"
/*****************/
/**** GENERIC ****/
/*****************/
namespace lain{
	namespace graphics {
// 不要离开这个文件
static const uint32_t SMALL_ALLOCATION_MAX_SIZE = 4096;

static const VkFormat RD_TO_VK_FORMAT[RDD::DATA_FORMAT_MAX] = {
	VK_FORMAT_R4G4_UNORM_PACK8,
	VK_FORMAT_R4G4B4A4_UNORM_PACK16,
	VK_FORMAT_B4G4R4A4_UNORM_PACK16,
	VK_FORMAT_R5G6B5_UNORM_PACK16,
	VK_FORMAT_B5G6R5_UNORM_PACK16,
	VK_FORMAT_R5G5B5A1_UNORM_PACK16,
	VK_FORMAT_B5G5R5A1_UNORM_PACK16,
	VK_FORMAT_A1R5G5B5_UNORM_PACK16,
	VK_FORMAT_R8_UNORM,
	VK_FORMAT_R8_SNORM,
	VK_FORMAT_R8_USCALED,
	VK_FORMAT_R8_SSCALED,
	VK_FORMAT_R8_UINT,
	VK_FORMAT_R8_SINT,
	VK_FORMAT_R8_SRGB,
	VK_FORMAT_R8G8_UNORM,
	VK_FORMAT_R8G8_SNORM,
	VK_FORMAT_R8G8_USCALED,
	VK_FORMAT_R8G8_SSCALED,
	VK_FORMAT_R8G8_UINT,
	VK_FORMAT_R8G8_SINT,
	VK_FORMAT_R8G8_SRGB,
	VK_FORMAT_R8G8B8_UNORM,
	VK_FORMAT_R8G8B8_SNORM, 
	VK_FORMAT_R8G8B8_USCALED,
	VK_FORMAT_R8G8B8_SSCALED,
	VK_FORMAT_R8G8B8_UINT,
	VK_FORMAT_R8G8B8_SINT,
	VK_FORMAT_R8G8B8_SRGB,
	VK_FORMAT_B8G8R8_UNORM,
	VK_FORMAT_B8G8R8_SNORM,
	VK_FORMAT_B8G8R8_USCALED,
	VK_FORMAT_B8G8R8_SSCALED,
	VK_FORMAT_B8G8R8_UINT,
	VK_FORMAT_B8G8R8_SINT,
	VK_FORMAT_B8G8R8_SRGB,
	VK_FORMAT_R8G8B8A8_UNORM,
	VK_FORMAT_R8G8B8A8_SNORM,
	VK_FORMAT_R8G8B8A8_USCALED,
	VK_FORMAT_R8G8B8A8_SSCALED,
	VK_FORMAT_R8G8B8A8_UINT,
	VK_FORMAT_R8G8B8A8_SINT,
	VK_FORMAT_R8G8B8A8_SRGB,
	VK_FORMAT_B8G8R8A8_UNORM,
	VK_FORMAT_B8G8R8A8_SNORM,
	VK_FORMAT_B8G8R8A8_USCALED,
	VK_FORMAT_B8G8R8A8_SSCALED,
	VK_FORMAT_B8G8R8A8_UINT,
	VK_FORMAT_B8G8R8A8_SINT,
	VK_FORMAT_B8G8R8A8_SRGB,
	VK_FORMAT_A8B8G8R8_UNORM_PACK32,
	VK_FORMAT_A8B8G8R8_SNORM_PACK32,
	VK_FORMAT_A8B8G8R8_USCALED_PACK32,
	VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
	VK_FORMAT_A8B8G8R8_UINT_PACK32,
	VK_FORMAT_A8B8G8R8_SINT_PACK32,
	VK_FORMAT_A8B8G8R8_SRGB_PACK32,
	VK_FORMAT_A2R10G10B10_UNORM_PACK32,
	VK_FORMAT_A2R10G10B10_SNORM_PACK32,
	VK_FORMAT_A2R10G10B10_USCALED_PACK32,
	VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
	VK_FORMAT_A2R10G10B10_UINT_PACK32,
	VK_FORMAT_A2R10G10B10_SINT_PACK32,
	VK_FORMAT_A2B10G10R10_UNORM_PACK32,
	VK_FORMAT_A2B10G10R10_SNORM_PACK32,
	VK_FORMAT_A2B10G10R10_USCALED_PACK32,
	VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
	VK_FORMAT_A2B10G10R10_UINT_PACK32,
	VK_FORMAT_A2B10G10R10_SINT_PACK32,
	VK_FORMAT_R16_UNORM,
	VK_FORMAT_R16_SNORM,
	VK_FORMAT_R16_USCALED,
	VK_FORMAT_R16_SSCALED,
	VK_FORMAT_R16_UINT,
	VK_FORMAT_R16_SINT,
	VK_FORMAT_R16_SFLOAT,
	VK_FORMAT_R16G16_UNORM,
	VK_FORMAT_R16G16_SNORM,
	VK_FORMAT_R16G16_USCALED,
	VK_FORMAT_R16G16_SSCALED,
	VK_FORMAT_R16G16_UINT,
	VK_FORMAT_R16G16_SINT,
	VK_FORMAT_R16G16_SFLOAT,
	VK_FORMAT_R16G16B16_UNORM,
	VK_FORMAT_R16G16B16_SNORM,
	VK_FORMAT_R16G16B16_USCALED,
	VK_FORMAT_R16G16B16_SSCALED,
	VK_FORMAT_R16G16B16_UINT,
	VK_FORMAT_R16G16B16_SINT,
	VK_FORMAT_R16G16B16_SFLOAT,
	VK_FORMAT_R16G16B16A16_UNORM,
	VK_FORMAT_R16G16B16A16_SNORM,
	VK_FORMAT_R16G16B16A16_USCALED,
	VK_FORMAT_R16G16B16A16_SSCALED,
	VK_FORMAT_R16G16B16A16_UINT,
	VK_FORMAT_R16G16B16A16_SINT,
	VK_FORMAT_R16G16B16A16_SFLOAT,
	VK_FORMAT_R32_UINT,
	VK_FORMAT_R32_SINT,
	VK_FORMAT_R32_SFLOAT,
	VK_FORMAT_R32G32_UINT,
	VK_FORMAT_R32G32_SINT,
	VK_FORMAT_R32G32_SFLOAT,
	VK_FORMAT_R32G32B32_UINT,
	VK_FORMAT_R32G32B32_SINT,
	VK_FORMAT_R32G32B32_SFLOAT,
	VK_FORMAT_R32G32B32A32_UINT,
	VK_FORMAT_R32G32B32A32_SINT,
	VK_FORMAT_R32G32B32A32_SFLOAT,
	VK_FORMAT_R64_UINT,
	VK_FORMAT_R64_SINT,
	VK_FORMAT_R64_SFLOAT,
	VK_FORMAT_R64G64_UINT,
	VK_FORMAT_R64G64_SINT,
	VK_FORMAT_R64G64_SFLOAT,
	VK_FORMAT_R64G64B64_UINT,
	VK_FORMAT_R64G64B64_SINT,
	VK_FORMAT_R64G64B64_SFLOAT,
	VK_FORMAT_R64G64B64A64_UINT,
	VK_FORMAT_R64G64B64A64_SINT,
	VK_FORMAT_R64G64B64A64_SFLOAT,
	VK_FORMAT_B10G11R11_UFLOAT_PACK32,
	VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
	VK_FORMAT_D16_UNORM,
	VK_FORMAT_X8_D24_UNORM_PACK32,
	VK_FORMAT_D32_SFLOAT,
	VK_FORMAT_S8_UINT,
	VK_FORMAT_D16_UNORM_S8_UINT,
	VK_FORMAT_D24_UNORM_S8_UINT,
	VK_FORMAT_D32_SFLOAT_S8_UINT,
	VK_FORMAT_BC1_RGB_UNORM_BLOCK,
	VK_FORMAT_BC1_RGB_SRGB_BLOCK,
	VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
	VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
	VK_FORMAT_BC2_UNORM_BLOCK,
	VK_FORMAT_BC2_SRGB_BLOCK,
	VK_FORMAT_BC3_UNORM_BLOCK,
	VK_FORMAT_BC3_SRGB_BLOCK,
	VK_FORMAT_BC4_UNORM_BLOCK,
	VK_FORMAT_BC4_SNORM_BLOCK,
	VK_FORMAT_BC5_UNORM_BLOCK,
	VK_FORMAT_BC5_SNORM_BLOCK,
	VK_FORMAT_BC6H_UFLOAT_BLOCK,
	VK_FORMAT_BC6H_SFLOAT_BLOCK,
	VK_FORMAT_BC7_UNORM_BLOCK,
	VK_FORMAT_BC7_SRGB_BLOCK,
	VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
	VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
	VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
	VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
	VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
	VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
	VK_FORMAT_EAC_R11_UNORM_BLOCK,
	VK_FORMAT_EAC_R11_SNORM_BLOCK,
	VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
	VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
	VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
	VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
	VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
	VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
	VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
	VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
	VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
	VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
	VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
	VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
	VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
	VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
	VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
	VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
	VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
	VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
	VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
	VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
	VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
	VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
	VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
	VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
	VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
	VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
	VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
	VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
	VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
	VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
	VK_FORMAT_G8B8G8R8_422_UNORM,
	VK_FORMAT_B8G8R8G8_422_UNORM,
	VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
	VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
	VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
	VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
	VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
	VK_FORMAT_R10X6_UNORM_PACK16,
	VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
	VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
	VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
	VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
	VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
	VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
	VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
	VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
	VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
	VK_FORMAT_R12X4_UNORM_PACK16,
	VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
	VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
	VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
	VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
	VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
	VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
	VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
	VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
	VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
	VK_FORMAT_G16B16G16R16_422_UNORM,
	VK_FORMAT_B16G16R16G16_422_UNORM,
	VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
	VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
	VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
	VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
	VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
};
static const VkSampleCountFlagBits RD_TO_VK_SAMPLE_COUNT[RDD::TEXTURE_SAMPLES_MAX] = {
	VK_SAMPLE_COUNT_1_BIT,
	VK_SAMPLE_COUNT_2_BIT,
	VK_SAMPLE_COUNT_4_BIT,
	VK_SAMPLE_COUNT_8_BIT,
	VK_SAMPLE_COUNT_16_BIT,
	VK_SAMPLE_COUNT_32_BIT,
	VK_SAMPLE_COUNT_64_BIT,
};
/*****************/
/**** TEXTURE ****/
/*****************/

static const VkImageType RD_TEX_TYPE_TO_VK_IMG_TYPE[RDD::TEXTURE_TYPE_MAX] = {
	VK_IMAGE_TYPE_1D,
	VK_IMAGE_TYPE_2D,
	VK_IMAGE_TYPE_3D,
	VK_IMAGE_TYPE_2D,
	VK_IMAGE_TYPE_1D,
	VK_IMAGE_TYPE_2D,
	VK_IMAGE_TYPE_2D,
};

struct
{
	uint32_t flags;
	RDD::CommandQueueFamilyBits index;
} static const RD_TEXTURE_QUEUE_TYPE_TO_VK_QUEUE_TYPE[] = {
	{ RDD::TextureMisc::TEXTURE_MISC_CONCURRENT_QUEUE_GRAPHICS_BIT, RDD::CommandQueueFamilyBits::COMMAND_QUEUE_FAMILY_GRAPHICS_BIT },
	{ RDD::TextureMisc::TEXTURE_MISC_CONCURRENT_QUEUE_ASYNC_COMPUTE_BIT, RDD::CommandQueueFamilyBits::COMMAND_QUEUE_FAMILY_COMPUTE_BIT },
	{ RDD::TextureMisc::TEXTURE_MISC_CONCURRENT_QUEUE_ASYNC_TRANSFER_BIT, RDD::CommandQueueFamilyBits::COMMAND_QUEUE_FAMILY_TRANSFER_BIT },
};


RDD::TextureID RenderingDeviceDriverVulkan::texture_create(const TextureFormat& p_format, const TextureView& p_view) {
	VkImageCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	if (!p_format.shareable_formats.is_empty()) {
		create_info.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT; //该图像可用于创建与图像格式不同的 VkImageView

		if (enabled_device_extension_names.has(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME)) {
			if(!p_format.shareable_formats.is_empty())
			{
				VkFormat* vk_allowed_formats = ALLOCA_ARRAY(VkFormat, p_format.shareable_formats.size());
				for (int i = 0; i < p_format.shareable_formats.size(); i++) {
					vk_allowed_formats[i] = RD_TO_VK_FORMAT[p_format.shareable_formats[i]];
				}

				VkImageFormatListCreateInfo* format_list_create_info = ALLOCA_SINGLE(VkImageFormatListCreateInfo);
				*format_list_create_info = {};
				format_list_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;
				format_list_create_info->viewFormatCount = p_format.shareable_formats.size();
				format_list_create_info->pViewFormats = vk_allowed_formats;

				create_info.pNext = format_list_create_info;
			}
		}
	}
	if (p_format.texture_type == TEXTURE_TYPE_CUBE || p_format.texture_type == TEXTURE_TYPE_CUBE_ARRAY) {
		create_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}
	if (p_format.texture_type == TEXTURE_TYPE_2D || p_format.texture_type == TEXTURE_TYPE_2D_ARRAY) {
		create_info.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
	}
	create_info.imageType = RD_TEX_TYPE_TO_VK_IMG_TYPE[p_format.texture_type];
	create_info.format = RD_TO_VK_FORMAT[p_format.format];
	create_info.extent.width = p_format.width;
	create_info.extent.height = p_format.height;
	create_info.extent.depth = p_format.depth;
	uint32_t queue_flags = p_format.misc & (TEXTURE_MISC_CONCURRENT_QUEUE_GRAPHICS_BIT |
		TEXTURE_MISC_CONCURRENT_QUEUE_ASYNC_COMPUTE_BIT |
		TEXTURE_MISC_CONCURRENT_QUEUE_ASYNC_TRANSFER_BIT |
		TEXTURE_MISC_CONCURRENT_QUEUE_VIDEO_DUPLEX);
	/*bool concurrent_queue = queue_flags != 0 ||
		staging_buffer != nullptr ||
		create_info.initial_layout != VK_IMAGE_LAYOUT_UNDEFINED;*/
	create_info.mipLevels = p_format.mipmaps;
	create_info.arrayLayers = p_format.array_layers;

	create_info.samples = _ensure_supported_sample_count(p_format.samples);
	create_info.tiling = (p_format.usage_bits & TEXTURE_USAGE_CPU_READ_BIT) ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;

	if ((p_format.usage_bits & TEXTURE_USAGE_SAMPLING_BIT)) {
		create_info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}
	if ((p_format.usage_bits & TEXTURE_USAGE_STORAGE_BIT)) {
		create_info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
	}
	if ((p_format.usage_bits & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT)) {
		create_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if ((p_format.usage_bits & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
		create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	if ((p_format.usage_bits & TEXTURE_USAGE_INPUT_ATTACHMENT_BIT)) {
		create_info.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	}
	if ((p_format.usage_bits & TEXTURE_USAGE_VRS_ATTACHMENT_BIT)) {
		create_info.usage |= VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
	}
	if ((p_format.usage_bits & TEXTURE_USAGE_CAN_UPDATE_BIT)) {
		create_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	if ((p_format.usage_bits & TEXTURE_USAGE_CAN_COPY_FROM_BIT)) {
		create_info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	if ((p_format.usage_bits & TEXTURE_USAGE_CAN_COPY_TO_BIT)) {
		create_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	bool concurrent_queue = queue_flags != 0;
	create_info.sharingMode = concurrent_queue? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
	if (concurrent_queue) {
		TightLocalVector<uint32_t> sharing_indices;

		static auto add_unique_family = [](TightLocalVector<uint32_t>& sharing_indices, uint32_t& count, Vector<CommandQueueFamilyID>& family ) {
			for (auto family_id: family) {
				uint32_t id = family_id.id - 1;
				if (sharing_indices.find(id) == -1)
					sharing_indices.push_back(id);
			}
			};
		for(auto &m : RD_TEXTURE_QUEUE_TYPE_TO_VK_QUEUE_TYPE)
			if ((queue_flags & m.flags) != 0) {
				add_unique_family(sharing_indices, create_info.queueFamilyIndexCount, queuefamily_to_ids[m.index] );
			}
		if (create_info.queueFamilyIndexCount > 1)
			create_info.pQueueFamilyIndices = sharing_indices.ptr();
		else
		{
			create_info.pQueueFamilyIndices = nullptr;
			create_info.queueFamilyIndexCount = 0;
			create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
	}
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	// Allocate memory.

	uint32_t width = 0, height = 0;
	uint32_t image_size = get_image_format_required_size(p_format.format, p_format.width, p_format.height, p_format.depth, p_format.mipmaps, &width, &height);

	VmaAllocationCreateInfo alloc_create_info = {};
	alloc_create_info.flags = (p_format.usage_bits & TEXTURE_USAGE_CPU_READ_BIT) ? VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT : 0;
	alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	if (image_size <= SMALL_ALLOCATION_MAX_SIZE) { // 小的分配用這個
		uint32_t mem_type_index = 0;
		vmaFindMemoryTypeIndexForImageInfo(allocator, &create_info, &alloc_create_info, &mem_type_index);
		alloc_create_info.pool = _find_or_create_small_allocs_pool(mem_type_index);
	}
	// Create.

	VkImage vk_image = VK_NULL_HANDLE;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo alloc_info = {};
	VkResult err = vmaCreateImage(allocator, &create_info, &alloc_create_info, &vk_image, &allocation, &alloc_info);
	ERR_FAIL_COND_V_MSG(err, TextureID(), "vmaCreateImage failed with error " + itos(err) + ".");

}

VkSampleCountFlagBits RenderingDeviceDriverVulkan::_ensure_supported_sample_count(TextureSamples p_requested_sample_count) {
	VkSampleCountFlags sample_count_flags = (physical_device_properties.limits.framebufferColorSampleCounts & physical_device_properties.limits.framebufferDepthSampleCounts);

	if ((sample_count_flags & RD_TO_VK_SAMPLE_COUNT[p_requested_sample_count])) {
		// The requested sample count is supported.
		return RD_TO_VK_SAMPLE_COUNT[p_requested_sample_count];
	}
	else {
		// Find the closest lower supported sample count.
		VkSampleCountFlagBits sample_count = RD_TO_VK_SAMPLE_COUNT[p_requested_sample_count];
		while (sample_count > VK_SAMPLE_COUNT_1_BIT) {
			if (sample_count_flags & sample_count) {
				return sample_count;
			}
			sample_count = (VkSampleCountFlagBits)(sample_count >> 1);
		}
	}
	return VK_SAMPLE_COUNT_1_BIT;
}

BitField<RDD::TextureUsageBits> RenderingDeviceDriverVulkan::texture_get_usages_supported_by_format(DataFormat p_format, bool p_cpu_readable) {
	VkFormatProperties properties = {};
	vkGetPhysicalDeviceFormatProperties(physical_device, RD_TO_VK_FORMAT[p_format], &properties);

	const VkFormatFeatureFlags& flags = p_cpu_readable ? properties.linearTilingFeatures : properties.optimalTilingFeatures;

	// Everything supported by default makes an all-or-nothing check easier for the caller.
	BitField<RDD::TextureUsageBits> supported = INT64_MAX;

	if (!(flags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
		supported.clear_flag(TEXTURE_USAGE_SAMPLING_BIT);
	}
	if (!(flags & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
		supported.clear_flag(TEXTURE_USAGE_COLOR_ATTACHMENT_BIT);
	}
	if (!(flags & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
		supported.clear_flag(TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
	if (!(flags & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
		supported.clear_flag(TEXTURE_USAGE_STORAGE_BIT);
	}
	if (!(flags & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)) {
		supported.clear_flag(TEXTURE_USAGE_STORAGE_ATOMIC_BIT);
	}
	// Validation via VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR fails if VRS attachment is not supported.
	if (p_format != DATA_FORMAT_R8_UINT) {
		supported.clear_flag(TEXTURE_USAGE_VRS_ATTACHMENT_BIT);
	}

	return supported;
}
/// <summary>
/// VMA
/// </summary>
/// <param name="p_texture"></param>
/// <returns></returns>
uint64_t RenderingDeviceDriverVulkan::texture_get_allocation_size(TextureID p_texture) {
	const TextureInfo* tex_info = (const TextureInfo*)p_texture.id;
	return tex_info->allocation.info.size;
}

RDD::CommandQueueFamilyID RenderingDeviceDriverVulkan::command_queue_family_get(BitField<RDD::CommandQueueFamilyBits> p_cmd_queue_family_bits, RenderingContextDriver::SurfaceID p_surface) {
	// Pick the queue with the least amount of bits that can fulfill the requirements.
	VkQueueFlags picked_queue_flags = VK_QUEUE_FLAG_BITS_MAX_ENUM;
	uint32_t picked_family_index = UINT_MAX;
	for (uint32_t i = 0; i < queue_family_properties.size(); i++) {
		if (queue_families[i].is_empty()) {
			// Ignore empty queue families.
			continue;
		}

		if (p_surface != 0 && !context_driver->queue_family_supports_present(physical_device, i, p_surface)) {
			// Present is not an actual bit but something that must be queried manually.
			continue;
		}

		// Preferring a queue with less bits will get us closer to getting a queue that performs better for our requirements.
		// For example, dedicated compute and transfer queues are usually indicated as such.
		const VkQueueFlags option_queue_flags = queue_family_properties[i].queueFlags;
		const bool includes_all_bits = (option_queue_flags & p_cmd_queue_family_bits) == p_cmd_queue_family_bits;
		const bool prefer_less_bits = option_queue_flags < picked_queue_flags;
		if (includes_all_bits && prefer_less_bits) {
			picked_family_index = i;
			picked_queue_flags = option_queue_flags;
		}
	}

	ERR_FAIL_COND_V_MSG(picked_family_index >= queue_family_properties.size(), CommandQueueFamilyID(), "A queue family with the requested bits could not be found.");

	// Since 0 is a valid index and we use 0 as the error case, we make the index start from 1 instead.
	return CommandQueueFamilyID(picked_family_index + 1); // 
}

/// <summary>
/// extensions, features, capabilities, queue,
/// allocator, pipeline cache
/// 我觉得使用类变量会给开发者一个不太清楚的感觉
/// </summary>
/// <param name="p_device_index"></param>
/// <param name="p_frame_count"></param>
/// <returns></returns>
Error RenderingDeviceDriverVulkan::initialize(uint32_t p_device_index, uint32_t p_frame_count) {
	context_device = context_driver->device_get(p_device_index);
	physical_device = context_driver->physical_device_get(p_device_index);
	vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);

	frame_count = p_frame_count;

	// Copy the queue family properties the context already retrieved.
	uint32_t queue_family_count = context_driver->queue_family_get_count(p_device_index);
	queue_family_properties.resize(queue_family_count);
	for (uint32_t i = 0; i < queue_family_count; i++) {
		queue_family_properties[i] = context_driver->queue_family_get(p_device_index, i);
	}

	Error err = _initialize_device_extensions(); 
	ERR_FAIL_COND_V(err != OK, err);

	err = _check_device_features();
	ERR_FAIL_COND_V(err != OK, err);

	err = _check_device_capabilities();
	ERR_FAIL_COND_V(err != OK, err);

	LocalVector<VkDeviceQueueCreateInfo> queue_create_info;
	err = _add_queue_create_info(queue_create_info);
	ERR_FAIL_COND_V(err != OK, err);

	err = _initialize_device(queue_create_info);
	ERR_FAIL_COND_V(err != OK, err);

	err = _initialize_allocator();
	ERR_FAIL_COND_V(err != OK, err);

	err = _initialize_pipeline_cache();
	ERR_FAIL_COND_V(err != OK, err);

	max_descriptor_sets_per_pool = GLOBAL_GET("rendering/rendering_device/vulkan/max_descriptors_per_pool");

	return OK;
}

/// -- initialize
void RenderingDeviceDriverVulkan::_register_requested_device_extension(const CharString& p_extension_name, bool p_required) {
	ERR_FAIL_COND(requested_device_extensions.has(p_extension_name));
	requested_device_extensions[p_extension_name] = p_required;
}
/// <summary>
/// vkEnumerateDeviceExtensionProperties
/// </summary>
/// <returns></returns>
Error RenderingDeviceDriverVulkan::_initialize_device_extensions() {
	enabled_device_extension_names.clear();

	_register_requested_device_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, true);
	_register_requested_device_extension(VK_KHR_MULTIVIEW_EXTENSION_NAME, false);
	_register_requested_device_extension(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, false);
	_register_requested_device_extension(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME, false);
	_register_requested_device_extension(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME, false);
	_register_requested_device_extension(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME, false);
	_register_requested_device_extension(VK_KHR_16BIT_STORAGE_EXTENSION_NAME, false);
	_register_requested_device_extension(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME, false);
	_register_requested_device_extension(VK_KHR_MAINTENANCE_2_EXTENSION_NAME, false);
	_register_requested_device_extension(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME, false);
	_register_requested_device_extension(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME, false);
	_register_requested_device_extension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, true);

	uint32_t device_extension_count = 0;
	VkResult err = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, nullptr);
	ERR_FAIL_COND_V(err != VK_SUCCESS, ERR_CANT_CREATE);
	ERR_FAIL_COND_V_MSG(device_extension_count == 0, ERR_CANT_CREATE, "vkEnumerateDeviceExtensionProperties failed to find any extensions\n\nDo you have a compatible Vulkan installable client driver (ICD) installed?");

	TightLocalVector<VkExtensionProperties> device_extensions;
	device_extensions.resize(device_extension_count);
	err = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, device_extensions.ptr());
	ERR_FAIL_COND_V(err != VK_SUCCESS, ERR_CANT_CREATE);

#ifdef DEV_ENABLED
	for (uint32_t i = 0; i < device_extension_count; i++) {
		print_verbose(String("VULKAN: Found device extension ") + String::utf8(device_extensions[i].extensionName));
	}
#endif

}

/// <summary>
/// vkGetPhysicalDeviceFeatures
/// </summary>
/// <returns></returns>
Error RenderingDeviceDriverVulkan::_check_device_features() {
	vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);

	// Check for required features.
	if (!physical_device_features.imageCubeArray || !physical_device_features.independentBlend) {
		String error_string = vformat("Your GPU (%s) does not support the following features which are required to use Vulkan-based renderers in Godot:\n\n", context_device.name);
		if (!physical_device_features.imageCubeArray) {
			error_string += "- No support for image cube arrays.\n";
		}
		if (!physical_device_features.independentBlend) {
			error_string += "- No support for independentBlend.\n";
		}
		error_string += "\nThis is usually a hardware limitation, so updating graphics drivers won't help in most cases.";

//#if defined(ANDROID_ENABLED) || defined(IOS_ENABLED)
//		// Android/iOS platform ports currently don't exit themselves when this method returns `ERR_CANT_CREATE`.
//		OS::get_singleton()->alert(error_string + "\nClick OK to exit (black screen will be visible).");
//#else
//		OS::get_singleton()->alert(error_string + "\nClick OK to exit.");
//#endif

		return ERR_CANT_CREATE;
	}
	// 如果支持则启用
	// Opt-in to the features we actually need/use. These can be changed in the future.
	// We do this for multiple reasons:
	//
	//	1. Certain features (like sparse* stuff) cause unnecessary internal driver allocations.
	//	2. Others like shaderStorageImageMultisample are a huge red flag
	//	   (MSAA + Storage is rarely needed).
	//	3. Most features when turned off aren't actually off (we just promise the driver not to use them)
	//	   and it is validation what will complain. This allows us to target a minimum baseline.
	//
	// TODO: Allow the user to override these settings (i.e. turn off more stuff) using profiles
	// so they can target a broad range of HW. For example Mali HW does not have
	// shaderClipDistance/shaderCullDistance; thus validation would complain if such feature is used;
	// allowing them to fix the problem without even owning Mali HW to test on.
	//
	// The excluded features are:
	// - robustBufferAccess (can hamper performance on some hardware)
	// - occlusionQueryPrecise
	// - pipelineStatisticsQuery
	// - shaderStorageImageMultisample (unsupported by Intel Arc, prevents from using MSAA storage accidentally)
	// - shaderResourceResidency
	// - sparseBinding (we don't use sparse features and enabling them cause extra internal allocations inside the Vulkan driver we don't need)
	// - sparseResidencyBuffer
	// - sparseResidencyImage2D
	// - sparseResidencyImage3D
	// - sparseResidency2Samples
	// - sparseResidency4Samples
	// - sparseResidency8Samples
	// - sparseResidency16Samples
	// - sparseResidencyAliased
	// - inheritedQueries

#define VK_DEVICEFEATURE_ENABLE_IF(x)                             \
	if (physical_device_features.x) {                             \
		requested_device_features.x = physical_device_features.x; \
	} else                                                        \
		((void)0)

	requested_device_features = {};
	VK_DEVICEFEATURE_ENABLE_IF(fullDrawIndexUint32);
	VK_DEVICEFEATURE_ENABLE_IF(imageCubeArray);
	VK_DEVICEFEATURE_ENABLE_IF(independentBlend);
	VK_DEVICEFEATURE_ENABLE_IF(geometryShader);
	VK_DEVICEFEATURE_ENABLE_IF(tessellationShader);
	VK_DEVICEFEATURE_ENABLE_IF(sampleRateShading);
	VK_DEVICEFEATURE_ENABLE_IF(dualSrcBlend);
	VK_DEVICEFEATURE_ENABLE_IF(logicOp);
	VK_DEVICEFEATURE_ENABLE_IF(multiDrawIndirect);
	VK_DEVICEFEATURE_ENABLE_IF(drawIndirectFirstInstance);
	VK_DEVICEFEATURE_ENABLE_IF(depthClamp);
	VK_DEVICEFEATURE_ENABLE_IF(depthBiasClamp);
	VK_DEVICEFEATURE_ENABLE_IF(fillModeNonSolid);
	VK_DEVICEFEATURE_ENABLE_IF(depthBounds);
	VK_DEVICEFEATURE_ENABLE_IF(wideLines);
	VK_DEVICEFEATURE_ENABLE_IF(largePoints);
	VK_DEVICEFEATURE_ENABLE_IF(alphaToOne);
	VK_DEVICEFEATURE_ENABLE_IF(multiViewport);
	VK_DEVICEFEATURE_ENABLE_IF(samplerAnisotropy);
	VK_DEVICEFEATURE_ENABLE_IF(textureCompressionETC2);
	VK_DEVICEFEATURE_ENABLE_IF(textureCompressionASTC_LDR);
	VK_DEVICEFEATURE_ENABLE_IF(textureCompressionBC);
	VK_DEVICEFEATURE_ENABLE_IF(vertexPipelineStoresAndAtomics);
	VK_DEVICEFEATURE_ENABLE_IF(fragmentStoresAndAtomics);
	VK_DEVICEFEATURE_ENABLE_IF(shaderTessellationAndGeometryPointSize);
	VK_DEVICEFEATURE_ENABLE_IF(shaderImageGatherExtended);
	VK_DEVICEFEATURE_ENABLE_IF(shaderStorageImageExtendedFormats);
	VK_DEVICEFEATURE_ENABLE_IF(shaderStorageImageReadWithoutFormat);
	VK_DEVICEFEATURE_ENABLE_IF(shaderStorageImageWriteWithoutFormat);
	VK_DEVICEFEATURE_ENABLE_IF(shaderUniformBufferArrayDynamicIndexing);
	VK_DEVICEFEATURE_ENABLE_IF(shaderSampledImageArrayDynamicIndexing);
	VK_DEVICEFEATURE_ENABLE_IF(shaderStorageBufferArrayDynamicIndexing);
	VK_DEVICEFEATURE_ENABLE_IF(shaderStorageImageArrayDynamicIndexing);
	VK_DEVICEFEATURE_ENABLE_IF(shaderClipDistance);
	VK_DEVICEFEATURE_ENABLE_IF(shaderCullDistance);
	VK_DEVICEFEATURE_ENABLE_IF(shaderFloat64);
	VK_DEVICEFEATURE_ENABLE_IF(shaderInt64);
	VK_DEVICEFEATURE_ENABLE_IF(shaderInt16);
	VK_DEVICEFEATURE_ENABLE_IF(shaderResourceMinLod);
	VK_DEVICEFEATURE_ENABLE_IF(variableMultisampleRate);

	return OK;
}

Error RenderingDeviceDriverVulkan::_check_device_capabilities() {
	// Fill device family and version.
	device_capabilities.device_family = DEVICE_VULKAN;
	device_capabilities.version_major = VK_API_VERSION_MAJOR(physical_device_properties.apiVersion);
	device_capabilities.version_minor = VK_API_VERSION_MINOR(physical_device_properties.apiVersion);

	// References:
	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_KHR_multiview.html
	// https://www.khronos.org/blog/vulkan-subgroup-tutorial
	const RenderingContextDriverVulkan::Functions& functions = context_driver->functions_get();
	if (functions.GetPhysicalDeviceFeatures2 != nullptr) {
		// We must check that the corresponding extension is present before assuming a feature as enabled.
		// See also: https://github.com/godotengine/godot/issues/65409

		void* next_features = nullptr;
		VkPhysicalDeviceVulkan12Features device_features_vk_1_2 = {};
		VkPhysicalDeviceShaderFloat16Int8FeaturesKHR shader_features = {};
		VkPhysicalDeviceFragmentShadingRateFeaturesKHR vrs_features = {};
		VkPhysicalDevice16BitStorageFeaturesKHR storage_feature = {};
		VkPhysicalDeviceMultiviewFeatures multiview_features = {};
		VkPhysicalDevicePipelineCreationCacheControlFeatures pipeline_cache_control_features = {};

		const bool use_1_2_features = physical_device_properties.apiVersion >= VK_API_VERSION_1_2;
		if (use_1_2_features) {
			device_features_vk_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
			device_features_vk_1_2.pNext = next_features;
			next_features = &device_features_vk_1_2;
		}
		else if (enabled_device_extension_names.has(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME)) {
			shader_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR;
			shader_features.pNext = next_features;
			next_features = &shader_features;
		}

		if (enabled_device_extension_names.has(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)) {
			vrs_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
			vrs_features.pNext = next_features;
			next_features = &vrs_features;
		}

		if (enabled_device_extension_names.has(VK_KHR_16BIT_STORAGE_EXTENSION_NAME)) {
			storage_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;
			storage_feature.pNext = next_features;
			next_features = &storage_feature;
		}

		if (enabled_device_extension_names.has(VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
			multiview_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
			multiview_features.pNext = next_features;
			next_features = &multiview_features;
		}

		if (enabled_device_extension_names.has(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME)) {
			pipeline_cache_control_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES;
			pipeline_cache_control_features.pNext = next_features;
			next_features = &pipeline_cache_control_features;
		}

		VkPhysicalDeviceFeatures2 device_features_2 = {};
		device_features_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		device_features_2.pNext = next_features;
		functions.GetPhysicalDeviceFeatures2(physical_device, &device_features_2);

		if (use_1_2_features) {
#ifdef MACOS_ENABLED
			ERR_FAIL_COND_V_MSG(!device_features_vk_1_2.shaderSampledImageArrayNonUniformIndexing, ERR_CANT_CREATE, "Your GPU doesn't support shaderSampledImageArrayNonUniformIndexing which is required to use the Vulkan-based renderers in Godot.");
#endif
			if (enabled_device_extension_names.has(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME)) {
				shader_capabilities.shader_float16_is_supported = device_features_vk_1_2.shaderFloat16;
				shader_capabilities.shader_int8_is_supported = device_features_vk_1_2.shaderInt8;
			}
		}
		else {
			if (enabled_device_extension_names.has(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME)) {
				shader_capabilities.shader_float16_is_supported = shader_features.shaderFloat16;
				shader_capabilities.shader_int8_is_supported = shader_features.shaderInt8;
			}
		}

		if (enabled_device_extension_names.has(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)) {
			vrs_capabilities.pipeline_vrs_supported = vrs_features.pipelineFragmentShadingRate;
			vrs_capabilities.primitive_vrs_supported = vrs_features.primitiveFragmentShadingRate;
			vrs_capabilities.attachment_vrs_supported = vrs_features.attachmentFragmentShadingRate;
		}

		if (enabled_device_extension_names.has(VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
			multiview_capabilities.is_supported = multiview_features.multiview;
			multiview_capabilities.geometry_shader_is_supported = multiview_features.multiviewGeometryShader;
			multiview_capabilities.tessellation_shader_is_supported = multiview_features.multiviewTessellationShader;
		}

		if (enabled_device_extension_names.has(VK_KHR_16BIT_STORAGE_EXTENSION_NAME)) {
			storage_buffer_capabilities.storage_buffer_16_bit_access_is_supported = storage_feature.storageBuffer16BitAccess;
			storage_buffer_capabilities.uniform_and_storage_buffer_16_bit_access_is_supported = storage_feature.uniformAndStorageBuffer16BitAccess;
			storage_buffer_capabilities.storage_push_constant_16_is_supported = storage_feature.storagePushConstant16;
			storage_buffer_capabilities.storage_input_output_16 = storage_feature.storageInputOutput16;
		}

		if (enabled_device_extension_names.has(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME)) {
			pipeline_cache_control_capabilities.is_supported = pipeline_cache_control_features.pipelineCreationCacheControl;
		}
	}

	if (functions.GetPhysicalDeviceProperties2 != nullptr) {
		void* next_properties = nullptr;
		VkPhysicalDeviceFragmentShadingRatePropertiesKHR vrs_properties = {};
		VkPhysicalDeviceMultiviewProperties multiview_properties = {};
		VkPhysicalDeviceSubgroupProperties subgroup_properties = {};
		VkPhysicalDeviceSubgroupSizeControlProperties subgroup_size_control_properties = {};
		VkPhysicalDeviceProperties2 physical_device_properties_2 = {};

		const bool use_1_1_properties = physical_device_properties.apiVersion >= VK_API_VERSION_1_1;
		if (use_1_1_properties) {
			subgroup_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
			subgroup_properties.pNext = next_properties;
			next_properties = &subgroup_properties;

			subgroup_capabilities.size_control_is_supported = enabled_device_extension_names.has(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
			if (subgroup_capabilities.size_control_is_supported) {
				subgroup_size_control_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES;
				subgroup_size_control_properties.pNext = next_properties;
				next_properties = &subgroup_size_control_properties;
			}
		}

		if (multiview_capabilities.is_supported) {
			multiview_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES;
			multiview_properties.pNext = next_properties;
			next_properties = &multiview_properties;
		}

		if (vrs_capabilities.attachment_vrs_supported) {
			vrs_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
			vrs_properties.pNext = next_properties;
			next_properties = &vrs_properties;
		}

		physical_device_properties_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		physical_device_properties_2.pNext = next_properties;
		functions.GetPhysicalDeviceProperties2(physical_device, &physical_device_properties_2);

		subgroup_capabilities.size = subgroup_properties.subgroupSize;
		subgroup_capabilities.min_size = subgroup_properties.subgroupSize;
		subgroup_capabilities.max_size = subgroup_properties.subgroupSize;
		subgroup_capabilities.supported_stages = subgroup_properties.supportedStages;
		subgroup_capabilities.supported_operations = subgroup_properties.supportedOperations;

		// Note: quadOperationsInAllStages will be true if:
		// - supportedStages has VK_SHADER_STAGE_ALL_GRAPHICS + VK_SHADER_STAGE_COMPUTE_BIT.
		// - supportedOperations has VK_SUBGROUP_FEATURE_QUAD_BIT.
		subgroup_capabilities.quad_operations_in_all_stages = subgroup_properties.quadOperationsInAllStages;

		if (subgroup_capabilities.size_control_is_supported && (subgroup_size_control_properties.requiredSubgroupSizeStages & VK_SHADER_STAGE_COMPUTE_BIT)) {
			subgroup_capabilities.min_size = subgroup_size_control_properties.minSubgroupSize;
			subgroup_capabilities.max_size = subgroup_size_control_properties.maxSubgroupSize;
		}

		if (vrs_capabilities.pipeline_vrs_supported || vrs_capabilities.primitive_vrs_supported || vrs_capabilities.attachment_vrs_supported) {
			print_verbose("- Vulkan Variable Rate Shading supported:");
			if (vrs_capabilities.pipeline_vrs_supported) {
				print_verbose("  Pipeline fragment shading rate");
			}
			if (vrs_capabilities.primitive_vrs_supported) {
				print_verbose("  Primitive fragment shading rate");
			}
			if (vrs_capabilities.attachment_vrs_supported) {
				// TODO: Expose these somehow to the end user.
				vrs_capabilities.min_texel_size.x = vrs_properties.minFragmentShadingRateAttachmentTexelSize.width;
				vrs_capabilities.min_texel_size.y = vrs_properties.minFragmentShadingRateAttachmentTexelSize.height;
				vrs_capabilities.max_texel_size.x = vrs_properties.maxFragmentShadingRateAttachmentTexelSize.width;
				vrs_capabilities.max_texel_size.y = vrs_properties.maxFragmentShadingRateAttachmentTexelSize.height;

				// We'll attempt to default to a texel size of 16x16.
				vrs_capabilities.texel_size = Vector2i(16, 16).clamp(vrs_capabilities.min_texel_size, vrs_capabilities.max_texel_size);

				print_verbose(String("  Attachment fragment shading rate") + String(", min texel size: (") + itos(vrs_capabilities.min_texel_size.x) + String(", ") + itos(vrs_capabilities.min_texel_size.y) + String(")") + String(", max texel size: (") + itos(vrs_capabilities.max_texel_size.x) + String(", ") + itos(vrs_capabilities.max_texel_size.y) + String(")"));
			}

		}
		else {
			print_verbose("- Vulkan Variable Rate Shading not supported");
		}

		if (multiview_capabilities.is_supported) {
			multiview_capabilities.max_view_count = multiview_properties.maxMultiviewViewCount;
			multiview_capabilities.max_instance_count = multiview_properties.maxMultiviewInstanceIndex;

			print_verbose("- Vulkan multiview supported:");
			print_verbose("  max view count: " + itos(multiview_capabilities.max_view_count));
			print_verbose("  max instances: " + itos(multiview_capabilities.max_instance_count));
		}
		else {
			print_verbose("- Vulkan multiview not supported");
		}

		print_verbose("- Vulkan subgroup:");
		print_verbose("  size: " + itos(subgroup_capabilities.size));
		print_verbose("  min size: " + itos(subgroup_capabilities.min_size));
		print_verbose("  max size: " + itos(subgroup_capabilities.max_size));
		print_verbose("  stages: " + subgroup_capabilities.supported_stages_desc());
		print_verbose("  supported ops: " + subgroup_capabilities.supported_operations_desc());
		if (subgroup_capabilities.quad_operations_in_all_stages) {
			print_verbose("  quad operations in all stages");
		}
	}

	return OK;
}

Error RenderingDeviceDriverVulkan::_add_queue_create_info(LocalVector<VkDeviceQueueCreateInfo>& r_queue_create_info) {
	uint32_t queue_family_count = queue_family_properties.size();
	queue_families.resize(queue_family_count);

	VkQueueFlags queue_flags_mask = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT;
	const uint32_t max_queue_count_per_family = 1;
	static const float queue_priorities[max_queue_count_per_family] = {};
	for (uint32_t i = 0; i < queue_family_count; i++) {
		if ((queue_family_properties[i].queueFlags & queue_flags_mask) == 0) {
			// We ignore creating queues in families that don't support any of the operations we require.
			continue;
		}

		VkDeviceQueueCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		create_info.queueFamilyIndex = i;
		create_info.queueCount = MIN(queue_family_properties[i].queueCount, max_queue_count_per_family);
		create_info.pQueuePriorities = queue_priorities;
		r_queue_create_info.push_back(create_info);

		// Prepare the vectors where the queues will be filled out.
		queue_families[i].resize(create_info.queueCount);
	}

	return OK;
}

Error RenderingDeviceDriverVulkan::_initialize_device(const LocalVector<VkDeviceQueueCreateInfo>& p_queue_create_info) {
	TightLocalVector<const char*> enabled_extension_names;
	enabled_extension_names.reserve(enabled_device_extension_names.size());
	for (const CharString& extension_name : enabled_device_extension_names) {
		enabled_extension_names.push_back(extension_name.ptr());
	}

	void* create_info_next = nullptr;
	VkPhysicalDeviceShaderFloat16Int8FeaturesKHR shader_features = {};
	shader_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR;
	shader_features.pNext = create_info_next;
	shader_features.shaderFloat16 = shader_capabilities.shader_float16_is_supported;
	shader_features.shaderInt8 = shader_capabilities.shader_int8_is_supported;
	create_info_next = &shader_features;

	VkPhysicalDeviceFragmentShadingRateFeaturesKHR vrs_features = {};
	if (vrs_capabilities.pipeline_vrs_supported || vrs_capabilities.primitive_vrs_supported || vrs_capabilities.attachment_vrs_supported) {
		vrs_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
		vrs_features.pNext = create_info_next;
		vrs_features.pipelineFragmentShadingRate = vrs_capabilities.pipeline_vrs_supported;
		vrs_features.primitiveFragmentShadingRate = vrs_capabilities.primitive_vrs_supported;
		vrs_features.attachmentFragmentShadingRate = vrs_capabilities.attachment_vrs_supported;
		create_info_next = &vrs_features;
	}

	VkPhysicalDevicePipelineCreationCacheControlFeatures pipeline_cache_control_features = {};
	if (pipeline_cache_control_capabilities.is_supported) {
		pipeline_cache_control_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES;
		pipeline_cache_control_features.pNext = create_info_next;
		pipeline_cache_control_features.pipelineCreationCacheControl = true;
		create_info_next = &pipeline_cache_control_features;
	}

	VkPhysicalDeviceVulkan11Features vulkan_1_1_features = {};
	VkPhysicalDevice16BitStorageFeaturesKHR storage_features = {};
	VkPhysicalDeviceMultiviewFeatures multiview_features = {};
	const bool enable_1_2_features = physical_device_properties.apiVersion >= VK_API_VERSION_1_2;
	if (enable_1_2_features) {
		// In Vulkan 1.2 and newer we use a newer struct to enable various features.
		vulkan_1_1_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
		vulkan_1_1_features.pNext = create_info_next;
		vulkan_1_1_features.storageBuffer16BitAccess = storage_buffer_capabilities.storage_buffer_16_bit_access_is_supported;
		vulkan_1_1_features.uniformAndStorageBuffer16BitAccess = storage_buffer_capabilities.uniform_and_storage_buffer_16_bit_access_is_supported;
		vulkan_1_1_features.storagePushConstant16 = storage_buffer_capabilities.storage_push_constant_16_is_supported;
		vulkan_1_1_features.storageInputOutput16 = storage_buffer_capabilities.storage_input_output_16;
		vulkan_1_1_features.multiview = multiview_capabilities.is_supported;
		vulkan_1_1_features.multiviewGeometryShader = multiview_capabilities.geometry_shader_is_supported;
		vulkan_1_1_features.multiviewTessellationShader = multiview_capabilities.tessellation_shader_is_supported;
		vulkan_1_1_features.variablePointersStorageBuffer = 0;
		vulkan_1_1_features.variablePointers = 0;
		vulkan_1_1_features.protectedMemory = 0;
		vulkan_1_1_features.samplerYcbcrConversion = 0;
		vulkan_1_1_features.shaderDrawParameters = 0;
		create_info_next = &vulkan_1_1_features;
	}
	else {
		// On Vulkan 1.0 and 1.1 we use our older structs to initialize these features.
		storage_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;
		storage_features.pNext = create_info_next;
		storage_features.storageBuffer16BitAccess = storage_buffer_capabilities.storage_buffer_16_bit_access_is_supported;
		storage_features.uniformAndStorageBuffer16BitAccess = storage_buffer_capabilities.uniform_and_storage_buffer_16_bit_access_is_supported;
		storage_features.storagePushConstant16 = storage_buffer_capabilities.storage_push_constant_16_is_supported;
		storage_features.storageInputOutput16 = storage_buffer_capabilities.storage_input_output_16;
		create_info_next = &storage_features;

		const bool enable_1_1_features = physical_device_properties.apiVersion >= VK_API_VERSION_1_1;
		if (enable_1_1_features) {
			multiview_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
			multiview_features.pNext = create_info_next;
			multiview_features.multiview = multiview_capabilities.is_supported;
			multiview_features.multiviewGeometryShader = multiview_capabilities.geometry_shader_is_supported;
			multiview_features.multiviewTessellationShader = multiview_capabilities.tessellation_shader_is_supported;
			create_info_next = &multiview_features;
		}
	}

	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pNext = create_info_next;
	create_info.queueCreateInfoCount = p_queue_create_info.size();
	create_info.pQueueCreateInfos = p_queue_create_info.ptr();
	create_info.enabledExtensionCount = enabled_extension_names.size();
	create_info.ppEnabledExtensionNames = enabled_extension_names.ptr();
	create_info.pEnabledFeatures = &requested_device_features;

	VkResult err = vkCreateDevice(physical_device, &create_info, nullptr, &vk_device);
	ERR_FAIL_COND_V(err != VK_SUCCESS, ERR_CANT_CREATE);

	for (uint32_t i = 0; i < queue_families.size(); i++) {
		for (uint32_t j = 0; j < queue_families[i].size(); j++) {
			vkGetDeviceQueue(vk_device, i, j, &queue_families[i][j].queue);
		}
	}

	for (size_t i = 0; i < queue_family_properties.size(); queue_family_properties) {
		auto index = CommandQueueFamilyID(i + 1);
		queueid_to_family[index] = (CommandQueueFamilyBits) queue_family_properties[i].queueFlags;
		static const auto _add_ids =  [&](CommandQueueFamilyBits bit) {
			if ((CommandQueueFamilyBits)queue_family_properties[i].queueFlags & bit)
				queuefamily_to_ids[bit].push_back(index);
			};
		_add_ids(COMMAND_QUEUE_FAMILY_GRAPHICS_BIT);
		_add_ids(COMMAND_QUEUE_FAMILY_COMPUTE_BIT);
		_add_ids(COMMAND_QUEUE_FAMILY_TRANSFER_BIT);
	}
	

	const RenderingContextDriverVulkan::Functions& functions = context_driver->functions_get();
	// enable getdeviceprocaddr extension
	if (functions.GetDeviceProcAddr != nullptr) {
		device_functions.CreateSwapchainKHR = PFN_vkCreateSwapchainKHR(functions.GetDeviceProcAddr(vk_device, "vkCreateSwapchainKHR"));
		device_functions.DestroySwapchainKHR = PFN_vkDestroySwapchainKHR(functions.GetDeviceProcAddr(vk_device, "vkDestroySwapchainKHR"));
		device_functions.GetSwapchainImagesKHR = PFN_vkGetSwapchainImagesKHR(functions.GetDeviceProcAddr(vk_device, "vkGetSwapchainImagesKHR"));
		device_functions.AcquireNextImageKHR = PFN_vkAcquireNextImageKHR(functions.GetDeviceProcAddr(vk_device, "vkAcquireNextImageKHR"));
		device_functions.QueuePresentKHR = PFN_vkQueuePresentKHR(functions.GetDeviceProcAddr(vk_device, "vkQueuePresentKHR"));

		if (enabled_device_extension_names.has(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME)) {
			device_functions.CreateRenderPass2KHR = PFN_vkCreateRenderPass2KHR(functions.GetDeviceProcAddr(vk_device, "vkCreateRenderPass2KHR"));
		}
	}

	return OK;
}

Error RenderingDeviceDriverVulkan::_initialize_allocator() {
	VmaAllocatorCreateInfo allocator_info = {};
	allocator_info.physicalDevice = physical_device;
	allocator_info.device = vk_device;
	allocator_info.instance = context_driver->instance_get();
	VkResult err = vmaCreateAllocator(&allocator_info, &allocator);
	ERR_FAIL_COND_V_MSG(err, ERR_CANT_CREATE, "vmaCreateAllocator failed with error " + itos(err) + ".");

	return OK;
}
/// <summary>
/// pipeline cache header
/// </summary>
/// <returns></returns>
Error RenderingDeviceDriverVulkan::_initialize_pipeline_cache() {
	pipelines_cache.buffer.resize(sizeof(PipelineCacheHeader));
	PipelineCacheHeader* header = (PipelineCacheHeader*)(pipelines_cache.buffer.ptrw());
	*header = {};
	header->magic = 868 + VK_PIPELINE_CACHE_HEADER_VERSION_ONE;
	header->device_id = physical_device_properties.deviceID;
	header->vendor_id = physical_device_properties.vendorID;
	header->driver_version = physical_device_properties.driverVersion;
	memcpy(header->uuid, physical_device_properties.pipelineCacheUUID, VK_UUID_SIZE);
	header->driver_abi = sizeof(void*);

	pipeline_cache_id = String::hex_encode_buffer(physical_device_properties.pipelineCacheUUID, VK_UUID_SIZE);
	pipeline_cache_id += "-driver-" + itos(physical_device_properties.driverVersion);

	return OK;
	}

void RenderingDeviceDriverVulkan::set_object_name(ObjectType p_type, ID p_driver_id, const String& p_name) {
	switch (p_type) {
	case OBJECT_TYPE_TEXTURE: {
		const TextureInfo* tex_info = (const TextureInfo*)p_driver_id.id;
		if (tex_info->allocation.handle) {
			_set_object_name(VK_OBJECT_TYPE_IMAGE, (uint64_t)tex_info->vk_view_create_info.image, p_name);
		}
		_set_object_name(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)tex_info->vk_view, p_name + " View");
	} break;
	case OBJECT_TYPE_SAMPLER: {
		_set_object_name(VK_OBJECT_TYPE_SAMPLER, p_driver_id.id, p_name);
	} break;
	/*case OBJECT_TYPE_BUFFER: {
		const BufferInfo* buf_info = (const BufferInfo*)p_driver_id.id;
		_set_object_name(VK_OBJECT_TYPE_BUFFER, (uint64_t)buf_info->vk_buffer, p_name);
		if (buf_info->vk_view) {
			_set_object_name(VK_OBJECT_TYPE_BUFFER_VIEW, (uint64_t)buf_info->vk_view, p_name + " View");
		}
	} break;
	case OBJECT_TYPE_SHADER: {
		const ShaderInfo* shader_info = (const ShaderInfo*)p_driver_id.id;
		for (uint32_t i = 0; i < shader_info->vk_descriptor_set_layouts.size(); i++) {
			_set_object_name(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)shader_info->vk_descriptor_set_layouts[i], p_name);
		}
		_set_object_name(VK_OBJECT_TYPE_PIPELINE_LAYOUT, (uint64_t)shader_info->vk_pipeline_layout, p_name + " Pipeline Layout");
	} break;
	case OBJECT_TYPE_UNIFORM_SET: {
		const UniformSetInfo* usi = (const UniformSetInfo*)p_driver_id.id;
		_set_object_name(VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)usi->vk_descriptor_set, p_name);
	} break;
	case OBJECT_TYPE_PIPELINE: {
		_set_object_name(VK_OBJECT_TYPE_PIPELINE, (uint64_t)p_driver_id.id, p_name);
	} break;*/
	default: {
		DEV_ASSERT(false);
	}
	}
}


void RenderingDeviceDriverVulkan::_set_object_name(VkObjectType p_object_type, uint64_t p_object_handle, String p_object_name) {
	const RenderingContextDriverVulkan::Functions& functions = context_driver->functions_get();
	if (functions.SetDebugUtilsObjectNameEXT != nullptr) {
		CharString obj_data = p_object_name.utf8();
		VkDebugUtilsObjectNameInfoEXT name_info;
		name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		name_info.pNext = nullptr;
		name_info.objectType = p_object_type;
		name_info.objectHandle = p_object_handle;
		name_info.pObjectName = obj_data.get_data();
		functions.SetDebugUtilsObjectNameEXT(vk_device, &name_info);
	}
}
	RenderingDeviceDriverVulkan::RenderingDeviceDriverVulkan(RenderingContextDriverVulkan* p_driver) {
		context_driver = p_driver;
	}

	RenderingDeviceDriverVulkan::~RenderingDeviceDriverVulkan() {
		while (small_allocs_pools.size()) {
			HashMap<uint32_t, VmaPool>::Iterator E = small_allocs_pools.begin();
			vmaDestroyPool(allocator, E->value);
			small_allocs_pools.remove(E);
		}
		vmaDestroyAllocator(allocator);

		if (vk_device != VK_NULL_HANDLE) {
			vkDestroyDevice(vk_device, nullptr);
		}
	}

	/// --- SubgroupCapabilities
	uint32_t RenderingDeviceDriverVulkan::SubgroupCapabilities::supported_stages_flags_rd() const {
		uint32_t flags = 0;

		if (supported_stages & VK_SHADER_STAGE_VERTEX_BIT) {
			flags += SHADER_STAGE_VERTEX_BIT;
		}
		if (supported_stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
			flags += SHADER_STAGE_TESSELATION_CONTROL_BIT;
		}
		if (supported_stages & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
			flags += SHADER_STAGE_TESSELATION_EVALUATION_BIT;
		}
		if (supported_stages & VK_SHADER_STAGE_GEOMETRY_BIT) {
			// FIXME: Add shader stage geometry bit.
		}
		if (supported_stages & VK_SHADER_STAGE_FRAGMENT_BIT) {
			flags += SHADER_STAGE_FRAGMENT_BIT;
		}
		if (supported_stages & VK_SHADER_STAGE_COMPUTE_BIT) {
			flags += SHADER_STAGE_COMPUTE_BIT;
		}

		return flags;
	}

	String RenderingDeviceDriverVulkan::SubgroupCapabilities::supported_stages_desc() const {
		String res;

		if (supported_stages & VK_SHADER_STAGE_VERTEX_BIT) {
			res += ", STAGE_VERTEX";
		}
		if (supported_stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
			res += ", STAGE_TESSELLATION_CONTROL";
		}
		if (supported_stages & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
			res += ", STAGE_TESSELLATION_EVALUATION";
		}
		if (supported_stages & VK_SHADER_STAGE_GEOMETRY_BIT) {
			res += ", STAGE_GEOMETRY";
		}
		if (supported_stages & VK_SHADER_STAGE_FRAGMENT_BIT) {
			res += ", STAGE_FRAGMENT";
		}
		if (supported_stages & VK_SHADER_STAGE_COMPUTE_BIT) {
			res += ", STAGE_COMPUTE";
		}

		// These are not defined on Android GRMBL.
		if (supported_stages & 0x00000100 /* VK_SHADER_STAGE_RAYGEN_BIT_KHR */) {
			res += ", STAGE_RAYGEN_KHR";
		}
		if (supported_stages & 0x00000200 /* VK_SHADER_STAGE_ANY_HIT_BIT_KHR */) {
			res += ", STAGE_ANY_HIT_KHR";
		}
		if (supported_stages & 0x00000400 /* VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR */) {
			res += ", STAGE_CLOSEST_HIT_KHR";
		}
		if (supported_stages & 0x00000800 /* VK_SHADER_STAGE_MISS_BIT_KHR */) {
			res += ", STAGE_MISS_KHR";
		}
		if (supported_stages & 0x00001000 /* VK_SHADER_STAGE_INTERSECTION_BIT_KHR */) {
			res += ", STAGE_INTERSECTION_KHR";
		}
		if (supported_stages & 0x00002000 /* VK_SHADER_STAGE_CALLABLE_BIT_KHR */) {
			res += ", STAGE_CALLABLE_KHR";
		}
		if (supported_stages & 0x00000040 /* VK_SHADER_STAGE_TASK_BIT_NV */) {
			res += ", STAGE_TASK_NV";
		}
		if (supported_stages & 0x00000080 /* VK_SHADER_STAGE_MESH_BIT_NV */) {
			res += ", STAGE_MESH_NV";
		}

		return res.substr(2); // Remove first ", ".
	}

	uint32_t RenderingDeviceDriverVulkan::SubgroupCapabilities::supported_operations_flags_rd() const {
		uint32_t flags = 0;

		if (supported_operations & VK_SUBGROUP_FEATURE_BASIC_BIT) {
			flags += SUBGROUP_BASIC_BIT;
		}
		if (supported_operations & VK_SUBGROUP_FEATURE_VOTE_BIT) {
			flags += SUBGROUP_VOTE_BIT;
		}
		if (supported_operations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) {
			flags += SUBGROUP_ARITHMETIC_BIT;
		}
		if (supported_operations & VK_SUBGROUP_FEATURE_BALLOT_BIT) {
			flags += SUBGROUP_BALLOT_BIT;
		}
		if (supported_operations & VK_SUBGROUP_FEATURE_SHUFFLE_BIT) {
			flags += SUBGROUP_SHUFFLE_BIT;
		}
		if (supported_operations & VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT) {
			flags += SUBGROUP_SHUFFLE_RELATIVE_BIT;
		}
		if (supported_operations & VK_SUBGROUP_FEATURE_CLUSTERED_BIT) {
			flags += SUBGROUP_CLUSTERED_BIT;
		}
		if (supported_operations & VK_SUBGROUP_FEATURE_QUAD_BIT) {
			flags += SUBGROUP_QUAD_BIT;
		}

		return flags;
	}

	String RenderingDeviceDriverVulkan::SubgroupCapabilities::supported_operations_desc() const {
		String res;

		if (supported_operations & VK_SUBGROUP_FEATURE_BASIC_BIT) {
			res += ", FEATURE_BASIC";
		}
		if (supported_operations & VK_SUBGROUP_FEATURE_VOTE_BIT) {
			res += ", FEATURE_VOTE";
		}
		if (supported_operations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) {
			res += ", FEATURE_ARITHMETIC";
		}
		if (supported_operations & VK_SUBGROUP_FEATURE_BALLOT_BIT) {
			res += ", FEATURE_BALLOT";
		}
		if (supported_operations & VK_SUBGROUP_FEATURE_SHUFFLE_BIT) {
			res += ", FEATURE_SHUFFLE";
		}
		if (supported_operations & VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT) {
			res += ", FEATURE_SHUFFLE_RELATIVE";
		}
		if (supported_operations & VK_SUBGROUP_FEATURE_CLUSTERED_BIT) {
			res += ", FEATURE_CLUSTERED";
		}
		if (supported_operations & VK_SUBGROUP_FEATURE_QUAD_BIT) {
			res += ", FEATURE_QUAD";
		}
		if (supported_operations & VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV) {
			res += ", FEATURE_PARTITIONED_NV";
		}

		return res.substr(2); // Remove first ", ".
	}



	/****************/
	/**** MEMORY ****/
	/****************/

	VmaPool RenderingDeviceDriverVulkan::_find_or_create_small_allocs_pool(uint32_t p_mem_type_index) {
		if (small_allocs_pools.has(p_mem_type_index)) {
			return small_allocs_pools[p_mem_type_index];
		}

		print_verbose("Creating VMA small objects pool for memory type index " + itos(p_mem_type_index));

		VmaPoolCreateInfo pci = {};
		pci.memoryTypeIndex = p_mem_type_index;
		pci.flags = 0;
		pci.blockSize = 0;
		pci.minBlockCount = 0;
		pci.maxBlockCount = SIZE_MAX;
		pci.priority = 0.5f;
		pci.minAllocationAlignment = 0;
		pci.pMemoryAllocateNext = nullptr;
		VmaPool pool = VK_NULL_HANDLE;
		VkResult res = vmaCreatePool(allocator, &pci, &pool);
		small_allocs_pools[p_mem_type_index] = pool; // Don't try to create it again if failed the first time.
		ERR_FAIL_COND_V_MSG(res, pool, "vmaCreatePool failed with error " + itos(res) + ".");

		return pool;
	}
} // namespace graphics

}// namespace lain