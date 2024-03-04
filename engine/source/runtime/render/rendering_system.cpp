#include "rendering_system.h"
namespace lain{
	RenderingSystem* RenderingSystem::p_singleton = nullptr;
	RenderingSystem::~RenderingSystem() {
		p_singleton = nullptr;
	}

}