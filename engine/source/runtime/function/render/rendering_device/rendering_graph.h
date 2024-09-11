#pragma once
#ifndef RENDERING_DEVICE_GRAPH
#define RENDERING_DEVICE_GRAPH
#include "core/typedefs.h"
#include "core/string/string_name.h"
#include "rendering_device_commons.h"
#include "rendering_device_driver.h"
#include "core/object/refcounted.h"
namespace lain {
		// 这里与RDD中的定义不同，基本都带着handle（指向内部需要数据结构的那个指针）
		// 建立于Driver上的上层数据结构
	class RenderGraph;
	// @?
	enum AttachmentInfoFlagBits
	{
		ATTACHMENT_INFO_PERSISTENT_BIT = 1 << 0,
		ATTACHMENT_INFO_UNORM_SRGB_ALIAS_BIT = 1 << 1,
		ATTACHMENT_INFO_SUPPORTS_PREROTATE_BIT = 1 << 2,
		ATTACHMENT_INFO_MIPGEN_BIT = 1 << 3,
		ATTACHMENT_INFO_INTERNAL_TRANSIENT_BIT = 1 << 16,
		ATTACHMENT_INFO_INTERNAL_PROXY_BIT = 1 << 17
	};

	enum SizeClass
	{
		Absolute,
		SwapchainRelative,
		InputRelative
	};

	struct BufferInfo {
		ui64 size;
		BitField<RDD::BufferUsageBits> usage;
		BitField<AttachmentInfoFlagBits> flags = ATTACHMENT_INFO_PERSISTENT_BIT;
		bool operator==(const BufferInfo& other) const
		{
			return size == other.size &&
				usage == other.usage &&
				flags == other.flags;
		}

		bool operator!=(const BufferInfo& other) const
		{
			return !(*this == other);
		}
	};

	struct AttachmentInfo
	{
		SizeClass size_class = SizeClass::SwapchainRelative;
		RDD::TextureFormat format;
		BitField<AttachmentInfoFlagBits> flags = ATTACHMENT_INFO_PERSISTENT_BIT;
		bool operator==(const AttachmentInfo& other) const
		{
			return size_class == other.size_class
				&& format == other.format
				&& flags == other.flags;
		}
		bool operator!=(const AttachmentInfo& other) const
		{
			return !(*this == other);
		}
	};

	class RenderResource : public RefCounted {
	public:
		
		
		struct ResourceDimensions
		{
			RDD::DataFormat format = RDD::DataFormat::DATA_FORMAT_MAX;
			BufferInfo buffer_info;
			unsigned width = 0;
			unsigned height = 0;
			unsigned depth = 1;
			unsigned layers = 1;
			unsigned levels = 1;
			unsigned samples = 1;
			BitField<AttachmentInfoFlagBits> flags = ATTACHMENT_INFO_PERSISTENT_BIT;
			//VkSurfaceTransformFlagBitsKHR transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
			BitField<RDD::CommandQueueFamilyBits> queues = 0;
			BitField<RDD::TextureUsageBits> image_usage = 0;

			bool operator==(const ResourceDimensions& other) const
			{
				return format == other.format &&
					width == other.width &&
					height == other.height &&
					depth == other.depth &&
					layers == other.layers &&
					levels == other.levels &&
					buffer_info == other.buffer_info &&
					flags == other.flags;
					/*transform == other.transform*/
				// image_usage is deliberately not part of this test.
				// queues is deliberately not part of this test.
			}

			bool operator!=(const ResourceDimensions& other) const
			{
				return !(*this == other);
			}

			bool uses_semaphore() const
			{
				if ((flags & ATTACHMENT_INFO_INTERNAL_PROXY_BIT) != 0)
					return true;

				// If more than one queue is used for a resource, we need to use semaphores.
				auto physical_queues = queues;

				// Regular compute uses regular graphics queue.
				if (physical_queues.has_flag(RDD::COMMAND_QUEUE_FAMILY_COMPUTE_BIT)) 
					physical_queues.set_flag(RDD::COMMAND_QUEUE_FAMILY_GRAPHICS_BIT);
				physical_queues.clear_flag(RDD::COMMAND_QUEUE_FAMILY_COMPUTE_BIT);
				return (physical_queues & (physical_queues - 1)) != 0;
			}

			bool is_storage_image() const
			{
				return image_usage.has_flag(RDD::TEXTURE_USAGE_STORAGE_BIT);
			}

