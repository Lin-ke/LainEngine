/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef COPY_RD
#define COPY_RD

#include "function/render/renderer_rd/shader_rd.h"
#include "core/io/file_access.h"
namespace lain{
class CopyShaderRD : public ShaderRD {

public:

	CopyShaderRD() {

		Error err;
		Ref<FileAccess> file = FileAccess::open("C:/LainEngine/engine/source/runtime/function/render/renderer_rd/shaders/copy.glsl.gen.compute.lglsl", FileAccess::READ,  &err);
		DEV_ASSERT(err == OK);
		static const String _compute_code = file->get_as_utf8_string();
		setup(String(), String(), _compute_code, "CopyShaderRD");
	}
};
}

#endif
