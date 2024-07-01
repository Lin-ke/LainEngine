#pragma once
#ifndef RENDERING_DEVICE_COMMONS_H
#define RENDERING_DEVICE_COMMONS_H
#include "core/typedefs.h"
#include "core/object/object.h"
#include "core/meta/type_info.h"
#define STEPIFY(m_number, m_alignment) ((((m_number) + ((m_alignment)-1)) / (m_alignment)) * (m_alignment))
namespace lain {
	namespace graphics {

	// transform of dataformat
	// None -> Max

	class RenderingDeviceCommons : public Object {
		
	public:
		static const int INVALID_ID = -1;
		enum DataFormat {
			DATA_FORMAT_R4G4_UNORM_PACK8,
			DATA_FORMAT_R4G4B4A4_UNORM_PACK16,
			DATA_FORMAT_B4G4R4A4_UNORM_PACK16,
			DATA_FORMAT_R5G6B5_UNORM_PACK16,
			DATA_FORMAT_B5G6R5_UNORM_PACK16,
			DATA_FORMAT_R5G5B5A1_UNORM_PACK16,
			DATA_FORMAT_B5G5R5A1_UNORM_PACK16,
			DATA_FORMAT_A1R5G5B5_UNORM_PACK16,
			DATA_FORMAT_R8_UNORM,
			DATA_FORMAT_R8_SNORM,
			DATA_FORMAT_R8_USCALED,
			DATA_FORMAT_R8_SSCALED,
			DATA_FORMAT_R8_UINT,
			DATA_FORMAT_R8_SINT,
			DATA_FORMAT_R8_SRGB,
			DATA_FORMAT_R8G8_UNORM,
			DATA_FORMAT_R8G8_SNORM,
			DATA_FORMAT_R8G8_USCALED,
			DATA_FORMAT_R8G8_SSCALED,
			DATA_FORMAT_R8G8_UINT,
			DATA_FORMAT_R8G8_SINT,
			DATA_FORMAT_R8G8_SRGB,
			DATA_FORMAT_R8G8B8_UNORM,
			DATA_FORMAT_R8G8B8_SNORM,
			DATA_FORMAT_R8G8B8_USCALED,
			DATA_FORMAT_R8G8B8_SSCALED,
			DATA_FORMAT_R8G8B8_UINT,
			DATA_FORMAT_R8G8B8_SINT,
			DATA_FORMAT_R8G8B8_SRGB,
			DATA_FORMAT_B8G8R8_UNORM,
			DATA_FORMAT_B8G8R8_SNORM,
			DATA_FORMAT_B8G8R8_USCALED,
			DATA_FORMAT_B8G8R8_SSCALED,
			DATA_FORMAT_B8G8R8_UINT,
			DATA_FORMAT_B8G8R8_SINT,
			DATA_FORMAT_B8G8R8_SRGB,
			DATA_FORMAT_R8G8B8A8_UNORM,
			DATA_FORMAT_R8G8B8A8_SNORM,
			DATA_FORMAT_R8G8B8A8_USCALED,
			DATA_FORMAT_R8G8B8A8_SSCALED,
			DATA_FORMAT_R8G8B8A8_UINT,
			DATA_FORMAT_R8G8B8A8_SINT,
			DATA_FORMAT_R8G8B8A8_SRGB,
			DATA_FORMAT_B8G8R8A8_UNORM,
			DATA_FORMAT_B8G8R8A8_SNORM,
			DATA_FORMAT_B8G8R8A8_USCALED,
			DATA_FORMAT_B8G8R8A8_SSCALED,
			DATA_FORMAT_B8G8R8A8_UINT,
			DATA_FORMAT_B8G8R8A8_SINT,
			DATA_FORMAT_B8G8R8A8_SRGB,
			DATA_FORMAT_A8B8G8R8_UNORM_PACK32,
			DATA_FORMAT_A8B8G8R8_SNORM_PACK32,
			DATA_FORMAT_A8B8G8R8_USCALED_PACK32,
			DATA_FORMAT_A8B8G8R8_SSCALED_PACK32,
			DATA_FORMAT_A8B8G8R8_UINT_PACK32,
			DATA_FORMAT_A8B8G8R8_SINT_PACK32,
			DATA_FORMAT_A8B8G8R8_SRGB_PACK32,
			DATA_FORMAT_A2R10G10B10_UNORM_PACK32,
			DATA_FORMAT_A2R10G10B10_SNORM_PACK32,
			DATA_FORMAT_A2R10G10B10_USCALED_PACK32,
			DATA_FORMAT_A2R10G10B10_SSCALED_PACK32,
			DATA_FORMAT_A2R10G10B10_UINT_PACK32,
			DATA_FORMAT_A2R10G10B10_SINT_PACK32,
			DATA_FORMAT_A2B10G10R10_UNORM_PACK32,
			DATA_FORMAT_A2B10G10R10_SNORM_PACK32,
			DATA_FORMAT_A2B10G10R10_USCALED_PACK32,
			DATA_FORMAT_A2B10G10R10_SSCALED_PACK32,
			DATA_FORMAT_A2B10G10R10_UINT_PACK32,
			DATA_FORMAT_A2B10G10R10_SINT_PACK32,
			DATA_FORMAT_R16_UNORM,
			DATA_FORMAT_R16_SNORM,
			DATA_FORMAT_R16_USCALED,
			DATA_FORMAT_R16_SSCALED,
			DATA_FORMAT_R16_UINT,
			DATA_FORMAT_R16_SINT,
			DATA_FORMAT_R16_SFLOAT,
			DATA_FORMAT_R16G16_UNORM,
			DATA_FORMAT_R16G16_SNORM,
			DATA_FORMAT_R16G16_USCALED,
			DATA_FORMAT_R16G16_SSCALED,
			DATA_FORMAT_R16G16_UINT,
			DATA_FORMAT_R16G16_SINT,
			DATA_FORMAT_R16G16_SFLOAT,
			DATA_FORMAT_R16G16B16_UNORM,
			DATA_FORMAT_R16G16B16_SNORM,
			DATA_FORMAT_R16G16B16_USCALED,
			DATA_FORMAT_R16G16B16_SSCALED,
			DATA_FORMAT_R16G16B16_UINT,
			DATA_FORMAT_R16G16B16_SINT,
			DATA_FORMAT_R16G16B16_SFLOAT,
			DATA_FORMAT_R16G16B16A16_UNORM,
			DATA_FORMAT_R16G16B16A16_SNORM,
			DATA_FORMAT_R16G16B16A16_USCALED,
			DATA_FORMAT_R16G16B16A16_SSCALED,
			DATA_FORMAT_R16G16B16A16_UINT,
			DATA_FORMAT_R16G16B16A16_SINT,
			DATA_FORMAT_R16G16B16A16_SFLOAT,
			DATA_FORMAT_R32_UINT,
			DATA_FORMAT_R32_SINT,
			DATA_FORMAT_R32_SFLOAT,
			DATA_FORMAT_R32G32_UINT,
			DATA_FORMAT_R32G32_SINT,
			DATA_FORMAT_R32G32_SFLOAT,
			DATA_FORMAT_R32G32B32_UINT,
			DATA_FORMAT_R32G32B32_SINT,
			DATA_FORMAT_R32G32B32_SFLOAT,
			DATA_FORMAT_R32G32B32A32_UINT,
			DATA_FORMAT_R32G32B32A32_SINT,
			DATA_FORMAT_R32G32B32A32_SFLOAT,
			DATA_FORMAT_R64_UINT,
			DATA_FORMAT_R64_SINT,
			DATA_FORMAT_R64_SFLOAT,
			DATA_FORMAT_R64G64_UINT,
			DATA_FORMAT_R64G64_SINT,
			DATA_FORMAT_R64G64_SFLOAT,
			DATA_FORMAT_R64G64B64_UINT,
			DATA_FORMAT_R64G64B64_SINT,
			DATA_FORMAT_R64G64B64_SFLOAT,
			DATA_FORMAT_R64G64B64A64_UINT,
			DATA_FORMAT_R64G64B64A64_SINT,
			DATA_FORMAT_R64G64B64A64_SFLOAT,
			DATA_FORMAT_B10G11R11_UFLOAT_PACK32,
			DATA_FORMAT_E5B9G9R9_UFLOAT_PACK32,
			DATA_FORMAT_D16_UNORM,
			DATA_FORMAT_X8_D24_UNORM_PACK32,
			DATA_FORMAT_D32_SFLOAT,
			DATA_FORMAT_S8_UINT,
			DATA_FORMAT_D16_UNORM_S8_UINT,
			DATA_FORMAT_D24_UNORM_S8_UINT,
			DATA_FORMAT_D32_SFLOAT_S8_UINT,
			DATA_FORMAT_BC1_RGB_UNORM_BLOCK,
			DATA_FORMAT_BC1_RGB_SRGB_BLOCK,
			DATA_FORMAT_BC1_RGBA_UNORM_BLOCK,
			DATA_FORMAT_BC1_RGBA_SRGB_BLOCK,
			DATA_FORMAT_BC2_UNORM_BLOCK,
			DATA_FORMAT_BC2_SRGB_BLOCK,
			DATA_FORMAT_BC3_UNORM_BLOCK,
			DATA_FORMAT_BC3_SRGB_BLOCK,
			DATA_FORMAT_BC4_UNORM_BLOCK,
			DATA_FORMAT_BC4_SNORM_BLOCK,
			DATA_FORMAT_BC5_UNORM_BLOCK,
			DATA_FORMAT_BC5_SNORM_BLOCK,
			DATA_FORMAT_BC6H_UFLOAT_BLOCK,
			DATA_FORMAT_BC6H_SFLOAT_BLOCK,
			DATA_FORMAT_BC7_UNORM_BLOCK,
			DATA_FORMAT_BC7_SRGB_BLOCK,
			DATA_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
			DATA_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
			DATA_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
			DATA_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
			DATA_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
			DATA_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
			DATA_FORMAT_EAC_R11_UNORM_BLOCK,
			DATA_FORMAT_EAC_R11_SNORM_BLOCK,
			DATA_FORMAT_EAC_R11G11_UNORM_BLOCK,
			DATA_FORMAT_EAC_R11G11_SNORM_BLOCK,
			DATA_FORMAT_ASTC_4x4_UNORM_BLOCK,
			DATA_FORMAT_ASTC_4x4_SRGB_BLOCK,
			DATA_FORMAT_ASTC_5x4_UNORM_BLOCK,
			DATA_FORMAT_ASTC_5x4_SRGB_BLOCK,
			DATA_FORMAT_ASTC_5x5_UNORM_BLOCK,
			DATA_FORMAT_ASTC_5x5_SRGB_BLOCK,
			DATA_FORMAT_ASTC_6x5_UNORM_BLOCK,
			DATA_FORMAT_ASTC_6x5_SRGB_BLOCK,
			DATA_FORMAT_ASTC_6x6_UNORM_BLOCK,
			DATA_FORMAT_ASTC_6x6_SRGB_BLOCK,
			DATA_FORMAT_ASTC_8x5_UNORM_BLOCK,
			DATA_FORMAT_ASTC_8x5_SRGB_BLOCK,
			DATA_FORMAT_ASTC_8x6_UNORM_BLOCK,
			DATA_FORMAT_ASTC_8x6_SRGB_BLOCK,
			DATA_FORMAT_ASTC_8x8_UNORM_BLOCK,
			DATA_FORMAT_ASTC_8x8_SRGB_BLOCK,
			DATA_FORMAT_ASTC_10x5_UNORM_BLOCK,
			DATA_FORMAT_ASTC_10x5_SRGB_BLOCK,
			DATA_FORMAT_ASTC_10x6_UNORM_BLOCK,
			DATA_FORMAT_ASTC_10x6_SRGB_BLOCK,
			DATA_FORMAT_ASTC_10x8_UNORM_BLOCK,
			DATA_FORMAT_ASTC_10x8_SRGB_BLOCK,
			DATA_FORMAT_ASTC_10x10_UNORM_BLOCK,
			DATA_FORMAT_ASTC_10x10_SRGB_BLOCK,
			DATA_FORMAT_ASTC_12x10_UNORM_BLOCK,
			DATA_FORMAT_ASTC_12x10_SRGB_BLOCK,
			DATA_FORMAT_ASTC_12x12_UNORM_BLOCK,
			DATA_FORMAT_ASTC_12x12_SRGB_BLOCK,
			DATA_FORMAT_G8B8G8R8_422_UNORM,
			DATA_FORMAT_B8G8R8G8_422_UNORM,
			DATA_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
			DATA_FORMAT_G8_B8R8_2PLANE_420_UNORM,
			DATA_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
			DATA_FORMAT_G8_B8R8_2PLANE_422_UNORM,
			DATA_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
			DATA_FORMAT_R10X6_UNORM_PACK16,
			DATA_FORMAT_R10X6G10X6_UNORM_2PACK16,
			DATA_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
			DATA_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
			DATA_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
			DATA_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
			DATA_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
			DATA_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
			DATA_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
			DATA_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
			DATA_FORMAT_R12X4_UNORM_PACK16,
			DATA_FORMAT_R12X4G12X4_UNORM_2PACK16,
			DATA_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
			DATA_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
			DATA_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
			DATA_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
			DATA_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
			DATA_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
			DATA_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
			DATA_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
			DATA_FORMAT_G16B16G16R16_422_UNORM,
			DATA_FORMAT_B16G16R16G16_422_UNORM,
			DATA_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
			DATA_FORMAT_G16_B16R16_2PLANE_420_UNORM,
			DATA_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
			DATA_FORMAT_G16_B16R16_2PLANE_422_UNORM,
			DATA_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
			DATA_FORMAT_MAX,
		};

