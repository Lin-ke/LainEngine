#include "rendering_device_driver_vulkan.h"
#include "core/config/project_settings.h"
#include "core/string/print_string.h"
#define PRINT_NATIVE_COMMANDS 1
/*****************/
/**** GENERIC ****/
/*****************/
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

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
				//add_unique_family(sharing_indices, create_info.queueFamilyIndexCount, queuefamily_to_ids[m.index] ); // @TODO 多队列共用
				//
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
	
	// Create view.
	VkImageViewCreateInfo image_view_create_info = {};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.image = vk_image;
	image_view_create_info.viewType = (VkImageViewType)p_format.texture_type;
	image_view_create_info.format = RD_TO_VK_FORMAT[p_view.format];
	image_view_create_info.components.r = (VkComponentSwizzle)p_view.swizzle_r;
	image_view_create_info.components.g = (VkComponentSwizzle)p_view.swizzle_g;
	image_view_create_info.components.b = (VkComponentSwizzle)p_view.swizzle_b;
	image_view_create_info.components.a = (VkComponentSwizzle)p_view.swizzle_a;
	image_view_create_info.subresourceRange.levelCount = create_info.mipLevels;
	image_view_create_info.subresourceRange.layerCount = create_info.arrayLayers;
	if ((p_format.usage_bits & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else {
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkImageView vk_image_view = VK_NULL_HANDLE;
	err = vkCreateImageView(vk_device, &image_view_create_info, nullptr, &vk_image_view);
	if (err) {
		vmaDestroyImage(allocator, vk_image, allocation);
		ERR_FAIL_COND_V_MSG(err, TextureID(), "vkCreateImageView failed with error " + itos(err) + ".");
	}

	// Bookkeep.

	TextureInfo* tex_info = VersatileResource::allocate<TextureInfo>(resources_allocator);
	tex_info->vk_view = vk_image_view;
	tex_info->rd_format = p_format.format;
	tex_info->vk_create_info = create_info;
	tex_info->vk_view_create_info = image_view_create_info;
	tex_info->allocation.handle = allocation;
	vmaGetAllocationInfo(allocator, tex_info->allocation.handle, &tex_info->allocation.info);

#if PRINT_NATIVE_COMMANDS
	print_line(vformat("vkCreateImageView: 0x%uX for 0x%uX", uint64_t(vk_image_view), uint64_t(vk_image)));
#endif

	return TextureID(tex_info);
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
/// <summary>
/// Command QUEUE
/// </summary>
/// <param name="p_cmd_queue_family_bits"></param>
/// <param name="p_surface"></param>
/// <returns></returns>
RDD::CommandQueueFamilyID RenderingDeviceDriverVulkan::command_queue_family_get(BitField<RDD::CommandQueueFamilyBits> p_cmd_queue_family_bits, RenderingContextDriver::SurfaceID p_surface) {
	// Pick the queue with the least amount of bits that can fulfill the requirements.
	VkQueueFlags picked_queue_flags = VK_QUEUE_FLAG_BITS_MAX_ENUM;
	uint32_t picked_family_index = UINT_MAX;
	for (uint32_t i = 0; i < queue_family_properties.size(); i++) {
		if (queue_families[i].is_empty()) {
			// Ignore empty queue families.
			continue;
		}

		// -- if p_surface == 0, pass this
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

// ----- QUEUE -----

RDD::CommandQueueID RenderingDeviceDriverVulkan::command_queue_create(CommandQueueFamilyID p_cmd_queue_family, bool p_identify_as_main_queue) {
	DEV_ASSERT(p_cmd_queue_family.id != 0);

	// Make a virtual queue on top of a real queue. Use the queue from the family with the least amount of virtual queues created.
	uint32_t family_index = p_cmd_queue_family.id - 1;
	TightLocalVector<Queue>& queue_family = queue_families[family_index];
	uint32_t picked_queue_index = UINT_MAX;
	uint32_t picked_virtual_count = UINT_MAX;
	print_verbose("queue_family count", queue_family.size()); // 实际上只有一条队列,max_queue_count_per_family
	for (uint32_t i = 0; i < queue_family.size(); i++) {
		if (queue_family[i].virtual_count < picked_virtual_count) {
			picked_queue_index = i;
			picked_virtual_count = queue_family[i].virtual_count;
		}
	}

	ERR_FAIL_COND_V_MSG(picked_queue_index >= queue_family.size(), CommandQueueID(), "A queue in the picked family could not be found.");

	// Create the virtual queue.
	CommandQueue* command_queue = memnew(CommandQueue);
	command_queue->queue_family = family_index;
	command_queue->queue_index = picked_queue_index;
	queue_family[picked_queue_index].virtual_count++;

	//// If is was identified as the main queue and a hook is active, indicate it as such to the hook.
	//if (p_identify_as_main_queue && (VulkanHooks::get_singleton() != nullptr)) {
	//	VulkanHooks::get_singleton()->set_direct_queue_family_and_index(family_index, picked_queue_index);
	//}

	return CommandQueueID(command_queue);
}

/// <summary>
/// execute和present
/// </summary>
/// <param name="p_cmd_queue"></param>
/// <param name="p_wait_semaphores"></param>
/// <param name="p_cmd_buffers"></param>
/// <param name="p_cmd_semaphores"></param>
/// <param name="p_cmd_fence"></param>
/// <param name="p_swap_chains"></param>
/// <returns></returns>
Error RenderingDeviceDriverVulkan::command_queue_execute_and_present(CommandQueueID p_cmd_queue, VectorView<SemaphoreID> p_wait_semaphores, VectorView<CommandBufferID> p_cmd_buffers, VectorView<SemaphoreID> p_cmd_semaphores, FenceID p_cmd_fence, VectorView<SwapChainID> p_swap_chains) {
	
	VkResult err;
	CommandQueue* command_queue = (CommandQueue*)(p_cmd_queue.id);
	Queue& device_queue = queue_families[command_queue->queue_family][command_queue->queue_index];

	Fence* fence = (Fence*)(p_cmd_fence.id);
	VkFence vk_fence = (fence != nullptr) ? fence->vk_fence : VK_NULL_HANDLE;
	// 需要waiting：
	// pending for execute;
	// 传入的p_wait_semaphores
	// 也许应该分开execute与present@TODO
	// 需要signal：
	// present
	// 传入的cmd semaphore

	thread_local LocalVector<VkSemaphore> wait_semaphores; // 为什么这个是thread local的？@？
	thread_local LocalVector<VkPipelineStageFlags> wait_semaphores_stages;
	if (!command_queue->pending_semaphores_for_execute.is_empty()) {
		for (uint32_t i = 0; i < command_queue->pending_semaphores_for_execute.size(); i++) {
			VkSemaphore wait_semaphore = command_queue->image_semaphores[command_queue->pending_semaphores_for_execute[i]];
			wait_semaphores.push_back(wait_semaphore);
			wait_semaphores_stages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		}

		command_queue->pending_semaphores_for_execute.clear(); // 这里不是加到free吗@？
	}
	for (uint32_t i = 0; i < p_wait_semaphores.size(); i++) {
		// FIXME: Allow specifying the stage mask in more detail.
		wait_semaphores.push_back(VkSemaphore(p_wait_semaphores[i].id));
		wait_semaphores_stages.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	}
	if (p_cmd_buffers.size() > 0) {
		thread_local LocalVector<VkCommandBuffer> command_buffers;
		thread_local LocalVector<VkSemaphore> signal_semaphores;
		command_buffers.clear();
		signal_semaphores.clear(); // submit需要的信号量
		for (uint32_t i = 0; i < p_cmd_buffers.size(); i++) {
			command_buffers.push_back(VkCommandBuffer(p_cmd_buffers[i].id)); // id直接到command buffer
		}

		for (uint32_t i = 0; i < p_cmd_semaphores.size(); i++) {
			signal_semaphores.push_back(VkSemaphore(p_cmd_semaphores[i].id));
		}

		VkSemaphore present_semaphore = VK_NULL_HANDLE;
		if (p_swap_chains.size() > 0) {
			// present semaphore可空
			if (command_queue->present_semaphores.is_empty()) {
				// Create the semaphores used for presentation if they haven't been created yet.
				VkSemaphore semaphore = VK_NULL_HANDLE;
				VkSemaphoreCreateInfo create_info = {};
				create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

				for (uint32_t i = 0; i < frame_count; i++) {
					err = vkCreateSemaphore(vk_device, &create_info, nullptr, &semaphore);
					ERR_FAIL_COND_V(err != VK_SUCCESS, FAILED);
					command_queue->present_semaphores.push_back(semaphore);
				}
			}

			// If a presentation semaphore is required, cycle across the ones available on the queue. It is technically possible
			// and valid to reuse the same semaphore for this particular operation, but we create multiple ones anyway in case
			// some hardware expects multiple semaphores to be used.
			present_semaphore = command_queue->present_semaphores[command_queue->present_semaphore_index];
			signal_semaphores.push_back(present_semaphore);
			command_queue->present_semaphore_index = (command_queue->present_semaphore_index + 1) % command_queue->present_semaphores.size();
		}

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount = wait_semaphores.size();
		submit_info.pWaitSemaphores = wait_semaphores.ptr();
		submit_info.pWaitDstStageMask = wait_semaphores_stages.ptr();
		submit_info.commandBufferCount = command_buffers.size();
		submit_info.pCommandBuffers = command_buffers.ptr();
		submit_info.signalSemaphoreCount = signal_semaphores.size();
		submit_info.pSignalSemaphores = signal_semaphores.ptr();

		device_queue.submit_mutex.lock();
		err = vkQueueSubmit(device_queue.queue, 1, &submit_info, vk_fence); // 传入的fence
		device_queue.submit_mutex.unlock();
		ERR_FAIL_COND_V(err != VK_SUCCESS, FAILED);

		// 有fence且command-queue有出发semaphore的
		if (fence != nullptr && !command_queue->pending_semaphores_for_fence.is_empty()) {
			fence->queue_signaled_from = command_queue;

			// Indicate to the fence that it should release the semaphores that were waited on this submission the next time the fence is waited on.
			for (uint32_t i = 0; i < command_queue->pending_semaphores_for_fence.size(); i++) {
				command_queue->image_semaphores_for_fences.push_back({ fence, command_queue->pending_semaphores_for_fence[i] });
			}

			command_queue->pending_semaphores_for_fence.clear();
		}

		if (present_semaphore != VK_NULL_HANDLE) {
			// If command buffers were executed, swap chains must wait on the present semaphore used by the command queue.
			wait_semaphores.clear();
			wait_semaphores.push_back(present_semaphore);
		}
	}

	if (p_swap_chains.size() > 0) {
		thread_local LocalVector<VkSwapchainKHR> swapchains;
		thread_local LocalVector<uint32_t> image_indices;
		thread_local LocalVector<VkResult> results;
		swapchains.clear();
		image_indices.clear();

		for (uint32_t i = 0; i < p_swap_chains.size(); i++) {
			SwapChain* swap_chain = (SwapChain*)(p_swap_chains[i].id);
			swapchains.push_back(swap_chain->vk_swapchain);
			DEV_ASSERT(swap_chain->image_index < swap_chain->images.size());
			image_indices.push_back(swap_chain->image_index);
		}

		results.resize(swapchains.size());

		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = wait_semaphores.size();
		present_info.pWaitSemaphores = wait_semaphores.ptr();
		present_info.swapchainCount = swapchains.size();
		present_info.pSwapchains = swapchains.ptr();
		// 每个交换链呈现哪个图像
		present_info.pImageIndices = image_indices.ptr();//pImageIndices 是指向每个交换链可呈现图像数组的索引数组的指针，其中包含 swapchainCount 条目。该数组中的每个条目标识要在 pSwapchains 数组中的相应条目上呈现的图像。
		present_info.pResults = results.ptr();

		device_queue.submit_mutex.lock();
		err = device_functions.QueuePresentKHR(device_queue.queue, &present_info);
		device_queue.submit_mutex.unlock();

		// Set the index to an invalid value. If any of the swap chains returned out of date, indicate it should be resized the next time it's acquired.
		bool any_result_is_out_of_date = false;
		for (uint32_t i = 0; i < p_swap_chains.size(); i++) {
			SwapChain* swap_chain = (SwapChain*)(p_swap_chains[i].id);
			swap_chain->image_index = UINT_MAX;
			if (results[i] == VK_ERROR_OUT_OF_DATE_KHR) {
				context_driver->surface_set_needs_resize(swap_chain->surface, true);
				any_result_is_out_of_date = true;
			}
		}

		if (any_result_is_out_of_date || err == VK_ERROR_OUT_OF_DATE_KHR) {
			// It is possible for presentation to fail with out of date while acquire might've succeeded previously. This case
			// will be considered a silent failure as it can be triggered easily by resizing a window in the OS natively.
			return FAILED;
		}

		// Handling VK_SUBOPTIMAL_KHR the same as VK_SUCCESS is completely intentional.
		//
		// Godot does not currently support native rotation in Android when creating the swap chain. It intentionally uses
		// VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR instead of the current transform bits available in the surface capabilities.
		// Choosing the transform that leads to optimal presentation leads to distortion that makes the application unusable,
		// as the rotation of all the content is not handled at the moment.
		//
		// VK_SUBOPTIMAL_KHR is accepted as a successful case even if it's not the most efficient solution to work around this
		// problem. This behavior should not be changed unless the swap chain recreation uses the current transform bits, as
		// it'll lead to very low performance in Android by entering an endless loop where it'll always resize the swap chain
		// every frame.

		ERR_FAIL_COND_V(err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR, FAILED);
	}

	return OK;
}
/// <summary>
/// extensions, features, capabilities, queue,
/// allocator, pipeline cache
/// 类变量标记不明确
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
	present_frame_latency = GLOBAL_GET("rendering/rendering_device/vulkan/present_wait_latency");

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
	// false means optical
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
	// vulkan hdr support ?
	_register_requested_device_extension(VK_EXT_HDR_METADATA_EXTENSION_NAME, false);
	// vulkan wait_delay support?
	_register_requested_device_extension(VK_KHR_PRESENT_ID_EXTENSION_NAME, false);
	_register_requested_device_extension(VK_KHR_PRESENT_WAIT_EXTENSION_NAME, false);

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

	// Enable all extensions that are supported and requested.
	for (uint32_t i = 0; i < device_extension_count; i++) {
		CharString extension_name(device_extensions[i].extensionName);
		if (requested_device_extensions.has(extension_name)) {
			enabled_device_extension_names.insert(extension_name);
		}
	}

	// Now check our requested extensions.
	for (KeyValue<CharString, bool>& requested_extension : requested_device_extensions) {
		if (!enabled_device_extension_names.has(requested_extension.key)) {
			if (requested_extension.value) {
				ERR_FAIL_V_MSG(ERR_BUG, String("Required extension ") + String::utf8(requested_extension.key) + String(" not found."));
			}
			else {
				print_verbose(String("Optional extension ") + String::utf8(requested_extension.key) + String(" not found"));
			}
		}
	}

	return OK;
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
		create_info.queueCount = MIN(queue_family_properties[i].queueCount, max_queue_count_per_family);  // 每个队列族最多1个【为什么？】
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

	/*for (size_t i = 0; i < queue_family_properties.size(); queue_family_properties) {
		auto index = CommandQueueFamilyID(i + 1);
		queueid_to_family[index] = (CommandQueueFamilyBits) queue_family_properties[i].queueFlags;
		static const auto _add_ids =  [&](CommandQueueFamilyBits bit) {
			if ((CommandQueueFamilyBits)queue_family_properties[i].queueFlags & bit)
				queuefamily_to_ids[bit].push_back(index);
			};
		_add_ids(COMMAND_QUEUE_FAMILY_GRAPHICS_BIT);
		_add_ids(COMMAND_QUEUE_FAMILY_COMPUTE_BIT);
		_add_ids(COMMAND_QUEUE_FAMILY_TRANSFER_BIT);
	}*/
	

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
	static auto& funcs = context_driver->functions_get();
	static const VmaVulkanFunctions p_funcs = {
		vkGetInstanceProcAddr = ::vkGetInstanceProcAddr,
		vkGetDeviceProcAddr = funcs.GetDeviceProcAddr,
	};
	allocator_info.pVulkanFunctions = &p_funcs;
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
	/****************/
	/**** Swap Chain ****/
	/****************/
	// create swap chain对象
	RDD::SwapChainID RenderingDeviceDriverVulkan::swap_chain_create(RenderingContextDriver::SurfaceID p_surface) {
		DEV_ASSERT(p_surface != 0);

		RenderingContextDriverVulkan::Surface* surface = (RenderingContextDriverVulkan::Surface*)(p_surface);
		const RenderingContextDriverVulkan::Functions& functions = context_driver->functions_get();

		// Retrieve the formats supported by the surface.
		uint32_t format_count = 0;
		VkResult err = functions.GetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface->vk_surface, &format_count, nullptr);
		ERR_FAIL_COND_V(err != VK_SUCCESS, SwapChainID());

		TightLocalVector<VkSurfaceFormatKHR> formats;
		formats.resize(format_count);
		err = functions.GetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface->vk_surface, &format_count, formats.ptr());
		ERR_FAIL_COND_V(err != VK_SUCCESS, SwapChainID());

		VkFormat format = VK_FORMAT_UNDEFINED;
		VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		// surface具有format
		if (format_count == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
			// If the format list includes just one entry of VK_FORMAT_UNDEFINED, the surface has no preferred format.
			format = VK_FORMAT_B8G8R8A8_UNORM;
			color_space = formats[0].colorSpace;
		}
		else if (format_count > 0) {
			// Use one of the supported formats, prefer B8G8R8A8_UNORM.
			// @TODO 修改这里的逻辑
			const VkFormat preferred_format = VK_FORMAT_B8G8R8A8_UNORM;
			const VkFormat second_format = VK_FORMAT_R8G8B8A8_UNORM;
			for (uint32_t i = 0; i < format_count; i++) {
				if (formats[i].format == preferred_format || formats[i].format == second_format) {
					format = formats[i].format;
					if (formats[i].format == preferred_format) {
						// This is the preferred format, stop searching.
						break;
					}
				}
			}
		}

		// No formats are supported.
		ERR_FAIL_COND_V_MSG(format == VK_FORMAT_UNDEFINED, SwapChainID(), "Surface did not return any valid formats.");

		// Create the render pass for the chosen format.
		// --- display render pass ---
		VkAttachmentDescription2KHR attachment = {};
		attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
		attachment.format = format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference2KHR color_reference = {};
		color_reference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
		color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription2KHR subpass = {};
		subpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_reference;

		VkRenderPassCreateInfo2KHR pass_info = {};
		pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR;
		pass_info.attachmentCount = 1;
		pass_info.pAttachments = &attachment;
		pass_info.subpassCount = 1;
		pass_info.pSubpasses = &subpass;

		VkRenderPass render_pass = VK_NULL_HANDLE;
		err = _create_render_pass(vk_device, &pass_info, nullptr, &render_pass);
		ERR_FAIL_COND_V(err != VK_SUCCESS, SwapChainID());

		SwapChain* swap_chain = memnew(SwapChain);
		swap_chain->surface = p_surface;
		swap_chain->format = format;
		swap_chain->color_space = color_space;
		swap_chain->render_pass = RenderPassID(render_pass);
		return SwapChainID(swap_chain);
	}


	void RenderingDeviceDriverVulkan::_swap_chain_release(SwapChain* swap_chain) {
		// Destroy views and framebuffers associated to the swapchain's images.
		for (FramebufferID framebuffer : swap_chain->framebuffers) {
			framebuffer_free(framebuffer);
		}

		for (VkImageView view : swap_chain->image_views) {
			vkDestroyImageView(vk_device, view, nullptr);
		}

		swap_chain->image_index = UINT_MAX;
		swap_chain->images.clear();
		swap_chain->image_views.clear();
		swap_chain->framebuffers.clear();

		if (swap_chain->vk_swapchain != VK_NULL_HANDLE) {
			device_functions.DestroySwapchainKHR(vk_device, swap_chain->vk_swapchain, nullptr);
			swap_chain->vk_swapchain = VK_NULL_HANDLE;
		}

		for (uint32_t i = 0; i < swap_chain->command_queues_acquired.size(); i++) {
			_recreate_image_semaphore(swap_chain->command_queues_acquired[i], swap_chain->command_queues_acquired_semaphores[i], false);
		}

		swap_chain->command_queues_acquired.clear();
		swap_chain->command_queues_acquired_semaphores.clear();
	}

	bool RenderingDeviceDriverVulkan::_recreate_image_semaphore(CommandQueue* p_command_queue, uint32_t p_semaphore_index, bool p_release_on_swap_chain) {
		_release_image_semaphore(p_command_queue, p_semaphore_index, p_release_on_swap_chain);

		VkSemaphore semaphore;
		VkSemaphoreCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkResult err = vkCreateSemaphore(vk_device, &create_info, nullptr, &semaphore);
		ERR_FAIL_COND_V(err != VK_SUCCESS, false);

		// Indicate the semaphore is free again and destroy the previous one before storing the new one.
		vkDestroySemaphore(vk_device, p_command_queue->image_semaphores[p_semaphore_index], nullptr);

		p_command_queue->image_semaphores[p_semaphore_index] = semaphore;
		p_command_queue->free_image_semaphores.push_back(p_semaphore_index);

		return true;
	}

	Error RenderingDeviceDriverVulkan::swap_chain_resize(CommandQueueID p_cmd_queue, SwapChainID p_swap_chain, uint32_t p_desired_framebuffer_count) {
		DEV_ASSERT(p_cmd_queue.id != 0);
		DEV_ASSERT(p_swap_chain.id != 0);

		CommandQueue* command_queue = (CommandQueue*)(p_cmd_queue.id);
		SwapChain* swap_chain = (SwapChain*)(p_swap_chain.id);

		// Release all current contents of the swap chain.
		_swap_chain_release(swap_chain);

		// Validate if the command queue being used supports creating the swap chain for this surface.
		const RenderingContextDriverVulkan::Functions& functions = context_driver->functions_get();
		if (!context_driver->queue_family_supports_present(physical_device, command_queue->queue_family, swap_chain->surface)) {
			ERR_FAIL_V_MSG(ERR_CANT_CREATE, "Surface is not supported by device. Did the GPU go offline? Was the window created on another monitor? Check"
				"previous errors & try launching with --gpu-validation.");
		}

		// Retrieve the surface's capabilities.
		RenderingContextDriverVulkan::Surface* surface = (RenderingContextDriverVulkan::Surface*)(swap_chain->surface);
		VkSurfaceCapabilitiesKHR surface_capabilities = {};
		VkResult err = functions.GetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface->vk_surface, &surface_capabilities);
		ERR_FAIL_COND_V(err != VK_SUCCESS, ERR_CANT_CREATE);

		VkExtent2D extent;
		if (surface_capabilities.currentExtent.width == 0xFFFFFFFF) {
			// The current extent is currently undefined, so the current surface width and height will be clamped to the surface's capabilities.
			extent.width = CLAMP(surface->width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
			extent.height = CLAMP(surface->height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
		}
		else {
			// Grab the dimensions from the current extent.
			extent = surface_capabilities.currentExtent;
			surface->width = extent.width;
			surface->height = extent.height;
		}

		if (surface->width == 0 || surface->height == 0) {
			// The surface doesn't have valid dimensions, so we can't create a swap chain.
			return ERR_SKIP;
		}

		// Find what present modes are supported.
		TightLocalVector<VkPresentModeKHR> present_modes;
		uint32_t present_modes_count = 0;
		err = functions.GetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface->vk_surface, &present_modes_count, nullptr);
		ERR_FAIL_COND_V(err != VK_SUCCESS, ERR_CANT_CREATE);

		present_modes.resize(present_modes_count);
		err = functions.GetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface->vk_surface, &present_modes_count, present_modes.ptr());
		ERR_FAIL_COND_V(err != VK_SUCCESS, ERR_CANT_CREATE);

		// Choose the present mode based on the display server setting.
		VkPresentModeKHR present_mode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
		String present_mode_name = "Enabled";
		switch (surface->vsync_mode) {
		case WindowSystem::VSYNC_MAILBOX:
			present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
			present_mode_name = "Mailbox";
			break;
		case WindowSystem::VSYNC_ADAPTIVE:
			present_mode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
			present_mode_name = "Adaptive";
			break;
		case WindowSystem::VSYNC_ENABLED:
			present_mode = VK_PRESENT_MODE_FIFO_KHR;
			present_mode_name = "Enabled";
			break;
		case WindowSystem::VSYNC_DISABLED:
			present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			present_mode_name = "Disabled";
			break;
		}

		bool present_mode_available = present_modes.find(present_mode) >= 0;
		if (present_mode_available) {
			print_verbose("Using present mode: " + present_mode_name);
		}
		else {
			// Present mode is not available, fall back to FIFO which is guaranteed to be supported.
			WARN_PRINT(vformat("The requested V-Sync mode %s is not available. Falling back to V-Sync mode Enabled.", present_mode_name));
			surface->vsync_mode = WindowSystem::VSYNC_ENABLED;
			present_mode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
		}

		// Clamp the desired image count to the surface's capabilities.
		uint32_t desired_swapchain_images = MAX(p_desired_framebuffer_count, surface_capabilities.minImageCount);
		if (surface_capabilities.maxImageCount > 0) {
			// Only clamp to the max image count if it's defined. A max image count of 0 means there's no upper limit to the amount of images.
			desired_swapchain_images = MIN(desired_swapchain_images, surface_capabilities.maxImageCount);
		}

		// Prefer identity transform if it's supported, use the current transform otherwise.
		// This behavior is intended as Godot does not supported *native rotation in platforms that use these bits.*
		// Refer to the comment in command_queue_present() for more details.
		VkSurfaceTransformFlagBitsKHR surface_transform_bits;
		if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
			surface_transform_bits = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else {
			surface_transform_bits = surface_capabilities.currentTransform;
		}

		VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		if (GLOBAL_GET("display/window/per_pixel_transparency/allowed").operator bool()|| !(surface_capabilities.supportedCompositeAlpha & composite_alpha)) {
			// Find a supported composite alpha mode - one of these is guaranteed to be set.
			VkCompositeAlphaFlagBitsKHR composite_alpha_flags[4] = {
				VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
				VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
				VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
				VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
			};

			for (uint32_t i = 0; i < 4; i++) { // 找第一个匹配的
				if (surface_capabilities.supportedCompositeAlpha & composite_alpha_flags[i]) {
					composite_alpha = composite_alpha_flags[i];
					break;
				}
			}
		}

		VkSwapchainCreateInfoKHR swap_create_info = {};
		swap_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swap_create_info.surface = surface->vk_surface;
		swap_create_info.minImageCount = desired_swapchain_images;
		swap_create_info.imageFormat = swap_chain->format;
		swap_create_info.imageColorSpace = swap_chain->color_space;
		swap_create_info.imageExtent = extent;
		swap_create_info.imageArrayLayers = 1;
		swap_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swap_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swap_create_info.preTransform = surface_transform_bits;
		swap_create_info.compositeAlpha = composite_alpha;
		swap_create_info.presentMode = present_mode;
		swap_create_info.clipped = true;
		err = device_functions.CreateSwapchainKHR(vk_device, &swap_create_info, nullptr, &swap_chain->vk_swapchain);
		ERR_FAIL_COND_V(err != VK_SUCCESS, ERR_CANT_CREATE);

		uint32_t image_count = 0; // 交换链 image
		err = device_functions.GetSwapchainImagesKHR(vk_device, swap_chain->vk_swapchain, &image_count, nullptr);
		ERR_FAIL_COND_V(err != VK_SUCCESS, ERR_CANT_CREATE);
		print_verbose("swap chain contains " , itos(image_count), "images")
		swap_chain->images.resize(image_count);
		err = device_functions.GetSwapchainImagesKHR(vk_device, swap_chain->vk_swapchain, &image_count, swap_chain->images.ptr());
		ERR_FAIL_COND_V(err != VK_SUCCESS, ERR_CANT_CREATE);

		VkImageViewCreateInfo view_create_info = {};
		view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_create_info.format = swap_chain->format;
		view_create_info.components.r = VK_COMPONENT_SWIZZLE_R; // 什么都不设置
		view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
		view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
		view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;
		view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_create_info.subresourceRange.levelCount = 1;
		view_create_info.subresourceRange.layerCount = 1; // 渲染目标，只有一个图层

		swap_chain->image_views.reserve(image_count);

		VkImageView image_view;
		for (uint32_t i = 0; i < image_count; i++) {
			view_create_info.image = swap_chain->images[i];
			err = vkCreateImageView(vk_device, &view_create_info, nullptr, &image_view);
			ERR_FAIL_COND_V(err != VK_SUCCESS, ERR_CANT_CREATE);

			swap_chain->image_views.push_back(image_view);
		}

		swap_chain->framebuffers.reserve(image_count);

		VkFramebufferCreateInfo fb_create_info = {};
		fb_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_create_info.renderPass = VkRenderPass(swap_chain->render_pass.id);
		fb_create_info.attachmentCount = 1;
		fb_create_info.width = surface->width;
		fb_create_info.height = surface->height;
		fb_create_info.layers = 1;

		VkFramebuffer framebuffer;
		for (uint32_t i = 0; i < image_count; i++) {
			fb_create_info.pAttachments = &swap_chain->image_views[i];
			err = vkCreateFramebuffer(vk_device, &fb_create_info, nullptr, &framebuffer);
			ERR_FAIL_COND_V(err != VK_SUCCESS, ERR_CANT_CREATE);

			swap_chain->framebuffers.push_back(RDD::FramebufferID(framebuffer));
		}

		//// Once everything's been created correctly, indicate the surface no longer needs to be resized.
		context_driver->surface_set_needs_resize(swap_chain->surface, false);

		return OK;
	}

	RDD::FramebufferID RenderingDeviceDriverVulkan::framebuffer_create(RenderPassID p_render_pass, VectorView<TextureID> p_attachments, uint32_t p_width, uint32_t p_height) {
		VkImageView* vk_img_views = ALLOCA_ARRAY(VkImageView, p_attachments.size());
		for (uint32_t i = 0; i < p_attachments.size(); i++) {
			vk_img_views[i] = ((const TextureInfo*)p_attachments[i].id)->vk_view;
		}

		VkFramebufferCreateInfo framebuffer_create_info = {};
		framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.renderPass = (VkRenderPass)p_render_pass.id;
		framebuffer_create_info.attachmentCount = p_attachments.size();
		framebuffer_create_info.pAttachments = vk_img_views;
		framebuffer_create_info.width = p_width;
		framebuffer_create_info.height = p_height;
		framebuffer_create_info.layers = 1;

		VkFramebuffer vk_framebuffer = VK_NULL_HANDLE;
		VkResult err = vkCreateFramebuffer(vk_device, &framebuffer_create_info, nullptr, &vk_framebuffer);
		ERR_FAIL_COND_V_MSG(err, FramebufferID(), "vkCreateFramebuffer failed with error " + itos(err) + ".");

#if PRINT_NATIVE_COMMANDS
		print_line(vformat("vkCreateFramebuffer 0x%uX with %d attachments", uint64_t(vk_framebuffer), p_attachments.size()));
		for (uint32_t i = 0; i < p_attachments.size(); i++) {
			const TextureInfo* attachment_info = (const TextureInfo*)p_attachments[i].id;
			print_line(vformat("  Attachment #%d: IMAGE 0x%uX VIEW 0x%uX", i, uint64_t(attachment_info->vk_view_create_info.image), uint64_t(attachment_info->vk_view)));
		}
#endif

		return FramebufferID(vk_framebuffer);
	}

	void RenderingDeviceDriverVulkan::framebuffer_free(FramebufferID p_framebuffer) {
		vkDestroyFramebuffer(vk_device, (VkFramebuffer)p_framebuffer.id, nullptr);
	}
	
	bool RenderingDeviceDriverVulkan::_release_image_semaphore(CommandQueue* p_command_queue, uint32_t p_semaphore_index, bool p_release_on_swap_chain) {
		SwapChain* swap_chain = p_command_queue->image_semaphores_swap_chains[p_semaphore_index];
		if (swap_chain != nullptr) {
			// Clear the swap chain from the command queue's vector.
			p_command_queue->image_semaphores_swap_chains[p_semaphore_index] = nullptr;

			if (p_release_on_swap_chain) {
				// Remove the acquired semaphore from the swap chain's vectors.
				for (uint32_t i = 0; i < swap_chain->command_queues_acquired.size(); i++) {
					if (swap_chain->command_queues_acquired[i] == p_command_queue && swap_chain->command_queues_acquired_semaphores[i] == p_semaphore_index) {
						swap_chain->command_queues_acquired.remove_at(i);
						swap_chain->command_queues_acquired_semaphores.remove_at(i);
						break;
					}
				}
			}

			return true;
		}

		return false;
	}
	static void _convert_subpass_attachments(const VkAttachmentReference2* p_attachment_references_2, uint32_t p_attachment_references_count, TightLocalVector<VkAttachmentReference>& r_attachment_references) {
		r_attachment_references.resize(p_attachment_references_count);
		for (uint32_t i = 0; i < p_attachment_references_count; i++) {
			// Ignore sType, pNext and aspectMask (which is currently unused).
			r_attachment_references[i].attachment = p_attachment_references_2[i].attachment;
			r_attachment_references[i].layout = p_attachment_references_2[i].layout;
		}
	}

	VkResult RenderingDeviceDriverVulkan::_create_render_pass(VkDevice p_device, const VkRenderPassCreateInfo2* p_create_info, const VkAllocationCallbacks* p_allocator, VkRenderPass* p_render_pass) {
		if (device_functions.CreateRenderPass2KHR != nullptr) {
			return device_functions.CreateRenderPass2KHR(p_device, p_create_info, p_allocator, p_render_pass);
		}
		else {
			// Compatibility fallback with regular create render pass but by converting the inputs from the newer version to the older one.
			TightLocalVector<VkAttachmentDescription> attachments;
			attachments.resize(p_create_info->attachmentCount);
			for (uint32_t i = 0; i < p_create_info->attachmentCount; i++) {
				// Ignores sType and pNext from the attachment.
				const VkAttachmentDescription2& src = p_create_info->pAttachments[i];
				VkAttachmentDescription& dst = attachments[i];
				dst.flags = src.flags;
				dst.format = src.format;
				dst.samples = src.samples;
				dst.loadOp = src.loadOp;
				dst.storeOp = src.storeOp;
				dst.stencilLoadOp = src.stencilLoadOp;
				dst.stencilStoreOp = src.stencilStoreOp;
				dst.initialLayout = src.initialLayout;
				dst.finalLayout = src.finalLayout;
			}

			const uint32_t attachment_vectors_per_subpass = 4; // input color resolve depth
			TightLocalVector<TightLocalVector<VkAttachmentReference>> subpasses_attachments;
			TightLocalVector<VkSubpassDescription> subpasses;
			subpasses_attachments.resize(p_create_info->subpassCount * attachment_vectors_per_subpass);
			subpasses.resize(p_create_info->subpassCount);

			for (uint32_t i = 0; i < p_create_info->subpassCount; i++) {
				const uint32_t vector_base_index = i * attachment_vectors_per_subpass;
				const uint32_t input_attachments_index = vector_base_index + 0;
				const uint32_t color_attachments_index = vector_base_index + 1;
				const uint32_t resolve_attachments_index = vector_base_index + 2;
				const uint32_t depth_attachment_index = vector_base_index + 3;
				_convert_subpass_attachments(p_create_info->pSubpasses[i].pInputAttachments, p_create_info->pSubpasses[i].inputAttachmentCount, subpasses_attachments[input_attachments_index]);
				_convert_subpass_attachments(p_create_info->pSubpasses[i].pColorAttachments, p_create_info->pSubpasses[i].colorAttachmentCount, subpasses_attachments[color_attachments_index]);
				_convert_subpass_attachments(p_create_info->pSubpasses[i].pResolveAttachments, (p_create_info->pSubpasses[i].pResolveAttachments != nullptr) ? p_create_info->pSubpasses[i].colorAttachmentCount : 0, subpasses_attachments[resolve_attachments_index]);
				_convert_subpass_attachments(p_create_info->pSubpasses[i].pDepthStencilAttachment, (p_create_info->pSubpasses[i].pDepthStencilAttachment != nullptr) ? 1 : 0, subpasses_attachments[depth_attachment_index]);

				// Ignores sType and pNext from the subpass.
				const VkSubpassDescription2& src_subpass = p_create_info->pSubpasses[i];
				VkSubpassDescription& dst_subpass = subpasses[i]; // 这里的pInputAttachment是VkAttachmentReference
				// 而不是VkAttachmentReference2
				dst_subpass.flags = src_subpass.flags;
				dst_subpass.pipelineBindPoint = src_subpass.pipelineBindPoint;
				dst_subpass.inputAttachmentCount = src_subpass.inputAttachmentCount;
				dst_subpass.pInputAttachments = subpasses_attachments[input_attachments_index].ptr();
				dst_subpass.colorAttachmentCount = src_subpass.colorAttachmentCount;
				dst_subpass.pColorAttachments = subpasses_attachments[color_attachments_index].ptr();
				dst_subpass.pResolveAttachments = subpasses_attachments[resolve_attachments_index].ptr();
				dst_subpass.pDepthStencilAttachment = subpasses_attachments[depth_attachment_index].ptr();
				dst_subpass.preserveAttachmentCount = src_subpass.preserveAttachmentCount;
				dst_subpass.pPreserveAttachments = src_subpass.pPreserveAttachments;
			}

			TightLocalVector<VkSubpassDependency> dependencies;
			dependencies.resize(p_create_info->dependencyCount);

			for (uint32_t i = 0; i < p_create_info->dependencyCount; i++) {
				// Ignores sType and pNext from the dependency, and viewMask which is currently unused.
				const VkSubpassDependency2& src_dependency = p_create_info->pDependencies[i];
				VkSubpassDependency& dst_dependency = dependencies[i];
				dst_dependency.srcSubpass = src_dependency.srcSubpass;
				dst_dependency.dstSubpass = src_dependency.dstSubpass;
				dst_dependency.srcStageMask = src_dependency.srcStageMask;
				dst_dependency.dstStageMask = src_dependency.dstStageMask;
				dst_dependency.srcAccessMask = src_dependency.srcAccessMask;
				dst_dependency.dstAccessMask = src_dependency.dstAccessMask;
				dst_dependency.dependencyFlags = src_dependency.dependencyFlags; 
			}// 差一个viewoffset的字段

			VkRenderPassCreateInfo create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			create_info.pNext = p_create_info->pNext;
			create_info.flags = p_create_info->flags;
			create_info.attachmentCount = attachments.size();
			create_info.pAttachments = attachments.ptr();
			create_info.subpassCount = subpasses.size();
			create_info.pSubpasses = subpasses.ptr();
			create_info.dependencyCount = dependencies.size();
			create_info.pDependencies = dependencies.ptr();
			return vkCreateRenderPass(vk_device, &create_info, p_allocator, p_render_pass);
		}
		
		return VK_SUCCESS;
	}


	void RenderingDeviceDriverVulkan::command_queue_free(CommandQueueID p_cmd_queue) {
		DEV_ASSERT(p_cmd_queue);

		CommandQueue* command_queue = (CommandQueue*)(p_cmd_queue.id);

		// Erase all the semaphores used for presentation.
		for (VkSemaphore semaphore : command_queue->present_semaphores) {
			vkDestroySemaphore(vk_device, semaphore, nullptr);
		}

		// Erase all the semaphores used for image acquisition.
		for (VkSemaphore semaphore : command_queue->image_semaphores) {
			vkDestroySemaphore(vk_device, semaphore, nullptr);
		}

		// Retrieve the queue family corresponding to the virtual queue.
		DEV_ASSERT(command_queue->queue_family < queue_families.size());
		TightLocalVector<Queue>& queue_family = queue_families[command_queue->queue_family];

		// Decrease the virtual queue count.
		DEV_ASSERT(command_queue->queue_index < queue_family.size());
		DEV_ASSERT(queue_family[command_queue->queue_index].virtual_count > 0);
		queue_family[command_queue->queue_index].virtual_count--;

		// Destroy the virtual queue structure.
		memdelete(command_queue);
	}

	/// --- Fence ---
	RDD::FenceID RenderingDeviceDriverVulkan::fence_create() {
		VkFence vk_fence = VK_NULL_HANDLE;
		VkFenceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		VkResult err = vkCreateFence(vk_device, &create_info, nullptr, &vk_fence);
		ERR_FAIL_COND_V(err != VK_SUCCESS, FenceID());

		Fence* fence = memnew(Fence);
		fence->vk_fence = vk_fence;
		fence->queue_signaled_from = nullptr;
		return FenceID(fence);
	}
	Error RenderingDeviceDriverVulkan::fence_wait(FenceID p_fence) {
		Fence* fence = (Fence*)(p_fence.id);
		VkResult err = vkWaitForFences(vk_device, 1, &fence->vk_fence, VK_TRUE, UINT64_MAX);
		ERR_FAIL_COND_V(err != VK_SUCCESS, FAILED);

		err = vkResetFences(vk_device, 1, &fence->vk_fence);
		ERR_FAIL_COND_V(err != VK_SUCCESS, FAILED);

		if (fence->queue_signaled_from != nullptr) {
			// Release all semaphores that the command queue associated to the fence waited on the last time it was submitted.
			LocalVector<Pair<Fence*, uint32_t>>& pairs = fence->queue_signaled_from->image_semaphores_for_fences;
			uint32_t i = 0;
			while (i < pairs.size()) {
				if (pairs[i].first == fence) {
					_release_image_semaphore(fence->queue_signaled_from, pairs[i].second, true);
					fence->queue_signaled_from->free_image_semaphores.push_back(pairs[i].second);
					pairs.remove_at(i);
				}
				else {
					i++;
				}
			}

			fence->queue_signaled_from = nullptr;
		}

		return OK;
	}
	void RenderingDeviceDriverVulkan::fence_free(FenceID p_fence) {
		Fence* fence = (Fence*)(p_fence.id);
		vkDestroyFence(vk_device, fence->vk_fence, nullptr);
		memdelete(fence);
	}
	/********************/
	/**** SEMAPHORES ****/
	/********************/

	RDD::SemaphoreID RenderingDeviceDriverVulkan::semaphore_create() {
		VkSemaphore semaphore = VK_NULL_HANDLE;
		VkSemaphoreCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkResult err = vkCreateSemaphore(vk_device, &create_info, nullptr, &semaphore);
		ERR_FAIL_COND_V(err != VK_SUCCESS, SemaphoreID());

		return SemaphoreID(semaphore);
	}

	void RenderingDeviceDriverVulkan::semaphore_free(SemaphoreID p_semaphore) {
		vkDestroySemaphore(vk_device, VkSemaphore(p_semaphore.id), nullptr);
	}
	/**********************/
	/**** VERTEX ARRAY ****/
	/**********************/
	RDD::VertexFormatID RenderingDeviceDriverVulkan::vertex_format_create(VectorView<VertexAttribute> p_vertex_attribs) {
		VertexFormatInfo* vf_info = VersatileResource::allocate<VertexFormatInfo>(resources_allocator);
		// 有几个attrib就搞几个绑定
		vf_info->vk_bindings.resize(p_vertex_attribs.size());
		vf_info->vk_attributes.resize(p_vertex_attribs.size());
		for (uint32_t i = 0; i < p_vertex_attribs.size(); i++) {
			vf_info->vk_bindings[i] = {};
			vf_info->vk_bindings[i].binding = i;
			vf_info->vk_bindings[i].stride = p_vertex_attribs[i].stride;
			vf_info->vk_bindings[i].inputRate = p_vertex_attribs[i].frequency == VERTEX_FREQUENCY_INSTANCE ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
			vf_info->vk_attributes[i] = {};
			vf_info->vk_attributes[i].binding = i;
			vf_info->vk_attributes[i].location = p_vertex_attribs[i].location;
			vf_info->vk_attributes[i].format = RD_TO_VK_FORMAT[p_vertex_attribs[i].format];
			vf_info->vk_attributes[i].offset = p_vertex_attribs[i].offset;
		}
		vf_info->vk_create_info = {};
		vf_info->vk_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vf_info->vk_create_info.vertexBindingDescriptionCount = vf_info->vk_bindings.size();
		vf_info->vk_create_info.pVertexBindingDescriptions = vf_info->vk_bindings.ptr();
		vf_info->vk_create_info.vertexAttributeDescriptionCount = vf_info->vk_attributes.size();
		vf_info->vk_create_info.pVertexAttributeDescriptions = vf_info->vk_attributes.ptr();
		return VertexFormatID(vf_info);

	}
	void RenderingDeviceDriverVulkan::vertex_format_free(VertexFormatID p_vertex_format) {
		VertexFormatInfo* vf_info = (VertexFormatInfo*)p_vertex_format.id;
		VersatileResource::free(resources_allocator, vf_info);
	}

	/*****************/
	/**** BUFFERS ****/
	/*****************/


	RDD::BufferID RenderingDeviceDriverVulkan::buffer_create(uint64_t p_size, BitField<BufferUsageBits> p_usage, MemoryAllocationType p_allocation_type) {
		VkBufferCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		create_info.size = p_size;
		create_info.usage = p_usage;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // 这一项可以开放一点@TODO做一个标准的允许多种队列访问接口

		VmaAllocationCreateInfo alloc_create_info = {};
		switch (p_allocation_type) {
		case MEMORY_ALLOCATION_TYPE_CPU: {
			bool is_src = p_usage.has_flag(BUFFER_USAGE_TRANSFER_FROM_BIT);
			bool is_dst = p_usage.has_flag(BUFFER_USAGE_TRANSFER_TO_BIT);
			if (is_src && !is_dst) {
				// Looks like a staging buffer: CPU maps, writes sequentially, then GPU copies to VRAM.
				alloc_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			}
			if (is_dst && !is_src) {
				// Looks like a readback buffer: GPU copies from VRAM, then CPU maps and reads.
				alloc_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			}
			alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			alloc_create_info.requiredFlags = (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		} break;
		case MEMORY_ALLOCATION_TYPE_GPU: {
			alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			if (p_size <= SMALL_ALLOCATION_MAX_SIZE) {
				uint32_t mem_type_index = 0;
				vmaFindMemoryTypeIndexForBufferInfo(allocator, &create_info, &alloc_create_info, &mem_type_index);
				alloc_create_info.pool = _find_or_create_small_allocs_pool(mem_type_index);
			}
		} break;
		}

		VkBuffer vk_buffer = VK_NULL_HANDLE;
		VmaAllocation allocation = nullptr;
		VmaAllocationInfo alloc_info = {};
		VkResult err = vmaCreateBuffer(allocator, &create_info, &alloc_create_info, &vk_buffer, &allocation, &alloc_info);
		ERR_FAIL_COND_V_MSG(err, BufferID(), "Can't create buffer of size: " + itos(p_size) + ", error " + itos(err) + ".");

		// Bookkeep.

		BufferInfo* buf_info = VersatileResource::allocate<BufferInfo>(resources_allocator);
		buf_info->vk_buffer = vk_buffer;
		buf_info->allocation.handle = allocation;
		buf_info->allocation.size = alloc_info.size;
		buf_info->size = p_size;

		return BufferID(buf_info);
	}
	/// <summary>
	/// buffer view
	/// </summary>
	/// <param name="p_buffer"></param>
	/// <param name="p_format"></param>
	/// <returns></returns>
	bool RenderingDeviceDriverVulkan::buffer_set_texel_format(BufferID p_buffer, DataFormat p_format) {
		BufferInfo* buf_info = (BufferInfo*)p_buffer.id;

		DEV_ASSERT(!buf_info->vk_view);

		VkBufferViewCreateInfo view_create_info = {};
		view_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		view_create_info.buffer = buf_info->vk_buffer;
		view_create_info.format = RD_TO_VK_FORMAT[p_format];
		view_create_info.range = buf_info->allocation.size;

		VkResult res = vkCreateBufferView(vk_device, &view_create_info, nullptr, &buf_info->vk_view);
		ERR_FAIL_COND_V_MSG(res, false, "Unable to create buffer view, error " + itos(res) + ".");

		return true;
	}
	void RenderingDeviceDriverVulkan::buffer_free(BufferID p_buffer) {
		BufferInfo* buf_info = (BufferInfo*)p_buffer.id;
		if (buf_info->vk_view) {
			vkDestroyBufferView(vk_device, buf_info->vk_view, nullptr);

		}
		vmaDestroyBuffer(allocator, buf_info->vk_buffer, buf_info->allocation.handle);
		VersatileResource::free(resources_allocator, buf_info);
	}
	uint64_t RenderingDeviceDriverVulkan::buffer_get_allocation_size(BufferID p_buffer) {
		const BufferInfo* buf_info = (const BufferInfo*)p_buffer.id;
		return buf_info->allocation.size;
	}

	uint8_t* RenderingDeviceDriverVulkan::buffer_map(BufferID p_buffer) { 
		const BufferInfo* buf_info = (const BufferInfo*)p_buffer.id;
		void* data_ptr = nullptr;
		VkResult err = vmaMapMemory(allocator, buf_info->allocation.handle, &data_ptr);
		ERR_FAIL_COND_V_MSG(err, nullptr, "vmaMapMemory failed with error " + itos(err) + ".");
		return (uint8_t*)data_ptr;
	}

	void RenderingDeviceDriverVulkan::buffer_unmap(BufferID p_buffer) {
		const BufferInfo* buf_info = (const BufferInfo*)p_buffer.id;
		vmaUnmapMemory(allocator, buf_info->allocation.handle);
	}
} // namespace graphics

}// namespace lain