/**************************************************************************/
/*  pipeline_cache_rd.h                                                   */
/**************************************************************************/

#ifndef PIPELINE_CACHE_RD_H
#define PIPELINE_CACHE_RD_H

#include "core/os/spin_lock.h"
#include "function/render/rendering_device/rendering_device.h"
namespace lain::RendererRD{
// cache已经创建过的pbo
class PipelineCacheRD {
	SpinLock spin_lock;

	RID shader;
	uint64_t input_mask;

	RD::RenderPrimitive render_primitive;
	RD::PipelineRasterizationState rasterization_state;
	RD::PipelineMultisampleState multisample_state;
	RD::PipelineDepthStencilState depth_stencil_state;
	RD::PipelineColorBlendState blend_state;
	int dynamic_state_flags = 0;
	Vector<RD::PipelineSpecializationConstant> base_specialization_constants;

	struct Version {
		RD::VertexFormatID vertex_id;
		RD::FramebufferFormatID framebuffer_id;
		uint32_t render_pass;
		bool wireframe;
		uint32_t bool_specializations; // 还特别拎出来bool的specialization，是因为这和 uber shader 特化的版本关系很大吗
		RID pipeline;
	};

	Version *versions = nullptr;
	uint32_t version_count;

	RID _generate_version(RD::VertexFormatID p_vertex_format_id, RD::FramebufferFormatID p_framebuffer_format_id, bool p_wireframe, uint32_t p_render_pass, uint32_t p_bool_specializations = 0);

	void _clear();

public:
	void setup(RID p_shader, RD::RenderPrimitive p_primitive, const RD::PipelineRasterizationState &p_rasterization_state, RD::PipelineMultisampleState p_multisample, const RD::PipelineDepthStencilState &p_depth_stencil_state, const RD::PipelineColorBlendState &p_blend_state, int p_dynamic_state_flags = 0, const Vector<RD::PipelineSpecializationConstant> &p_base_specialization_constants = Vector<RD::PipelineSpecializationConstant>());
	void update_specialization_constants(const Vector<RD::PipelineSpecializationConstant> &p_base_specialization_constants);
	void update_shader(RID p_shader);

	_FORCE_INLINE_ RID get_render_pipeline(RD::VertexFormatID p_vertex_format_id, RD::FramebufferFormatID p_framebuffer_format_id, bool p_wireframe = false, uint32_t p_render_pass = 0, uint32_t p_bool_specializations = 0) {
#ifdef DEBUG_ENABLED
		ERR_FAIL_COND_V_MSG(shader.is_null(), RID(),
				"Attempted to use an unused shader variant (shader is null),");
#endif

		spin_lock.lock();
		p_wireframe |= rasterization_state.wireframe;

		RID result;
		for (uint32_t i = 0; i < version_count; i++) {
			if (versions[i].vertex_id == p_vertex_format_id && versions[i].framebuffer_id == p_framebuffer_format_id && versions[i].wireframe == p_wireframe && versions[i].render_pass == p_render_pass && versions[i].bool_specializations == p_bool_specializations) {
				result = versions[i].pipeline;
				spin_lock.unlock();
				return result;
			}
		}
		result = _generate_version(p_vertex_format_id, p_framebuffer_format_id, p_wireframe, p_render_pass, p_bool_specializations);
		spin_lock.unlock();
		return result;
	}

	_FORCE_INLINE_ uint64_t get_vertex_input_mask() {
		if (input_mask == 0) {
			ERR_FAIL_COND_V(shader.is_null(), 0);
			input_mask = RD::get_singleton()->shader_get_vertex_input_attribute_mask(shader);
		}
		return input_mask;
	}
	void clear();
	PipelineCacheRD();
	~PipelineCacheRD();
	
};

}
#endif // PIPELINE_CACHE_RD_H
