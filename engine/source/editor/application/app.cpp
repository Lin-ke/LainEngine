#ifdef L_PLATFORM_WINDOWS
#include <base.h>
#include <core/meta/meta_example.cpp>
#include <core/engine/engine.h>
#include <core/math/vector2.h>
#include <core/error/error_macros.h>
#include <core/os/memory.h>
int YminusA(int a, lain::Vector2& obj);
void TryERR_FAIL_INDEX();
int main(int argc, char** argv) {
	lain::Log::Init();
	L_CORE_ERROR("Hello! Var={0}", 5);
	L_INFO("Hello! Var={0}", 5);
	lain::Engine* engine = new lain::Engine();
	engine->startEngine("");
	engine->initialize();
	//lain::CowData<std::string>();
	

}
void TryERR_FAIL_INDEX() {
	ERR_FAIL_INDEX(-1, 3);
}
int YminusA(int a, lain::Vector2& obj) {
	auto meta = lain::Reflection::TypeMeta::newMetaFromName("Vector2");
	if (!meta.isValid()) {
		L_ERROR("meta is not valid");
	}
	auto field_accessor = meta.getFieldByName("x");
	float b = 16;
	if (field_accessor.isValid()) {
		L_INFO("valid");
		// set
		field_accessor.set((void*)&obj, (float *)&b);
		L_JSON(obj);
	}
	else {
		L_ERROR("Field is not valid");
	}
	return 0;


}

#endif
