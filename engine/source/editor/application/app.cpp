#ifdef L_PLATFORM_WINDOWS
#ifdef L_DEBUG

#endif
#include <base.h>
#include <core/meta/meta_example.cpp>
#include <core/engine/engine.h>
#include <core/math/vector2.h>
#include <core/error/error_macros.h>
#include <core/os/memory.h>
#include <core/templates/cowdata.h>
#include <core/templates/vector.h>
#include <core/templates/safe_refcount.h>
#include <core/meta/meta_example.h>
using lain::Vector;
int YminusA(int a, lain::Vector2& obj);
void TryERR_FAIL_INDEX();
int main(int argc, char** argv) {
	lain::Log::Init();
	L_CORE_ERROR("Hello! Var={0}", 5);
	L_INFO("Hello! Var={0}", 5);
	L_PRINT(Memory::get_mem_usage());

	lain::Engine* engine = new lain::Engine();
	L_PRINT(Memory::get_mem_usage());
	engine->startEngine("");
	engine->initialize();
	lain::metaExample();
	lain::Vector2 a(1, 2);
	auto p_ref=lain::Reflection::ReflectionPtr<lain::Vector2>("Vector2", &a);
	L_PRINT(p_ref.getTypeName());
	
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
// in debug mode, each allocation is padded with 16 bytes
// GObeject contains reflection ptr as its' component.
