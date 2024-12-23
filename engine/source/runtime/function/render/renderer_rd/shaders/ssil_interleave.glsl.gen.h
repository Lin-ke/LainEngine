/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef SSIL_INTERLEAVE_RD
#define SSIL_INTERLEAVE_RD

#include "function/render/renderer_rd/shader_rd.h"
#include "core/io/file_access.h"
namespace lain{
class SsilInterleaveShaderRD : public ShaderRD {

public:

	SsilInterleaveShaderRD() {

		Error err;
		Ref<FileAccess> file = FileAccess::open("C:/LainEngine/engine/source/runtime/function/render/renderer_rd/shaders/ssil_interleave.glsl.gen.compute.lglsl", FileAccess::READ,  &err);
		DEV_ASSERT(err == OK);
		static const String _compute_code = file->get_as_utf8_string();
		setup(String(), String(), _compute_code, "SsilInterleaveShaderRD");
	}
};
}

#endif
