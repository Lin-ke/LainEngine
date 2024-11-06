/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef TEST1_RD
#define TEST1_RD

#include "function/render/renderer_rd/shader_rd.h"
#include "core/io/file_access.h"
namespace lain{
class Test1ShaderRD : public ShaderRD {

public:

	Test1ShaderRD() {

		Error err;
		Ref<FileAccess> file1 = FileAccess::open("C:/LainEngine/game/test1.glsl.gen.vertex.lglsl", FileAccess::READ,  &err);
		DEV_ASSERT(err == OK);
		static const String _vertex_code = file1->get_as_utf8_string();
		Ref<FileAccess> file2 = FileAccess::open("C:/LainEngine/game/test1.glsl.gen.fragment.lglsl", FileAccess::READ,  &err);
		DEV_ASSERT(err == OK);
		static const String _fragment_code = file2->get_as_utf8_string();
		setup(_vertex_code, _fragment_code, String(), "Test1ShaderRD");
	}
};
}

#endif
