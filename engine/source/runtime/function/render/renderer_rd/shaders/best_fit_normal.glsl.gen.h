/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef BEST_FIT_NORMAL_RD
#define BEST_FIT_NORMAL_RD

#include "function/render/renderer_rd/shader_rd.h"
#include "core/io/file_access.h"
namespace lain{
class BestFitNormalShaderRD : public ShaderRD {

public:

	BestFitNormalShaderRD() {

		Error err;
		Ref<FileAccess> file = FileAccess::open("D:/LainEngine/engine/source/runtime/function/render/renderer_rd/shaders/best_fit_normal.glsl.gen.compute.lglsl", FileAccess::READ,  &err);
		DEV_ASSERT(err == OK);
		static const String _compute_code = file->get_as_utf8_string();
		setup(String(), String(), _compute_code, "BestFitNormalShaderRD");
	}
};
}

#endif
