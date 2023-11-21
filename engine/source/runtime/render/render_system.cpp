#include "render_system.h"
namespace lain{
	RenderSystem* RenderSystem::p_singleton = nullptr;
	RenderSystem::~RenderSystem() {
		p_singleton = nullptr;
	}

}