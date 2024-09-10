#include "rendering_system_default.h"
using namespace lain;
int RenderingSystemDefault::changes = 0;

RenderingSystemDefault::RenderingSystemDefault(bool p_create_thread) {
	create_thread = p_create_thread;
    RS::init();
}

RenderingSystemDefault::~RenderingSystemDefault() {
    
}

void RenderingSystemDefault::init(){

}

void RenderingSystemDefault::_free(RID p_rid) {
// 	if (unlikely(p_rid.is_null())) {
// 		return;
// 	}
// 	if (RSG::utilities->free(p_rid)) {
// 		return;
// 	}
// 	if (RSG::canvas->free(p_rid)) {
// 		return;
// 	}
// 	if (RSG::viewport->free(p_rid)) {
// 		return;
// 	}
// 	if (RSG::scene->free(p_rid)) {
// 		return;
// 	}
// }
}

void lain::RenderingSystemDefault::free(RID p_rid) {
    if (Thread::get_caller_id() == server_thread) {
			command_queue.flush_if_pending();
			_free(p_rid);
		} else {
			command_queue.push(this, &RenderingSystemDefault::_free, p_rid);
		}
}
