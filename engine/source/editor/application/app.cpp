#ifdef L_PLATFORM_WINDOWS
#include <core/log/log.h>
#include <core/meta/meta_example.cpp>
#include <engine.h>
#include <core/math/vector2.h>
int YminusA(int a, Lain::Vector2& obj);
int main(int argc, char** argv) {
	Lain::Log::Init();
	L_CORE_ERROR("Hello! Var={0}", 5);
	L_INFO("Hello! Var={0}", 5);
	Lain::Engine* engine = new Lain::Engine();
	engine->startEngine("");
	engine->initialize();
	Lain::metaExample();

	// meta: vector2 test

	auto v2 = Lain::Vector2(1, 2);
	auto js = Lain::Serializer::write(v2);
	auto string = js.dump();
	L_CORE_INFO(string);
	// bind_methods
	auto meta = Lain::Reflection::TypeMeta::newMetaFromName("Vector2");
	
	
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
int YminusA(int a, Lain::Vector2& obj) {
	auto meta = Lain::Reflection::TypeMeta::newMetaFromName("Vector2");
	if (!meta.isValid()) {
		L_ERROR("meta is not valid");
	}
	auto field_accessor = meta.getFieldByName("x");
	int b = 16;
	if (field_accessor.isValid()) {
		L_INFO("valid");
		// set
		field_accessor.set((void*)&obj, (float *)b);
		Lain::Log::GetClientLogger()->info((obj.getX(), obj.getY()));
	}
	else {
		L_ERROR("Field is not valid");
	}
	return 0;


}

#endif
// engineruntime注入到editor当中
// editor->initialize(engine);

// 构造了空的method对象会造成内存泄露吗？不会