		enum CompareOperator {
			COMPARE_OP_NEVER,
			COMPARE_OP_LESS,
			COMPARE_OP_EQUAL,
			COMPARE_OP_LESS_OR_EQUAL,
			COMPARE_OP_GREATER,
			COMPARE_OP_NOT_EQUAL,
			COMPARE_OP_GREATER_OR_EQUAL,
			COMPARE_OP_ALWAYS,
			COMPARE_OP_MAX
		};

	/*****************/
	/**** TEXTURE ****/
	/*****************/
		// texture2d array和3d还是有所不同的
		enum TextureType {
			TEXTURE_TYPE_1D,
			TEXTURE_TYPE_2D,
			TEXTURE_TYPE_3D,
			TEXTURE_TYPE_CUBE,
			TEXTURE_TYPE_1D_ARRAY,
			TEXTURE_TYPE_2D_ARRAY,
			TEXTURE_TYPE_CUBE_ARRAY,
			TEXTURE_TYPE_MAX,
		};
		// msaa usage?
		enum TextureSamples {
			TEXTURE_SAMPLES_1 = 1,
			TEXTURE_SAMPLES_2 = 2,
			TEXTURE_SAMPLES_4 = 4,
			TEXTURE_SAMPLES_8 = 8,
			TEXTURE_SAMPLES_16 = 16,
			TEXTURE_SAMPLES_32 = 32,
			TEXTURE_SAMPLES_64 = 64,
			TEXTURE_SAMPLES_MAX,
		};
		// VK_IMAGE_USAGE_TRANSFER_SRC_BIT -> TEXTURE_USAGE_CAN_COPY_FROM_BIT, see texture_create
		// VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR -> TEXTURE_USAGE_VRS_ATTACHMENT_BIT
		// 可变着色率
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageUsageFlagBits.html
		ENUM(TextureMisc, Enable) {
			TEXTURE_MISC_GENERATE_MIPS_BIT = 1 << 0,
			TEXTURE_MISC_FORCE_ARRAY_BIT = 1 << 1,
			TEXTURE_MISC_MUTABLE_SRGB_BIT = 1 << 2,
			TEXTURE_MISC_CONCURRENT_QUEUE_GRAPHICS_BIT = 1 << 3,
			TEXTURE_MISC_CONCURRENT_QUEUE_ASYNC_COMPUTE_BIT = 1 << 4,
			TEXTURE_MISC_CONCURRENT_QUEUE_ASYNC_TRANSFER_BIT = 1 << 6,
			TEXTURE_MISC_CONCURRENT_QUEUE_VIDEO_DECODE_BIT = 1 << 7,
			TEXTURE_MISC_VERIFY_FORMAT_FEATURE_SAMPLED_LINEAR_FILTER_BIT = 1 << 8,
			TEXTURE_MISC_LINEAR_IMAGE_IGNORE_DEVICE_LOCAL_BIT = 1 << 9,
			TEXTURE_MISC_FORCE_NO_DEDICATED_BIT = 1 << 10,
			TEXTURE_MISC_NO_DEFAULT_VIEWS_BIT = 1 << 11,
			TEXTURE_MISC_EXTERNAL_MEMORY_BIT = 1 << 12,
			TEXTURE_MISC_CONCURRENT_QUEUE_VIDEO_ENCODE_BIT = 1 << 13,
			TEXTURE_MISC_CONCURRENT_QUEUE_VIDEO_DUPLEX =
			TEXTURE_MISC_CONCURRENT_QUEUE_VIDEO_DECODE_BIT |
			TEXTURE_MISC_CONCURRENT_QUEUE_VIDEO_ENCODE_BIT,
		};

