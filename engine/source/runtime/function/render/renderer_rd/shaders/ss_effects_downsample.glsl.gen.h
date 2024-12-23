/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef SS_EFFECTS_DOWNSAMPLE_RD
#define SS_EFFECTS_DOWNSAMPLE_RD

#include "function/render/renderer_rd/shader_rd.h"
#include "core/io/file_access.h"
namespace lain{
class SsEffectsDownsampleShaderRD : public ShaderRD {

public:

	SsEffectsDownsampleShaderRD() {

		Error err;
		Ref<FileAccess> file = FileAccess::open("C:/LainEngine/engine/source/runtime/function/render/renderer_rd/shaders/ss_effects_downsample.glsl.gen.compute.lglsl", FileAccess::READ,  &err);
		DEV_ASSERT(err == OK);
		static const String _compute_code = file->get_as_utf8_string();
		setup(String(), String(), _compute_code, "SsEffectsDownsampleShaderRD");
	}
};
}

#endif