			bool is_buffer_like() const
			{
				return is_storage_image() || (buffer_info.size != 0) || (flags.has_flag(ATTACHMENT_INFO_INTERNAL_PROXY_BIT));
			}

			StringName name;
		};


	
	public:
		enum class Type
		{
			Buffer,
			Texture,
			Proxy // 用于同步计算
		};

		enum { Unused = ~0u };

		
		RenderResource(Type p_type, unsigned p_index):resource_type(p_type), index(p_index){
		}
	private:
		Type resource_type;
		unsigned index; // 这个id就是暂时分配一个

		BitField<RDD::CommandQueueFamilyBits> used_queues;
		uint32_t physical_index = Unused; // 处理各种的alias情况
		StringName name;
	};
	using RR = RenderResource ;

	class RenderBufferResource : public RenderResource
	{
	public:
		explicit RenderBufferResource(unsigned index_)
			: RenderResource(RenderResource::Type::Buffer, index_)
		{
		}

		void set_buffer_info(const BufferInfo& info_)
		{
			info = info_;
		}

		const BufferInfo& get_buffer_info() const
		{
			return info;
		}
	private:
		BufferInfo info;
	};

	class RenderTextureResource : public RenderResource
	{
	public:
		explicit RenderTextureResource(unsigned index_)
			: RenderResource(RenderResource::Type::Texture, index_)
		{
		}

		void set_attachment_info(const AttachmentInfo& info_){info = info_;}

		const AttachmentInfo& get_attachment_info() const { return info; }

		AttachmentInfo& get_attachment_info(){
			return info;
		}

		void set_transient_state(bool enable){
			transient = enable;
		}

		bool get_transient_state() const{
			return transient;
		}
	private:
		AttachmentInfo info;
		BitField< RDD::TextureUsageBits> usage;
		bool transient = false; // transient attachments @?
	};


	
	/*class PhysicalRenderPass : RefCounted {
		
	private:
		RDD::RenderPassID handle;
		RDD::Framebuffeunsigned frame_buffer;
		RDD::RenderPassInfo info;
		uint32_t width = 0;
		uint32_t height = 0;
	};
	*/

	// 将被作为subpass
	class RenderPass{
	public:
		struct AccessedResource
		{
			BitField<RDD::PipelineStageBits> stages = 0;
			BitField<RDD::BarrierAccessBits> access = 0;
			BitField<RDD::TextureLayout> layout = RDD::TEXTURE_LAYOUT_UNDEFINED;
		};
		struct AccessedTextureResource : AccessedResource
		{
			RenderTextureResource* texture = nullptr;
		};

		struct AccessedBufferResource : AccessedResource
		{
			RenderBufferResource* buffer = nullptr;
		};

		struct AccessedProxyResource : AccessedResource
		{
			RenderResource* proxy = nullptr;
		};

		RenderTextureResource& set_depth_stencil_input(const StringName& name);
		RenderTextureResource& set_depth_stencil_output(const StringName& name, const AttachmentInfo& info);
		RenderTextureResource& add_color_output(const StringName& name, const AttachmentInfo& info, const StringName& input = "");
		RenderTextureResource& add_resolve_output(const StringName& name, const AttachmentInfo& info);
		RenderTextureResource& add_attachment_input(const StringName& name);
		RenderTextureResource& add_history_input(const StringName& name);
		RenderTextureResource& add_texture_input(const StringName& name,
			VkPipelineStageFlags2 stages = 0);
		RenderTextureResource& add_blit_texture_read_only_input(const StringName& name);
		RenderBufferResource& add_uniform_input(const StringName& name,
			VkPipelineStageFlags2 stages = 0);
		RenderBufferResource& add_storage_read_only_input(const StringName& name,
			VkPipelineStageFlags2 stages = 0);
		RenderBufferResource& add_storage_output(const StringName& name, const BufferInfo& info, const StringName& input = "");
		RenderBufferResource& add_transfer_output(const StringName& name, const BufferInfo& info);
		RenderTextureResource& add_storage_texture_output(const StringName& name, const AttachmentInfo& info, const StringName& input = "");
		RenderTextureResource& add_blit_texture_output(const StringName& name, const AttachmentInfo& info, const StringName& input = "");
		RenderBufferResource& add_vertex_buffer_input(const StringName& name);
		RenderBufferResource& add_index_buffer_input(const StringName& name);
		RenderBufferResource& add_indirect_buffer_input(const StringName& name);
		void add_proxy_output(const StringName& name, BitField<RDD::PipelineStageBits> stages);
		void add_proxy_input(const StringName& name, BitField<RDD::PipelineStageBits> stages);
		void add_fake_resource_write_alias(const StringName& from, const StringName& to);
		RenderBufferResource& add_generic_buffer_input(const StringName& name,
			BitField<RDD::PipelineStageBits> stages,
			BitField<RDD::BarrierAccessBits> access,
			BitField<RDD::BufferUsageBits> usage);
	private:
		Ref<RenderGraph> graph;
		uint32_t index;
		uint32_t physical_pass;
		BitField<RDD::CommandQueueFamilyBits> queue;
		std::function<void(RDD::CommandBufferID)> render_pass_builder; // 这里只能暴露CommandBufferID
		