		ENUM(TextureUsageBits, Enable) {
			TEXTURE_USAGE_SAMPLING_BIT = (1 << 0),
			TEXTURE_USAGE_COLOR_ATTACHMENT_BIT = (1 << 1),
			TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT = (1 << 2),
			TEXTURE_USAGE_STORAGE_BIT = (1 << 3),
			TEXTURE_USAGE_STORAGE_ATOMIC_BIT = (1 << 4),
			TEXTURE_USAGE_CPU_READ_BIT = (1 << 5),
			TEXTURE_USAGE_CAN_UPDATE_BIT = (1 << 6),
			TEXTURE_USAGE_CAN_COPY_FROM_BIT = (1 << 7),
			TEXTURE_USAGE_CAN_COPY_TO_BIT = (1 << 8),
			TEXTURE_USAGE_INPUT_ATTACHMENT_BIT = (1 << 9),
			TEXTURE_USAGE_VRS_ATTACHMENT_BIT = (1 << 10),
		};

		struct TextureFormat {
			DataFormat format = DATA_FORMAT_R8_UNORM;
			uint32_t width = 1;
			uint32_t height = 1;
			uint32_t depth = 1;
			uint32_t array_layers = 1;
			uint32_t mipmaps = 1;
			TextureType texture_type = TEXTURE_TYPE_2D;
			TextureSamples samples = TEXTURE_SAMPLES_1;
			uint32_t usage_bits = 0; // 基本是TextureUsageBits
			BitField<TextureMisc> misc;
			Vector<DataFormat> shareable_formats;
			bool is_resolve_buffer = false;

