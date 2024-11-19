/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef CLUSTER_DEBUG_RD
#define CLUSTER_DEBUG_RD

#include "function/render/renderer_rd/shader_rd.h"
#include "core/io/file_access.h"
namespace lain{
class ClusterDebugShaderRD : public ShaderRD {

public:

	ClusterDebugShaderRD() {

		Error err;
		Ref<FileAccess> file = FileAccess::open("D:/LainEngine/engine/source/runtime/function/render/renderer_rd/shaders/cluster_debug.glsl.gen.compute.lglsl", FileAccess::READ,  &err);
		DEV_ASSERT(err == OK);
		static const String _compute_code = file->get_as_utf8_string();
		setup(String(), String(), _compute_code, "ClusterDebugShaderRD");
	}
};
}

#endif
