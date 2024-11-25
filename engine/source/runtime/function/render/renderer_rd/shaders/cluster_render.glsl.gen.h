/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef CLUSTER_RENDER_RD
#define CLUSTER_RENDER_RD

#include "function/render/renderer_rd/shader_rd.h"
#include "core/io/file_access.h"
namespace lain{
class ClusterRenderShaderRD : public ShaderRD {

public:

	ClusterRenderShaderRD() {

		Error err;
		Ref<FileAccess> file1 = FileAccess::open("C:/LainEngine/engine/source/runtime/function/render/renderer_rd/shaders/cluster_render.glsl.gen.vertex.lglsl", FileAccess::READ,  &err);
		DEV_ASSERT(err == OK);
		static const String _vertex_code = file1->get_as_utf8_string();
		Ref<FileAccess> file2 = FileAccess::open("C:/LainEngine/engine/source/runtime/function/render/renderer_rd/shaders/cluster_render.glsl.gen.fragment.lglsl", FileAccess::READ,  &err);
		DEV_ASSERT(err == OK);
		static const String _fragment_code = file2->get_as_utf8_string();
		setup(_vertex_code, _fragment_code, String(), "ClusterRenderShaderRD");
	}
};
}

#endif