			bool operator==(const TextureFormat& b) const {
				return format == b.format &&
					width == b.width &&
					height == b.height &&
					depth == b.depth &&
					array_layers == b.array_layers &&
					mipmaps == b.mipmaps &&
					texture_type == b.texture_type &&
					samples == b.samples &&
					usage_bits == b.usage_bits &&
					misc == b.misc &&
					shareable_formats == b.shareable_formats;
			}
		};
		// same with vulkan VkComponentSwizzle 
		enum TextureSwizzle {
			TEXTURE_SWIZZLE_IDENTITY,
			TEXTURE_SWIZZLE_ZERO,
			TEXTURE_SWIZZLE_ONE,
			TEXTURE_SWIZZLE_R,
			TEXTURE_SWIZZLE_G,
			TEXTURE_SWIZZLE_B,
			TEXTURE_SWIZZLE_A,
			TEXTURE_SWIZZLE_MAX
		};
		// 目前支持的切片类型
		enum TextureSliceType {
			TEXTURE_SLICE_2D,
			TEXTURE_SLICE_CUBEMAP,
			TEXTURE_SLICE_3D,
			TEXTURE_SLICE_2D_ARRAY,
			TEXTURE_SLICE_MAX
		};
		/*****************/
		/**** SAMPLER ****/
		/*****************/
		enum SamplerFilter {
			SAMPLER_FILTER_NEAREST,
			SAMPLER_FILTER_LINEAR,
			SAMPLER_FILTER_CUBIC, // ext
		};

		enum SamplerRepeatMode {
			SAMPLER_REPEAT_MODE_REPEAT,
			SAMPLER_REPEAT_MODE_MIRRORED_REPEAT,
			SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE,
			SAMPLER_REPEAT_MODE_CLAMP_TO_BORDER,
			SAMPLER_REPEAT_MODE_MIRROR_CLAMP_TO_EDGE,
			SAMPLER_REPEAT_MODE_MAX
		};

		enum SamplerBorderColor {
			SAMPLER_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
			SAMPLER_BORDER_COLOR_INT_TRANSPARENT_BLACK,
			SAMPLER_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
			SAMPLER_BORDER_COLOR_INT_OPAQUE_BLACK,
			SAMPLER_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			SAMPLER_BORDER_COLOR_INT_OPAQUE_WHITE,
			SAMPLER_BORDER_COLOR_MAX
		};

		struct SamplerState {
			SamplerFilter mag_filter = SAMPLER_FILTER_NEAREST; // sample footprint is smaller than a pixel
			SamplerFilter min_filter = SAMPLER_FILTER_NEAREST; // sample footprint is larger than a pixel 
			SamplerFilter mip_filter = SAMPLER_FILTER_NEAREST;  // between two mipmap
			SamplerRepeatMode repeat_u = SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE;
			SamplerRepeatMode repeat_v = SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE;
			SamplerRepeatMode repeat_w = SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE;
			float lod_bias = 0.0f;
			bool use_anisotropy = false;
			float anisotropy_max = 1.0f;
			bool enable_compare = false;
			CompareOperator compare_op = COMPARE_OP_ALWAYS;
			float min_lod = 0.0f;
			float max_lod = static_cast<float>(1e20); // Something very large should do.
			SamplerBorderColor border_color = SAMPLER_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			bool unnormalized_uvw = false;
		};

		/**********************/
		/**** VERTEX ARRAY ****/
		/**********************/

		enum IndexBufferFormat {
			INDEX_BUFFER_FORMAT_UINT16,
			INDEX_BUFFER_FORMAT_UINT32,
		};

		enum VertexFrequency {
			VERTEX_FREQUENCY_VERTEX,
			VERTEX_FREQUENCY_INSTANCE,
		};

		struct VertexAttribute {
			uint32_t location = 0; // Shader location.
			uint32_t offset = 0;
			DataFormat format = DATA_FORMAT_MAX;
			uint32_t stride = 0;
			VertexFrequency frequency = VERTEX_FREQUENCY_VERTEX;
		};

		/*********************/
		/**** FRAMEBUFFER ****/
		/*********************/

		static const int32_t ATTACHMENT_UNUSED = -1;

		/****************/
		/**** SHADER ****/
		/****************/
		
