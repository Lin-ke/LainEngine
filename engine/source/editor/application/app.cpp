#ifdef L_PLATFORM_WINDOWS
#include <core/log/log.h>
#include <core/meta/meta_example.cpp>
#include <engine.h>

int main(int argc, char** argv) {
	Lain::Log::Init();
	L_CORE_ERROR("Hello! Var={0}", 5);
	L_INFO("Hello! Var={0}", 5);
	Lain::Engine* engine = new Lain::Engine();
	engine->startEngine("");
	engine->initialize();
	Lain::metaExample();
	
	// engineruntime注入到editor当中
	// editor->initialize(engine);

}

#endif
