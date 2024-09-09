#include "rendering_system_default.h"
using namespace lain;
RenderingSystemDefault::RenderingSystemDefault(bool p_create_thread) {
	create_thread = p_create_thread;
    RS::init();
}

RenderingSystemDefault::~RenderingSystemDefault() {
    
}