		ENUM(ShaderStage, Disable) {
			SHADER_STAGE_VERTEX,
			SHADER_STAGE_FRAGMENT,
			SHADER_STAGE_TESSELATION_CONTROL,
			SHADER_STAGE_TESSELATION_EVALUATION,
			SHADER_STAGE_COMPUTE,
			SHADER_STAGE_MAX,
			SHADER_STAGE_VERTEX_BIT = (1 << SHADER_STAGE_VERTEX),
			SHADER_STAGE_FRAGMENT_BIT = (1 << SHADER_STAGE_FRAGMENT),
			SHADER_STAGE_TESSELATION_CONTROL_BIT = (1 << SHADER_STAGE_TESSELATION_CONTROL),
			SHADER_STAGE_TESSELATION_EVALUATION_BIT = (1 << SHADER_STAGE_TESSELATION_EVALUATION),
			SHADER_STAGE_COMPUTE_BIT = (1 << SHADER_STAGE_COMPUTE),
			
		};


		struct ShaderStageSPIRVData {
			ShaderStage shader_stage = SHADER_STAGE_MAX;
			Vector<uint8_t> spirv;
		};

		/*********************/
		/**** UNIFORM SET ****/
		/*********************/

		static const uint32_t MAX_UNIFORM_SETS = 16;
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorType.html
		enum UniformType {
			UNIFORM_TYPE_SAMPLER, // For sampling only (sampler GLSL type).
			UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, // For sampling only, but includes a texture, (samplerXX GLSL type), first a sampler then a texture.
			UNIFORM_TYPE_TEXTURE, // Only texture, (textureXX GLSL type). // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
			UNIFORM_TYPE_IMAGE, // Storage image (imageXX GLSL type), for compute mostly. // VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
			UNIFORM_TYPE_TEXTURE_BUFFER, // Buffer texture (or TBO, textureBuffer type).
			UNIFORM_TYPE_SAMPLER_WITH_TEXTURE_BUFFER, // Buffer texture with a sampler(or TBO, samplerBuffer type).
			UNIFORM_TYPE_IMAGE_BUFFER, // Texel buffer, (imageBuffer type), for compute mostly.
			UNIFORM_TYPE_UNIFORM_BUFFER, // Regular uniform buffer (or UBO). 【小型只读数据】
			UNIFORM_TYPE_DYNAMIC_UNIFORM_BUFFER, // 【dynamic】
			UNIFORM_TYPE_STORAGE_BUFFER, // Storage buffer ("buffer" qualifier) like UBO, but supports storage, for compute mostly.【不知道大小，或者是可写的数据，整个场景填充到一个缓冲区中】
			UNIFORM_TYPE_INPUT_ATTACHMENT, // Used for sub-pass read/write, for mobile mostly.
			UNIFORM_TYPE_ACCELERATION_STRUCTURE, // Used for accelerationStructureEXT 

			UNIFORM_TYPE_MAX
		};
		// array?
		/******************/
		/**** PIPELINE ****/
		/******************/
		// https://docs.vulkan.org/samples/latest/samples/performance/specialization_constants/README.html
		// 编译器静态展开
		enum PipelineSpecializationConstantType {
			PIPELINE_SPECIALIZATION_CONSTANT_TYPE_BOOL,
			PIPELINE_SPECIALIZATION_CONSTANT_TYPE_INT,
			PIPELINE_SPECIALIZATION_CONSTANT_TYPE_FLOAT,
		};
		struct PipelineSpecializationConstant {
			PipelineSpecializationConstantType type = {};
			uint32_t constant_id = 0xffffffff;
			union {
				uint32_t int_value = 0;
				float float_value;
				bool bool_value;
			};
		};
		/*******************/
		/**** RENDERING ****/
		/*******************/
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPrimitiveTopology.html
		enum RenderPrimitive {
			RENDER_PRIMITIVE_POINTS,
			RENDER_PRIMITIVE_LINES,
			RENDER_PRIMITIVE_LINES_WITH_ADJACENCY,
			RENDER_PRIMITIVE_LINESTRIPS,
			RENDER_PRIMITIVE_LINESTRIPS_WITH_ADJACENCY,
			RENDER_PRIMITIVE_TRIANGLES,
			RENDER_PRIMITIVE_TRIANGLES_WITH_ADJACENCY,
			RENDER_PRIMITIVE_TRIANGLE_STRIPS,
			RENDER_PRIMITIVE_TRIANGLE_STRIPS_WITH_AJACENCY,
			RENDER_PRIMITIVE_TRIANGLE_STRIPS_WITH_RESTART_INDEX,
			RENDER_PRIMITIVE_TESSELATION_PATCH,
			RENDER_PRIMITIVE_MAX
		};
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCullModeFlagBits.html
		enum PolygonCullMode {
			POLYGON_CULL_DISABLED,
			POLYGON_CULL_FRONT,
			POLYGON_CULL_BACK,
			POLYGON_CULL_FRONT_AND_BACK,
			POLYGON_CULL_MAX
		};
		enum PolygonFrontFace {
			POLYGON_FRONT_FACE_CLOCKWISE,
			POLYGON_FRONT_FACE_COUNTER_CLOCKWISE,
		};
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkStencilOp.html
		enum StencilOperation {
			STENCIL_OP_KEEP,
			STENCIL_OP_ZERO,
			STENCIL_OP_REPLACE,
			STENCIL_OP_INCREMENT_AND_CLAMP,
			STENCIL_OP_DECREMENT_AND_CLAMP,
			STENCIL_OP_INVERT,
			STENCIL_OP_INCREMENT_AND_WRAP,
			STENCIL_OP_DECREMENT_AND_WRAP,
			STENCIL_OP_MAX
		};
		enum StencilFace {
			STENCIL_FACE_FRONT_BIT = 0x00000001,
			STENCIL_FACE_BACK_BIT = 0x00000002,
			STENCIL_FACE_FRONT_AND_BACK = 0x00000003,
			STENCIL_FACE_MAX
		};
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkLogicOp.html
		enum LogicOperation {
			LOGIC_OP_CLEAR,
			LOGIC_OP_AND,
			LOGIC_OP_AND_REVERSE,
			LOGIC_OP_COPY,
			LOGIC_OP_AND_INVERTED,
			LOGIC_OP_NO_OP,
			LOGIC_OP_XOR,
			LOGIC_OP_OR,
			LOGIC_OP_NOR,
			LOGIC_OP_EQUIVALENT,
			LOGIC_OP_INVERT,
			LOGIC_OP_OR_REVERSE,
			LOGIC_OP_COPY_INVERTED,
			LOGIC_OP_OR_INVERTED,
			LOGIC_OP_NAND,
			LOGIC_OP_SET,
			LOGIC_OP_MAX
		};
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBlendFactor.html
		enum BlendFactor {
			BLEND_FACTOR_ZERO,
			BLEND_FACTOR_ONE,
			BLEND_FACTOR_SRC_COLOR,
			BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
			BLEND_FACTOR_DST_COLOR,
			BLEND_FACTOR_ONE_MINUS_DST_COLOR,
			BLEND_FACTOR_SRC_ALPHA,
			BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			BLEND_FACTOR_DST_ALPHA,
			BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
			BLEND_FACTOR_CONSTANT_COLOR,
			BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
			BLEND_FACTOR_CONSTANT_ALPHA,
			BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
			BLEND_FACTOR_SRC_ALPHA_SATURATE,
			BLEND_FACTOR_SRC1_COLOR,
			BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
			BLEND_FACTOR_SRC1_ALPHA,
			BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
			BLEND_FACTOR_MAX
		};
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBlendOp.html
		enum BlendOperation {
			BLEND_OP_ADD,
			BLEND_OP_SUBTRACT,
			BLEND_OP_REVERSE_SUBTRACT,
			BLEND_OP_MINIMUM,
			BLEND_OP_MAXIMUM, // Yes, this one is an actual operator.
			BLEND_OP_MAX
		};
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineRasterizationStateCreateInfo.html
		struct PipelineRasterizationState {
			bool enable_depth_clamp = false;
			// clamp，在Vieport transform之后进行。
			bool discard_primitives = false;
			// 是否在光栅化阶段之前立即丢弃图元。
			bool wireframe = false;
			// Polygon Mode is fill/ line
			PolygonCullMode cull_mode = POLYGON_CULL_DISABLED;
			PolygonFrontFace front_face = POLYGON_FRONT_FACE_CLOCKWISE;
			bool depth_bias_enabled = false; // bias fragment depth values
			float depth_bias_constant_factor = 0.0f; // scalar factor controlling the constant depth value added to each fragment
			float depth_bias_clamp = 0.0f;
			float depth_bias_slope_factor = 0.0f; // applied to a fragment’s slope in depth bias calculations
			float line_width = 1.0f; // width of rasterized line segments
			uint32_t patch_control_points = 1; // 在曲面细分启用时有用
		};
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineMultisampleStateCreateInfo.html
		struct PipelineMultisampleState {
			TextureSamples sample_count = TEXTURE_SAMPLES_1; // rasterizationSamples
			bool enable_sample_shading = false; // sample Shading
			float min_sample_shading = 0.0f;
			Vector<uint32_t> sample_mask; //an array of VkSampleMask values used in the sample mask test.
			bool enable_alpha_to_coverage = false;
			bool enable_alpha_to_one = false;
		};

