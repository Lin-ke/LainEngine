#pragma once
#ifndef RENDERING_DEVICE_H
#define RENDERING_DEVICE_H
#include "core/meta/class_db.h"
#include "rendering_device_commons.h"
#include "core/thread/worker_thread_pool.h"
#include "core/os/thread_safe.h"
#include "core/io/rid_owner.h"
#include "rendering_device_driver.h"
#include "core/templates/rb_map.h"

namespace lain::graphics {

	class RenderingDevice : public RenderingDeviceCommons {
		LCLASS(RenderingDevice, Object)
		_THREAD_SAFE_CLASS_
		RenderingDeviceDriver* driver = nullptr;
	public:

		/***************************/
		/**** BUFFER MANAGEMENT ****/
		/***************************/
		struct Buffer {
			RDD::BufferID driver_id;
			uint32_t size = 0;
			BitField<RDD::BufferUsageBits> usage;
			//RDG::ResourceTracker* draw_tracker = nullptr;
		};

		
		/*****************/
		/**** TEXTURE ****/
		/*****************/

		// In modern APIs, the concept of textures may not exist;
		// instead there is the image (the memory pretty much,
		// the view (how the memory is interpreted) and the
		// sampler (how it's sampled from the shader).
		//
		// Texture here includes the first two stages, but
		// It's possible to create textures sharing the image
		// but with different views. The main use case for this
		// is textures that can be read as both SRGB/Linear,
		// or slices of a texture (a mipmap, a layer, a 3D slice)
		// for a framebuffer to render into it.
		struct Texture {
			RDD::TextureID driver_id;
			TextureType type = TEXTURE_TYPE_MAX;
			DataFormat format = DATA_FORMAT_MAX;
			TextureSamples samples = TEXTURE_SAMPLES_MAX;
			TextureSliceType slice_type = TEXTURE_SLICE_MAX;
			Rect2i slice_rect;
			uint32_t width = 0;
			uint32_t height = 0;
			uint32_t depth = 0;
			uint32_t layers = 0;
			uint32_t mipmaps = 0;
			uint32_t usage_flags = 0;
			uint32_t base_mipmap = 0;
			uint32_t base_layer = 0;

			Vector<DataFormat> allowed_shared_formats;

			bool is_resolve_buffer = false;
			bool has_initial_data = false;

			BitField<RDD::TextureAspectBits> read_aspect_flags;
			BitField<RDD::TextureAspectBits> barrier_aspect_flags;
			bool bound = false; // Bound to framebuffer.
			RID owner;

			/*RDG::ResourceTracker* draw_tracker = nullptr;
			HashMap<Rect2i, RDG::ResourceTracker*> slice_trackers;*/

			RDD::TextureSubresourceRange barrier_range() const {
				RDD::TextureSubresourceRange r;
				r.aspect = barrier_aspect_flags;
				r.base_mipmap = base_mipmap;
				r.mipmap_count = mipmaps;
				r.base_layer = base_layer;
				r.layer_count = layers;
				return r;
			}
		};

		

		struct TextureView {
			DataFormat format_override = DATA_FORMAT_MAX; // // Means, use same as format.
			TextureSwizzle swizzle_r = TEXTURE_SWIZZLE_R;
			TextureSwizzle swizzle_g = TEXTURE_SWIZZLE_G;
			TextureSwizzle swizzle_b = TEXTURE_SWIZZLE_B;
			TextureSwizzle swizzle_a = TEXTURE_SWIZZLE_A;

			bool operator==(const TextureView& p_other) const {
				if (format_override != p_other.format_override) {
					return false;
				}
				else if (swizzle_r != p_other.swizzle_r) {
					return false;
				}
				else if (swizzle_g != p_other.swizzle_g) {
					return false;
				}
				else if (swizzle_b != p_other.swizzle_b) {
					return false;
				}
				else if (swizzle_a != p_other.swizzle_a) {
					return false;
				}
				else {
					return true;
				}
			}
		};
		uint64_t texture_memory = 0;
		RID_Alloc <Texture> texture_owner;

		RID texture_create(const TextureFormat& p_format, const TextureView& p_view, const Vector<Vector<uint8_t>>& p_data = Vector<Vector<uint8_t>>());
		TextureFormat texture_get_format(RID p_texture);

		
		/************************/
		/**** DRAW LISTS (I) ****/
		/************************/
		///
		/// LoadOP
		///
		enum InitialAction { 
			INITIAL_ACTION_LOAD,
			INITIAL_ACTION_CLEAR,
			INITIAL_ACTION_DISCARD,
			INITIAL_ACTION_MAX
		};
		enum FinalAction {
			FINAL_ACTION_STORE,
			FINAL_ACTION_DISCARD,
			FINAL_ACTION_MAX
		};
		/*********************/
		/**** RenderPass ****/
		/*********************/
		// In modern APIs, generally, framebuffers work similar to how they
		// do in OpenGL, with the exception that
		// the "format" (RDD::RenderPassID) is not dynamic
		// and must be more or less the same as the one
		// used for the render pipelines. 需要与renderpass中规定的兼容

