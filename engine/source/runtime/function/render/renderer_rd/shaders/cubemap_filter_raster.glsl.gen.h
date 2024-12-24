/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef CUBEMAP_FILTER_RASTER_RD
#define CUBEMAP_FILTER_RASTER_RD

#include "function/render/renderer_rd/shader_rd.h"
#include "core/io/file_access.h"
namespace lain{
class CubemapFilterRasterShaderRD : public ShaderRD {

public:

	CubemapFilterRasterShaderRD() {

		Error err;
		Ref<FileAccess> file1 = FileAccess::open("C:/LainEngine/engine/source/runtime/function/render/renderer_rd/shaders/cubemap_filter_raster.glsl.gen.vertex.lglsl", FileAccess::READ,  &err);
		DEV_ASSERT(err == OK);
		static const String _vertex_code = file1->get_as_utf8_string();
		Ref<FileAccess> file2 = FileAccess::open("C:/LainEngine/engine/source/runtime/function/render/renderer_rd/shaders/cubemap_filter_raster.glsl.gen.fragment.lglsl", FileAccess::READ,  &err);
		DEV_ASSERT(err == OK);
		static const String _fragment_code = file2->get_as_utf8_string();
		setup(_vertex_code, _fragment_code, String(), "CubemapFilterRasterShaderRD");
	}
};
}

#endif