		struct PipelineDepthStencilState {
			bool enable_depth_test = false;
			bool enable_depth_write = false;
			CompareOperator depth_compare_operator = COMPARE_OP_ALWAYS;
			bool enable_depth_range = false; // depth bound test
			float depth_range_min = 0;
			float depth_range_max = 0;
			bool enable_stencil = false;

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkStencilOpState.html
			struct StencilOperationState {
				StencilOperation fail = STENCIL_OP_ZERO;
				StencilOperation pass = STENCIL_OP_ZERO;
				StencilOperation depth_fail = STENCIL_OP_ZERO; // 指定对通过模板测试但未通过深度测试的样本执行的操作的值。
				CompareOperator compare = COMPARE_OP_ALWAYS;
				uint32_t compare_mask = 0; // CompareMask 选择参与模板测试的无符号整数模板值的位
				uint32_t write_mask = 0; // 选择由模板帧缓冲区附件中的模板测试更新的无符号整数模板值的位。
				uint32_t reference = 0; // reference value that is used in the unsigned stencil comparison.
			};

			StencilOperationState front_op;
			StencilOperationState back_op;
		};

		struct PipelineColorBlendState {
			bool enable_logic_op = false;
			LogicOperation logic_op = LOGIC_OP_CLEAR;
			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineColorBlendAttachmentState.html
			// color blend attachment
			struct Attachment {
				bool enable_blend = false;
				BlendFactor src_color_blend_factor = BLEND_FACTOR_ZERO;
				BlendFactor dst_color_blend_factor = BLEND_FACTOR_ZERO;
				BlendOperation color_blend_op = BLEND_OP_ADD;
				BlendFactor src_alpha_blend_factor = BLEND_FACTOR_ZERO;
				BlendFactor dst_alpha_blend_factor = BLEND_FACTOR_ZERO;
				BlendOperation alpha_blend_op = BLEND_OP_ADD;
				bool write_r = true;
				bool write_g = true;
				bool write_b = true;
				bool write_a = true;
			};

			static PipelineColorBlendState create_disabled(int p_attachments = 1) {
				PipelineColorBlendState bs;
				for (int i = 0; i < p_attachments; i++) {
					bs.attachments.push_back(Attachment());
				}
				return bs;
			}

			static PipelineColorBlendState create_blend(int p_attachments = 1) {
				PipelineColorBlendState bs;
				for (int i = 0; i < p_attachments; i++) {
					Attachment ba;
					ba.enable_blend = true;
					ba.src_color_blend_factor = BLEND_FACTOR_SRC_ALPHA;
					ba.dst_color_blend_factor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					ba.src_alpha_blend_factor = BLEND_FACTOR_SRC_ALPHA;
					ba.dst_alpha_blend_factor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

					bs.attachments.push_back(ba);
				}
				return bs;
			}

