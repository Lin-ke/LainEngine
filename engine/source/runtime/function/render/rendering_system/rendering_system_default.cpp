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
	// 在这里新建各种类的实例
	if (create_thread) {
		// WorkerThreadPool::TaskID tid = WorkerThreadPool::get_singleton()->add_task(callable_mp(this, & RenderingSystemDefault::_thread_loop), true);
		WorkerThreadPool::TaskID tid = WorkerThreadPool::get_singleton()->add_template_task(this, &RenderingSystemDefault::_thread_loop, this, true);

		command_queue.set_pump_task_id(tid); 
		command_queue.push(this, &RenderingSystemDefault::_assign_mt_ids, tid);
		command_queue.push_and_sync(this, &RenderingSystemDefault::_init);
		DEV_ASSERT(server_task_id == tid);
	}
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

void lain::RenderingSystemDefault::_thread_loop(void* p_data) {
	while (!exit) { 
		WorkerThreadPool::get_singleton()->yield(); // 等待， 直到push 会 notify
		command_queue.flush_all();
	}
}

void lain::RenderingSystemDefault::_assign_mt_ids(WorkerThreadPool::TaskID p_pump_task_id) {
	server_thread = Thread::get_caller_id();
	server_task_id = p_pump_task_id;
}

void lain::RenderingSystemDefault::free(RID p_rid) {
    if (Thread::get_caller_id() == server_thread) {
			command_queue.flush_if_pending();
			_free(p_rid);
		} else {
			command_queue.push(this, &RenderingSystemDefault::_free, p_rid);
		}
}

void lain::RenderingSystemDefault::finish() {
	if (create_thread) {
		command_queue.push(this, &RenderingSystemDefault::_finish);
		command_queue.push(this, &RenderingSystemDefault::_thread_exit);
		if (server_task_id != WorkerThreadPool::INVALID_TASK_ID) {
			WorkerThreadPool::get_singleton()->wait_for_task_completion(server_task_id);
			server_task_id = WorkerThreadPool::INVALID_TASK_ID;
		}
		server_thread = Thread::MAIN_ID;
	} 
	else{
		_finish();
	}
}
