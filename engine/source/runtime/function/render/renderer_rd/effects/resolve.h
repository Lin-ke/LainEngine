
/**************************************************************************/
/*  resolve.h                                                             */
/**************************************************************************/

#ifndef RESOLVE_RD_H
#define RESOLVE_RD_H

#include "function/render/renderer_rd/pipeline_cache_rd.h"
#include "function/render/rendering_system/rendering_system.h"
#include "function/render/renderer_rd/shaders/resolve.glsl.gen.h"


namespace lain::RendererRD {

class Resolve {
private:
	struct ResolvePushConstant {
		int32_t screen_size[2];
		int32_t samples;
		uint32_t pad;
	};

	enum ResolveMode {
		RESOLVE_MODE_GI,
		RESOLVE_MODE_GI_VOXEL_GI,
		RESOLVE_MODE_DEPTH,
		RESOLVE_MODE_MAX
	};

	struct ResolveShader {
		ResolvePushConstant push_constant;
		ResolveShaderRD shader;
		RID shader_version;
		RID pipelines[RESOLVE_MODE_MAX]; //3 quality levels
	} resolve;

public:
	Resolve();
	~Resolve();

	void resolve_gi(RID p_source_depth, RID p_source_normal_roughness, RID p_source_voxel_gi, RID p_dest_depth, RID p_dest_normal_roughness, RID p_dest_voxel_gi, Vector2i p_screen_size, int p_samples);
	void resolve_depth(RID p_source_depth, RID p_dest_depth, Vector2i p_screen_size, int p_samples);
};

} // namespace RendererRD

#endif // RESOLVE_RD_H
