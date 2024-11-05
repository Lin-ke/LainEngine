#include "rendering_system_default.h"
#include "function/render/scene/renderer_scene_cull.h"

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
void RenderingSystemDefault::sync() {
	if (create_thread) {
		command_queue.sync();
	} else {
		command_queue.flush_all(); // Flush all pending from other threads.
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

void lain::RenderingSystemDefault::_finish() {


	// RSG::canvas->finalize();
	// memdelete(RSG::canvas);
	RSG::rasterizer->finalize();
	memdelete(RSG::viewport);
	memdelete(RSG::rasterizer);
	memdelete(RSG::scene);
	// memdelete(RSG::camera_attributes);
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

void lain::RenderingSystemDefault::_init() {
	RSG::threaded = create_thread;

	// RSG::canvas = memnew(RendererCanvasCull);
	RSG::viewport = memnew(RendererViewport);
	RendererSceneCull *sr = memnew(RendererSceneCull);
	// RSG::camera_attributes = memnew(RendererCameraAttributes);
	RSG::scene = sr;
	RSG::rasterizer = RendererCompositor::create();
	RSG::utilities = RSG::rasterizer->get_utilities();
	RSG::rasterizer->initialize();
	RSG::light_storage = RSG::rasterizer->get_light_storage();
	RSG::material_storage = RSG::rasterizer->get_material_storage();
	RSG::mesh_storage = RSG::rasterizer->get_mesh_storage();
	// RSG::particles_storage = RSG::rasterizer->get_particles_storage();
	RSG::texture_storage = RSG::rasterizer->get_texture_storage();
	// RSG::gi = RSG::rasterizer->get_gi();
	// RSG::fog = RSG::rasterizer->get_fog();
	// RSG::canvas_render = RSG::rasterizer->get_canvas();
	sr->set_scene_render(RSG::rasterizer->get_scene());
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

void RenderingSystemDefault::draw(bool p_swap_buffers, double frame_step){
ERR_FAIL_COND_MSG(!Thread::is_main_thread(), "Manually triggering the draw function from the RenderingServer can only be done on the main thread. Call this function from the main thread or use call_deferred().");
	// Needs to be done before changes is reset to 0, to not force the editor to redraw.
	// RS::get_singleton()->emit_signal(SNAME("frame_pre_draw"));
	changes = 0;
	if (create_thread) {
		command_queue.push(this, &RenderingSystemDefault::_draw, p_swap_buffers, frame_step);
	} else {
		_draw(p_swap_buffers, frame_step);
	}	
}

void lain::RenderingSystemDefault::tick() {
	// canvas tick
}

bool lain::RenderingSystemDefault::has_changed() const {
  	return changes > 0;
}

uint64_t lain::RenderingSystemDefault::get_rendering_info(RenderingInfo p_info)
{
	if (p_info == RENDERING_INFO_TOTAL_OBJECTS_IN_FRAME) {
		return RSG::viewport->get_total_objects_drawn();
	} else if (p_info == RENDERING_INFO_TOTAL_PRIMITIVES_IN_FRAME) {
		return RSG::viewport->get_total_primitives_drawn();
	} else if (p_info == RENDERING_INFO_TOTAL_DRAW_CALLS_IN_FRAME) {
		return RSG::viewport->get_total_draw_calls_used();
	}
	return RSG::utilities->get_rendering_info(p_info);
}

void RenderingSystemDefault::_draw(bool p_swap_buffers, double frame_step) {
	RSG::rasterizer->begin_frame(frame_step);

	TIMESTAMP_BEGIN()

	uint64_t time_usec = OS::GetSingleton()->GetTicksUsec();

	RENDER_TIMESTAMP("Prepare Render Frame");
	RSG::scene->update(); //update scenes stuff before updating instances

	frame_setup_time = double(OS::GetSingleton()->GetTicksUsec() - time_usec) / 1000.0;

	// RSG::particles_storage->update_particles(); //need to be done after instances are updated (colliders and particle transforms), and colliders are rendered

	// RSG::scene->render_probes();

	RSG::viewport->draw_viewports(p_swap_buffers);
	// RSG::canvas_render->update();

	RSG::rasterizer->end_frame(p_swap_buffers);

// #ifndef _3D_DISABLED
// 	XRServer *xr_server = XRServer::get_singleton();
// 	if (xr_server != nullptr) {
// 		// let our XR server know we're done so we can get our frame timing
// 		xr_server->end_frame();
// 	}
// #endif // _3D_DISABLED

	// RSG::canvas->update_visibility_notifiers();
	// RSG::scene->update_visibility_notifiers();

	// if (create_thread) {
	// 	callable_mp(this, &RenderingSystemDefault::_run_post_draw_steps).call_deferred();
	// } else {
	// 	_run_post_draw_steps();
	// }

	if (RSG::utilities->get_captured_timestamps_count()) {
		Vector<FrameProfileArea> new_profile;
		if (RSG::utilities->capturing_timestamps) {
			new_profile.resize(RSG::utilities->get_captured_timestamps_count());
		}

		uint64_t base_cpu = RSG::utilities->get_captured_timestamp_cpu_time(0);
		uint64_t base_gpu = RSG::utilities->get_captured_timestamp_gpu_time(0);
		for (uint32_t i = 0; i < RSG::utilities->get_captured_timestamps_count(); i++) {
			uint64_t time_cpu = RSG::utilities->get_captured_timestamp_cpu_time(i);
			uint64_t time_gpu = RSG::utilities->get_captured_timestamp_gpu_time(i);

			String name = RSG::utilities->get_captured_timestamp_name(i);

			if (name.begins_with("vp_")) {
				RSG::viewport->handle_timestamp(name, time_cpu, time_gpu);
			}

			if (RSG::utilities->capturing_timestamps) {
				new_profile.write[i].gpu_msec = double((time_gpu - base_gpu) / 1000) / 1000.0;
				new_profile.write[i].cpu_msec = double(time_cpu - base_cpu) / 1000.0;
				new_profile.write[i].name = RSG::utilities->get_captured_timestamp_name(i);
			}
		}

		frame_profile = new_profile;
	}

	frame_profile_frame = RSG::utilities->get_captured_timestamps_frame();

	if (print_gpu_profile) {
		if (print_frame_profile_ticks_from == 0) {
			print_frame_profile_ticks_from = OS::GetSingleton()->GetTicksUsec();
		}
		double total_time = 0.0;

		for (int i = 0; i < frame_profile.size() - 1; i++) {
			String name = frame_profile[i].name;
			if (name[0] == '<' || name[0] == '>') {
				continue;
			}

			double time = frame_profile[i + 1].gpu_msec - frame_profile[i].gpu_msec;

			if (name[0] != '<' && name[0] != '>') {
				if (print_gpu_profile_task_time.has(name)) {
					print_gpu_profile_task_time[name] += time;
				} else {
					print_gpu_profile_task_time[name] = time;
				}
			}
		}

		if (frame_profile.size()) {
			total_time = frame_profile[frame_profile.size() - 1].gpu_msec;
		}

		uint64_t ticks_elapsed = OS::GetSingleton()->GetTicksUsec() - print_frame_profile_ticks_from;
		print_frame_profile_frame_count++;
		if (ticks_elapsed > 1000000) {
			print_line("GPU PROFILE (total " + rtos(total_time) + "ms): ");

			float print_threshold = 0.01;
			for (const KeyValue<String, float> &E : print_gpu_profile_task_time) {
				double time = E.value / double(print_frame_profile_frame_count);
				if (time > print_threshold) {
					print_line("\t-" + E.key + ": " + rtos(time) + "ms");
				}
			}
			print_gpu_profile_task_time.clear();
			print_frame_profile_ticks_from = OS::GetSingleton()->GetTicksUsec();
			print_frame_profile_frame_count = 0;
		}
	}

	RSG::utilities->update_memory_info();
}

// 回调
void RenderingSystemDefault::_run_post_draw_steps() {
	// while (frame_drawn_callbacks.front()) {
	// 	Callable c = frame_drawn_callbacks.front()->get();
	// 	Variant result;
	// 	Callable::CallError ce;
	// 	c.callp(nullptr, 0, result, ce);
	// 	if (ce.error != Callable::CallError::CALL_OK) {
	// 		String err = Variant::get_callable_error_text(c, nullptr, 0, ce);
	// 		ERR_PRINT("Error calling frame drawn function: " + err);
	// 	}

	// 	frame_drawn_callbacks.pop_front();
	// }

	// emit_signal(SNAME("frame_post_draw"));
}