		std::function<bool (float, uint32_t)> depth_stencil_cleaner;
		std::function<bool(unsigned, Color)> color_cleaner;

		LocalVector<RenderTextureResource*> color_outputs;
		LocalVector<RenderTextureResource*> resolve_outputs;
		LocalVector<RenderTextureResource*> color_inputs;
		LocalVector<RenderTextureResource*> color_scale_inputs;
		LocalVector<RenderTextureResource*> storage_texture_inputs;
		LocalVector<RenderTextureResource*> storage_texture_outputs;
		LocalVector<RenderTextureResource*> blit_texture_inputs;
		LocalVector<RenderTextureResource*> blit_texture_outputs;
		LocalVector<RenderTextureResource*> attachments_inputs;
		LocalVector<RenderTextureResource*> history_inputs;
		LocalVector<RenderBufferResource*> storage_outputs;
		LocalVector<RenderBufferResource*> storage_inputs;
		LocalVector<RenderBufferResource*> transfer_outputs;
		RenderTextureResource* depth_stencil_input = nullptr;
		RenderTextureResource* depth_stencil_output = nullptr;
		LocalVector<AccessedTextureResource> generic_texture;
		LocalVector<AccessedBufferResource> generic_buffer;
		LocalVector<AccessedProxyResource> proxy_inputs;
		LocalVector<AccessedProxyResource> proxy_outputs;
		LocalVector<KeyValue< RenderTextureResource, RenderTextureResource>> name_remap;
		StringName pass_name;

		RenderBufferResource& add_generic_buffer_input(const StringName& name,
			BitField<RDD::PipelineStageBits > p_stages,
			BitField<RDD::BarrierAccessBits > p_access,
			BitField<RDD::BufferUsageBits> p_usage);
	};

	

	class RenderGraph : public RefCounted {
		LCLASS(RenderGraph, RefCounted);
	public:
		Ref<RenderPass> add_pass(const StringName& p_name, BitField<RDD::CommandQueueFamilyBits> queue);
		Ref<RenderPass> find_pass(const StringName& p_name);
		void compile();
		RenderTextureResource& get_texture_resource(const StringName& name);
		RenderBufferResource& get_buffer_resource(const StringName& name);
		RenderResource& get_proxy_resource(const StringName& name);
		/// members
	private:
		RDD* driver = nullptr;
		Vector<Ref<RenderPass>> passes;
		Vector<Ref<RenderResource>> resources;
		HashMap<StringName, uint32_t> pass_to_index;
		HashMap<StringName, uint32_t> resource_to_index;
		StringName backbuffer_source;
		Vector<uint32_t> pass_stack; // 存储写入backbuffer以及相关依赖的pass

		/********
		Barrier
		********/
		struct Barrier
		{
			unsigned resource_index;
			RDD::TextureLayout layout;
			BitField<RDD::BarrierAccessBits> access;
			BitField<RDD::PipelineStageBits> stages;
			bool history; // src or dst?
		};

		struct Barriers
		{
			LocalVector<Barrier> invalidate; // 内存可见过程
			LocalVector<Barrier> flush; // 内存可用过程
		};

		LocalVector<Barriers> pass_barriers;
		// tool function
		void _filter_passes();
		void _validate_passes();
		void _build_barriers();

		RR::ResourceDimensions swapchain_dimensions;
		// RenderPass，包括一些subpass

		struct PhysicalPass {
			LocalVector<uint32_t> passes;
			LocalVector<uint32_t> discards;
			LocalVector<Barrier> invalidate;
			LocalVector<Barrier> flush;
			LocalVector<Barrier> history;
			Vector< KeyValue<uint32_t, uint32_t> > alias_transfer;

			// RDD::RenderPassInfo render_pass_info;
			Vector<unsigned> physical_color_attachment;
			unsigned physical_color_attachment;
		};
		// Vector

	};


	}
#endif // !RENDERING_DEVICE_GRAPH
