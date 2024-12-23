/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef SUBSURFACE_SCATTERING_RD
#define SUBSURFACE_SCATTERING_RD

#include "function/render/renderer_rd/shader_rd.h"
#include "core/io/file_access.h"
namespace lain{
class SubsurfaceScatteringShaderRD : public ShaderRD {

public:

	SubsurfaceScatteringShaderRD() {

		Error err;
		Ref<FileAccess> file = FileAccess::open("C:/LainEngine/engine/source/runtime/function/render/renderer_rd/shaders/subsurface_scattering.glsl.gen.compute.lglsl", FileAccess::READ,  &err);
		DEV_ASSERT(err == OK);
		static const String _compute_code = file->get_as_utf8_string();
		setup(String(), String(), _compute_code, "SubsurfaceScatteringShaderRD");
	}
};
}

#endif
