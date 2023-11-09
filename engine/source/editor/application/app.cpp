#ifdef L_PLATFORM_WINDOWS
#include <core/log/log.h>
#include <core/meta/meta_example.cpp>
#include <engine.h>
#include <core/math/vector2.h>
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
	if (!meta.isValid()) {
		L_ERROR("meta is not valid");
	}
	
	auto method = meta.getMethodByName("set_x");
	if (!method.isValid()) {
		L_ERROR("method is not valid");
	}
		
	else {
		L_INFO("method is valid");
	}
	Lain::Reflection::MethodAccessor* acc;
	size_t size = meta.getMethodsList(acc);
	for (int i = 0; i < size; ++i) {
		L_INFO("method name:", acc[i].getMethodName());
	}
}

#endif
// engineruntime注入到editor当中
// editor->initialize(engine);

// 构造了空的method对象会造成内存泄露吗？不会