			Vector<Attachment> attachments; // One per render target texture.
			// VkPipelineColorBlendAttachmentState
			Color blend_constant;
		};
		// 动态管线就是说在createinfo设置中的对应值会被忽略，必须使用cmdset动态指定
		// vulkan 特有功能
		enum PipelineDynamicStateFlags {
			DYNAMIC_STATE_LINE_WIDTH = (1 << 0),
			DYNAMIC_STATE_DEPTH_BIAS = (1 << 1),
			DYNAMIC_STATE_BLEND_CONSTANTS = (1 << 2),
			DYNAMIC_STATE_DEPTH_BOUNDS = (1 << 3),
			DYNAMIC_STATE_STENCIL_COMPARE_MASK = (1 << 4),
			DYNAMIC_STATE_STENCIL_WRITE_MASK = (1 << 5),
			DYNAMIC_STATE_STENCIL_REFERENCE = (1 << 6),
			DYNAMIC_STATE_CULL_MODE = (1<<7),
			DYNAMIC_STATE_FRONT_FACE = (1 << 8),	
			DYNAMIC_STATE_PRIMITIVE_TOPOLOGY = (1<<9),
			DYNAMIC_STATE_VIEWPORT_WITH_COUNT = (1 << 10),
			DYNAMIC_STATE_SCISSOR_WITH_COUNT = (1 << 11),
			DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE = (1 << 12),
			DYNAMIC_STATE_DEPTH_TEST_ENABLE = (1 << 13),
			DYNAMIC_STATE_DEPTH_WRITE_ENABLE = (1 << 14),
			DYNAMIC_STATE_DEPTH_COMPARE_OP = (1 << 15),
			DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE = (1 << 16),
			DYNAMIC_STATE_STENCIL_TEST_ENABLE = (1 << 17),
			DYNAMIC_STATE_STENCIL_OP = (1 << 18),
			DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE = (1 << 19),
			DYNAMIC_STATE_DEPTH_BIAS_ENABLE = (1 << 20),
			DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE = (1 << 21),
		};


		// This enum matches VkPhysicalDeviceType (except for `DEVICE_TYPE_MAX`).
	// Unlike VkPhysicalDeviceType, DeviceType is exposed to the scripting API.
		enum DeviceType {
			DEVICE_TYPE_OTHER,
			DEVICE_TYPE_INTEGRATED_GPU,
			DEVICE_TYPE_DISCRETE_GPU,
			DEVICE_TYPE_VIRTUAL_GPU,
			DEVICE_TYPE_CPU,
			DEVICE_TYPE_MAX
		};
		// 一种与底层API无关的方式定义的底层资源
		// Defined in an API-agnostic way.
	// Some may not make sense for the underlying API; in that case, 0 is returned.
		enum DriverResource {
			DRIVER_RESOURCE_LOGICAL_DEVICE,
			DRIVER_RESOURCE_PHYSICAL_DEVICE,
			DRIVER_RESOURCE_TOPMOST_OBJECT,
			DRIVER_RESOURCE_COMMAND_QUEUE,
			DRIVER_RESOURCE_QUEUE_FAMILY,
			DRIVER_RESOURCE_TEXTURE,
			DRIVER_RESOURCE_TEXTURE_VIEW,
			DRIVER_RESOURCE_TEXTURE_DATA_FORMAT,
			DRIVER_RESOURCE_SAMPLER,
			DRIVER_RESOURCE_UNIFORM_SET,
			DRIVER_RESOURCE_BUFFER,
			DRIVER_RESOURCE_COMPUTE_PIPELINE,
			DRIVER_RESOURCE_RENDER_PIPELINE,
		};
		enum Limit {
			LIMIT_MAX_BOUND_UNIFORM_SETS,
			LIMIT_MAX_FRAMEBUFFER_COLOR_ATTACHMENTS,
			LIMIT_MAX_TEXTURES_PER_UNIFORM_SET,
			LIMIT_MAX_SAMPLERS_PER_UNIFORM_SET,
			LIMIT_MAX_STORAGE_BUFFERS_PER_UNIFORM_SET,
			LIMIT_MAX_STORAGE_IMAGES_PER_UNIFORM_SET,
			LIMIT_MAX_UNIFORM_BUFFERS_PER_UNIFORM_SET,
			LIMIT_MAX_DRAW_INDEXED_INDEX,
			LIMIT_MAX_FRAMEBUFFER_HEIGHT,
			LIMIT_MAX_FRAMEBUFFER_WIDTH,
			LIMIT_MAX_TEXTURE_ARRAY_LAYERS,
			LIMIT_MAX_TEXTURE_SIZE_1D,
			LIMIT_MAX_TEXTURE_SIZE_2D,
			LIMIT_MAX_TEXTURE_SIZE_3D,
			LIMIT_MAX_TEXTURE_SIZE_CUBE,
			LIMIT_MAX_TEXTURES_PER_SHADER_STAGE,
			LIMIT_MAX_SAMPLERS_PER_SHADER_STAGE,
			LIMIT_MAX_STORAGE_BUFFERS_PER_SHADER_STAGE,
			LIMIT_MAX_STORAGE_IMAGES_PER_SHADER_STAGE,
			LIMIT_MAX_UNIFORM_BUFFERS_PER_SHADER_STAGE,
			LIMIT_MAX_PUSH_CONSTANT_SIZE,
			LIMIT_MAX_UNIFORM_BUFFER_SIZE,
			LIMIT_MAX_VERTEX_INPUT_ATTRIBUTE_OFFSET,
			LIMIT_MAX_VERTEX_INPUT_ATTRIBUTES,
			LIMIT_MAX_VERTEX_INPUT_BINDINGS,
			LIMIT_MAX_VERTEX_INPUT_BINDING_STRIDE,
			LIMIT_MIN_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
			LIMIT_MAX_COMPUTE_SHARED_MEMORY_SIZE,
			LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_X,
			LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Y,
			LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Z,
			LIMIT_MAX_COMPUTE_WORKGROUP_INVOCATIONS,
			LIMIT_MAX_COMPUTE_WORKGROUP_SIZE_X,
			LIMIT_MAX_COMPUTE_WORKGROUP_SIZE_Y,
			LIMIT_MAX_COMPUTE_WORKGROUP_SIZE_Z,
			LIMIT_MAX_VIEWPORT_DIMENSIONS_X,
			LIMIT_MAX_VIEWPORT_DIMENSIONS_Y,
			LIMIT_SUBGROUP_SIZE,
			LIMIT_SUBGROUP_MIN_SIZE,
			LIMIT_SUBGROUP_MAX_SIZE,
			LIMIT_SUBGROUP_IN_SHADERS, // Set flags using SHADER_STAGE_VERTEX_BIT, SHADER_STAGE_FRAGMENT_BIT, etc.
			LIMIT_SUBGROUP_OPERATIONS,
			LIMIT_VRS_TEXEL_WIDTH,
			LIMIT_VRS_TEXEL_HEIGHT,
		};