		struct AttachmentFormat {
			enum { UNUSED_ATTACHMENT = 0xFFFFFFFF };
			DataFormat format;
			TextureSamples samples;
			uint32_t usage_flags; // TextureUsageBits
			AttachmentFormat() {
				format = DATA_FORMAT_R8G8B8A8_UNORM;
				samples = TEXTURE_SAMPLES_1;
				usage_flags = TEXTURE_USAGE_COLOR_ATTACHMENT_BIT;
			}
			AttachmentFormat(DataFormat p_format, TextureSamples p_samples, uint32_t p_usage_flags) {
				format = p_format; 
				samples = p_samples;
				usage_flags = p_usage_flags;
			}
			AttachmentFormat(RID p_texture, uint32_t p_usage_flags) {
				auto texture_format = RenderingDevice::get_singleton()->texture_get_format(p_texture);
				format = texture_format.format;
				samples = texture_format.samples;
				usage_flags = p_usage_flags;
			}

		};
		enum DepthStencilUsage {
			None,
			ReadOnly,
			ReadWrite
		};

		// subpass in granite
		// attachment与实际imageview的绑定未做
		// renderpass是使用attachmentinfo定义的
		// 格式信息可以一直保存
		struct FramebufferPass {
			Vector<int32_t> color_attachments;
			Vector<int32_t> input_attachments;
			Vector<int32_t> resolve_attachments;
			Vector<int32_t> preserve_attachments;
			int32_t depth_attachment = ATTACHMENT_UNUSED;
			int32_t vrs_attachment = ATTACHMENT_UNUSED; // density map for VRS, only used if supported
		};

		typedef int64_t FramebufferFormatID;
		// RenderPassformat的引入
		struct FramebufferFormatKey {	
		Vector<AttachmentFormat> attachment_formats;
			Vector<FramebufferPass> passes;
			uint32_t view_count = 1;
			bool operator<(const FramebufferFormatKey& p_key) const;
		};
		

private:
		/**************************/
		/**** QUEUE MANAGEMENT ****/
		/**************************/

		RDD::CommandQueueFamilyID main_queue_family;
		RDD::CommandQueueFamilyID present_queue_family;
		RDD::CommandQueueID main_queue;
		RDD::CommandQueueID present_queue;
		/**************************/
		/**** FRAME MANAGEMENT ****/
		/**************************/

		// This is the frame structure. There are normally
		// 3 of these (used for triple buffering), or 2
		// (double buffering). They are cycled constantly.
		//
		// It contains two command buffers, one that is
		// used internally for setting up (creating stuff)
		// and another used mostly for drawing.
		//
		// They also contains a list of things that need
		// to be disposed of when deleted, which can't
		// happen immediately due to the asynchronous
		// nature of the GPU. They will get deleted
		// when the frame is cycled.
		//struct Frame {
		//	// List in usage order, from last to free to first to free.
		//	List<Buffer> buffers_to_dispose_of;
		//	List<Texture> textures_to_dispose_of;
		//	List<Framebuffer> framebuffers_to_dispose_of;
		//	List<RDD::SamplerID> samplers_to_dispose_of;
		//	List<Shader> shaders_to_dispose_of;
		//	List<UniformSet> uniform_sets_to_dispose_of;
		//	List<RenderPipeline> render_pipelines_to_dispose_of;
		//	List<ComputePipeline> compute_pipelines_to_dispose_of;

		//	RDD::CommandPoolID command_pool;

		//	// Used at the beginning of every frame for set-up.
		//	// Used for filling up newly created buffers with data provided on creation.
		//	// Primarily intended to be accessed by worker threads.
		//	// Ideally this command buffer should use an async transfer queue.
		//	RDD::CommandBufferID setup_command_buffer;

		//	// The main command buffer for drawing and compute.
		//	// Primarily intended to be used by the main thread to do most stuff.
		//	RDD::CommandBufferID draw_command_buffer;

		//	// Signaled by the setup submission. Draw must wait on this semaphore.
		//	RDD::SemaphoreID setup_semaphore;

		//	// Signaled by the draw submission. Present must wait on this semaphore.
		//	RDD::SemaphoreID draw_semaphore;

		//	// Signaled by the draw submission. Must wait on this fence before beginning
		//	// command recording for the frame.
		//	RDD::FenceID draw_fence;
		//	bool draw_fence_signaled = false;

		//	// Swap chains prepared for drawing during the frame that must be presented.
		//	LocalVector<RDD::SwapChainID> swap_chains_to_present;

		//	struct Timestamp {
		//		String description;
		//		uint64_t value = 0;
		//	};

		//	RDD::QueryPoolID timestamp_pool;

		//	TightLocalVector<String> timestamp_names;
		//	TightLocalVector<uint64_t> timestamp_cpu_values;
		//	uint32_t timestamp_count = 0;
		//	TightLocalVector<String> timestamp_result_names;
		//	TightLocalVector<uint64_t> timestamp_cpu_result_values;
		//	TightLocalVector<uint64_t> timestamp_result_values;
		//	uint32_t timestamp_result_count = 0;
		//	uint64_t index = 0;
		//};

		static RenderingDevice* singleton;
		static RenderingDevice* get_singleton() { return singleton; }
		RenderingDevice() { if (singleton == nullptr) singleton = this; }
		~RenderingDevice() {}
	};
	typedef RenderingDevice RD;

}

#endif