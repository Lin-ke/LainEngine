#include "function/render/rendering_system/rendering_system.h"
#include "function/render/renderer_rd/shaders/copy.glsl.gen.h"
namespace lain::RendererRD{
	class CopyEffects{

			struct CopyPushConstant {
		int32_t section[4];
		int32_t target[2];
		uint32_t flags;
		uint32_t pad;
		// Glow.
		float glow_strength;
		float glow_bloom;
		float glow_hdr_threshold;
		float glow_hdr_scale;

		float glow_exposure;
		float glow_white;
		float glow_luminance_cap;
		float glow_auto_exposure_scale;
		// DOF.
		float camera_z_far;
		float camera_z_near;
		uint32_t pad2[2];
		//SET color
		float set_color[4];
	};

	// 一些flag
		enum {
		COPY_FLAG_HORIZONTAL = (1 << 0),
		COPY_FLAG_USE_COPY_SECTION = (1 << 1),
		COPY_FLAG_USE_ORTHOGONAL_PROJECTION = (1 << 2),
		COPY_FLAG_DOF_NEAR_FIRST_TAP = (1 << 3),
		COPY_FLAG_GLOW_FIRST_PASS = (1 << 4),
		COPY_FLAG_FLIP_Y = (1 << 5),
		COPY_FLAG_FORCE_LUMINANCE = (1 << 6),
		COPY_FLAG_ALL_SOURCE = (1 << 7),
		COPY_FLAG_ALPHA_TO_ONE = (1 << 8),
	};

	enum CopyMode {
		COPY_MODE_GAUSSIAN_COPY,
		COPY_MODE_GAUSSIAN_COPY_8BIT,
		COPY_MODE_GAUSSIAN_GLOW,
		COPY_MODE_GAUSSIAN_GLOW_AUTO_EXPOSURE,
		COPY_MODE_SIMPLY_COPY,
		COPY_MODE_SIMPLY_COPY_8BIT,
		COPY_MODE_SIMPLY_COPY_DEPTH,
		COPY_MODE_SET_COLOR,
		COPY_MODE_SET_COLOR_8BIT,
		COPY_MODE_MIPMAP,
		COPY_MODE_LINEARIZE_DEPTH,
		COPY_MODE_CUBE_TO_PANORAMA,
		COPY_MODE_CUBE_ARRAY_TO_PANORAMA,
		COPY_MODE_MAX,

	};

	struct Copy {
		CopyPushConstant push_constant;
		CopyShaderRD shader;
		RID shader_version;
		RID pipelines[COPY_MODE_MAX];

	} copy;



	void copy_to_rect(RID p_source_rd_texture, RID p_dest_texture, const Rect2i &p_rect, bool p_flip_y = false, bool p_force_luminance = false, bool p_all_source = false, bool p_8_bit_dst = false, bool p_alpha_to_one = false);

	};
}