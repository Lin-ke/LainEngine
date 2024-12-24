/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef CUBEMAP_DOWNSAMPLER_RD
#define CUBEMAP_DOWNSAMPLER_RD

#include "function/render/renderer_rd/shader_rd.h"
#include "core/io/file_access.h"
namespace lain{
class CubemapDownsamplerShaderRD : public ShaderRD {

public:

	CubemapDownsamplerShaderRD() {

		Error err;
		Ref<FileAccess> file = FileAccess::open("C:/LainEngine/engine/source/runtime/function/render/renderer_rd/shaders/cubemap_downsampler.glsl.gen.compute.lglsl", FileAccess::READ,  &err);
		DEV_ASSERT(err == OK);
		static const String _compute_code = file->get_as_utf8_string();
		setup(String(), String(), _compute_code, "CubemapDownsamplerShaderRD");
	}
};
}

#endif
