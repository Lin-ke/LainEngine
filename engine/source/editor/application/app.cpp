#ifdef L_PLATFORM_WINDOWS
#include <base.h>
#include <core/meta/meta_example.cpp>
#include <core/engine/engine.h>
#include <core/math/vector2.h>
int YminusA(int a, lain::Vector2& obj);
int main(int argc, char** argv) {
	lain::Log::Init();
	L_CORE_ERROR("Hello! Var={0}", 5);
	L_INFO("Hello! Var={0}", 5);
	lain::Engine* engine = new lain::Engine();
	engine->startEngine("");
	engine->initialize();

	// meta: vector2 test

	auto v2 = lain::Vector2(1, 2);
	auto js = lain::Serializer::write(v2);
	auto string = js.dump();
	L_CORE_INFO(string);
	// bind_methods
	auto meta = lain::Reflection::TypeMeta::newMetaFromName("Vector2");
	
	
	auto method = meta.getMethodByName("set_x");
	if (!method.isValid()) {
		L_ERROR("method is not valid");
	}
		
	else {
		L_INFO("method is valid");
	}
	// try set method
	YminusA(3, v2);

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
