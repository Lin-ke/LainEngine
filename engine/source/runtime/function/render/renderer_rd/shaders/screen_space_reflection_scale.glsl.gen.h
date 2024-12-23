/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef SCREEN_SPACE_REFLECTION_SCALE_RD
#define SCREEN_SPACE_REFLECTION_SCALE_RD

#include "function/render/renderer_rd/shader_rd.h"
#include "core/io/file_access.h"
namespace lain{
class ScreenSpaceReflectionScaleShaderRD : public ShaderRD {

public:

	ScreenSpaceReflectionScaleShaderRD() {

		Error err;
		Ref<FileAccess> file = FileAccess::open("C:/LainEngine/engine/source/runtime/function/render/renderer_rd/shaders/screen_space_reflection_scale.glsl.gen.compute.lglsl", FileAccess::READ,  &err);
		DEV_ASSERT(err == OK);
		static const String _compute_code = file->get_as_utf8_string();
		setup(String(), String(), _compute_code, "ScreenSpaceReflectionScaleShaderRD");
	}
};
}

#endif