		enum Features {
			SUPPORTS_MULTIVIEW,
			SUPPORTS_FSR_HALF_FLOAT, // amd fsr
			SUPPORTS_ATTACHMENT_VRS,
			// If not supported, a fragment shader with only side effets (i.e., writes  to buffers, but doesn't output to attachments), may be optimized down to no-op by the GPU driver.
			SUPPORTS_FRAGMENT_SHADER_WITH_ONLY_SIDE_EFFECTS,
		};

		// subgroup允许共享数据
		enum SubgroupOperations {
			SUBGROUP_BASIC_BIT = 1,
			SUBGROUP_VOTE_BIT = 2,
			SUBGROUP_ARITHMETIC_BIT = 4,
			SUBGROUP_BALLOT_BIT = 8,
			SUBGROUP_SHUFFLE_BIT = 16,
			SUBGROUP_SHUFFLE_RELATIVE_BIT = 32,
			SUBGROUP_CLUSTERED_BIT = 64,
			SUBGROUP_QUAD_BIT = 128,
		};

protected:
	// RHI
	/*****************/
	/**** GENERIC ****/
	/*****************/

	static const char* const FORMAT_NAMES[DATA_FORMAT_MAX];

	/*****************/
	/**** TEXTURE ****/
	/*****************/

	static const uint32_t MAX_IMAGE_FORMAT_PLANES = 2;

	static const uint32_t TEXTURE_SAMPLES_COUNT[TEXTURE_SAMPLES_MAX];

	static uint32_t get_image_format_pixel_size(DataFormat p_format);
	static void get_compressed_image_format_block_dimensions(DataFormat p_format, uint32_t& r_w, uint32_t& r_h);
	uint32_t get_compressed_image_format_block_byte_size(DataFormat p_format);
	static uint32_t get_compressed_image_format_pixel_rshift(DataFormat p_format);
	static uint32_t get_image_format_required_size(DataFormat p_format, uint32_t p_width, uint32_t p_height, uint32_t p_depth, uint32_t p_mipmaps, uint32_t* r_blockw = nullptr, uint32_t* r_blockh = nullptr, uint32_t* r_depth = nullptr);
	static uint32_t get_image_required_mipmaps(uint32_t p_width, uint32_t p_height, uint32_t p_depth);
	static bool format_has_stencil(DataFormat p_format);
	static uint32_t format_get_plane_count(DataFormat p_format);

	/*****************/
	/**** SAMPLER ****/
	/*****************/

	static const Color SAMPLER_BORDER_COLOR_VALUE[SAMPLER_BORDER_COLOR_MAX];

	/**********************/
	/**** VERTEX ARRAY ****/
	/**********************/

	static uint32_t get_format_vertex_size(DataFormat p_format);

	/****************/
	/**** SHADER ****/
	/****************/

	static const char* SHADER_STAGE_NAMES[SHADER_STAGE_MAX];

public:
	// Shader Description
	struct ShaderUniform {
		UniformType type = UniformType::UNIFORM_TYPE_MAX;
		bool writable = false;
		uint32_t binding = 0;
		BitField<ShaderStage> stages;
		uint32_t length = 0; // Size of arrays (in total elements), or ubos (in bytes * total elements).

		bool operator!=(const ShaderUniform& p_other) const {
			return binding != p_other.binding || type != p_other.type || writable != p_other.writable || stages != p_other.stages || length != p_other.length;
		}

		bool operator<(const ShaderUniform& p_other) const {
			if (binding != p_other.binding) {
				return binding < p_other.binding;
			}
			if (type != p_other.type) {
				return type < p_other.type;
			}
			if (writable != p_other.writable) {
				return writable < p_other.writable;
			}
			if (stages != p_other.stages) {
				return stages < p_other.stages;
			}
			if (length != p_other.length) {
				return length < p_other.length;
			}
			return false;
		}
	};
	// 除了这里还有哪里需要管道特化常量？
	struct ShaderSpecializationConstant : public PipelineSpecializationConstant {
		BitField<ShaderStage> stages;

		bool operator<(const ShaderSpecializationConstant& p_other) const { return constant_id < p_other.constant_id; }
	};
	// @todo 加入光追的部分
	struct ShaderDescription{
		uint64_t vertex_input_mask = 0; // layout(location)的location
		uint32_t fragment_output_mask = 0;
		bool is_compute = false;
		uint32_t compute_local_size[3] = {};
		uint32_t push_constant_size = 0;

		Vector<Vector<ShaderUniform>> uniform_sets;
		Vector<ShaderSpecializationConstant> specialization_constants;
		Vector<ShaderStage> stages;
	};
	protected:
	struct ShaderReflection : public ShaderDescription {
		BitField<ShaderStage> stages;
		BitField<ShaderStage> push_constant_stages;
	}; // 可能说reflection的部分是个内部的东西，不应该暴露给用户

	};
	}// namespace graphic
} // namespace lain

#endif