/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef RESOLVE_RD
#define RESOLVE_RD

#include "function/render/renderer_rd/shader_rd.h"
#include "core/io/file_access.h"
namespace lain{
class ResolveShaderRD : public ShaderRD {

public:

	ResolveShaderRD() {

		Error err;
		Ref<FileAccess> file = FileAccess::open("C:/LainEngine/engine/source/runtime/function/render/renderer_rd/shaders/resolve.glsl.gen.compute.lglsl", FileAccess::READ,  &err);
		DEV_ASSERT(err == OK);
		static const String _compute_code = file->get_as_utf8_string();
		setup(String(), String(), _compute_code, "ResolveShaderRD");
	}
};
}

#endif
