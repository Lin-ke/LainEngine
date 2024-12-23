
#ifndef TAA_RD_H
#define TAA_RD_H

#include "function/render/renderer_rd/pipeline_cache_rd.h"
#include "function/render/renderer_rd/shaders/taa_resolve.glsl.gen.h"
#include "function/render/renderer_rd/storage/render_scene_buffers_rd.h"
#include "function/render/scene/renderer_scene_renderer_api.h"

#include "function/render/rendering_system/rendering_system.h"

namespace lain::RendererRD {

class TAA {
public:
	TAA();
	~TAA();

	void process(Ref<RenderSceneBuffersRD> p_render_buffers, RD::DataFormat p_format, float p_z_near, float p_z_far);

private:
	struct TAAResolvePushConstant {
		float resolution_width;
		float resolution_height;
		float disocclusion_threshold;
		float disocclusion_scale;
	};

	TaaResolveShaderRD taa_shader;
	RID shader_version;
	RID pipeline;

	void resolve(RID p_frame, RID p_temp, RID p_depth, RID p_velocity, RID p_prev_velocity, RID p_history, Size2 p_resolution, float p_z_near, float p_z_far);
};

} // namespace RendererRD

#endif // TAA_RD_H
