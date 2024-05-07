#include "worker_thread_pool.h"
namespace lain {

	WorkerThreadPool* WorkerThreadPool::singleton = nullptr;

	WorkerThreadPool::WorkerThreadPool() {
		singleton = this;
	}

	WorkerThreadPool::~WorkerThreadPool() {
		finish();
	}
	WorkerThreadPool::TaskID WorkerThreadPool::add_native_task(void (*p_func)(void*), void* p_userdata, bool p_high_priority, const String& p_description) {
		return _add_task(Callable(), p_func, p_userdata, nullptr, p_high_priority, p_description);
	}

	WorkerThreadPool::TaskID WorkerThreadPool::_add_task(const Callable& p_callable, void (*p_func)(void*), void* p_userdata, BaseTemplateUserdata* p_template_userdata, bool p_high_priority, const String& p_description) {
		task_mutex.lock();
		// Get a free task
		Task* task = task_allocator.alloc();
		TaskID id = last_task++;
		task->self = id;
		//task->callable = p_callable;
		task->native_func = p_func;
		task->native_func_userdata = p_userdata;
		task->description = p_description;
		task->template_userdata = p_template_userdata;
		tasks.insert(id, task);

		_post_tasks_and_unlock(&task, 1, p_high_priority);

		return id;
	}


	void WorkerThreadPool::_post_tasks_and_unlock(Task** p_tasks, uint32_t p_count, bool p_high_priority) {
		// Fall back to processing on the calling thread if there are no worker threads.
		// Separated into its own variable to make it easier to extend this logic
		// in custom builds.
		bool process_on_calling_thread = threads.size() == 0;
		if (process_on_calling_thread) {
			task_mutex.unlock();
			for (uint32_t i = 0; i < p_count; i++) {
				_process_task(p_tasks[i]);
			}
			return;
		}

		uint32_t to_process = 0;
		uint32_t to_promote = 0;

		ThreadData* caller_pool_thread = thread_ids.has(Thread::get_caller_id()) ? &threads[thread_ids[Thread::get_caller_id()]] : nullptr;

		for (uint32_t i = 0; i < p_count; i++) {
			p_tasks[i]->low_priority = !p_high_priority;
			if (p_high_priority || low_priority_threads_used < max_low_priority_threads) {
				task_queue.add_last(&p_tasks[i]->task_elem);
				if (!p_high_priority) {
					low_priority_threads_used++;
				}
				to_process++;
			}
			else {
				// Too many threads using low priority, must go to queue.
				low_priority_task_queue.add_last(&p_tasks[i]->task_elem);
				to_promote++;
			}
		}

		_notify_threads(caller_pool_thread, to_process, to_promote);

		task_mutex.unlock();
	}


	void WorkerThreadPool::init(int p_thread_count, float p_low_priority_task_ratio) {
		ERR_FAIL_COND(threads.size() > 0);
		if (p_thread_count < 0) {
			p_thread_count = OS::GetSingleton()->GetDefaultThreadPoolSize();
		}

		max_low_priority_threads = CLAMP(p_thread_count * p_low_priority_task_ratio, 1, p_thread_count - 1);

		threads.resize(p_thread_count);
		// 插入thread index 和thread id的关系
		for (uint32_t i = 0; i < threads.size(); i++) {
			threads[i].index = i;
			// 调用构造函数，线程创建，并开始运行_thread_function
			threads[i].thread.start(&WorkerThreadPool::_thread_function, &threads[i]);
			thread_ids.insert(threads[i].thread.get_id(), i);
		}
	}
	// 线程：从Task queue里找到task然后执行
	void WorkerThreadPool::_thread_function(void* p_user) {
		ThreadData* thread_data = (ThreadData*)p_user;
		while (true) {
			Task* task_to_process = nullptr;
			{
				MutexLock lock(singleton->task_mutex); // unique_lock
				// 这个锁锁的是与任务分发相关的类单例互斥量task_mutex
				if (singleton->exit_threads) {
					return;
				}
				thread_data->signaled = false;

				if (singleton->task_queue.first()) {
					task_to_process = singleton->task_queue.first()->self();
					singleton->task_queue.remove(singleton->task_queue.first());
					// 获得任务
				}
				else { // 没有任务，等待
					thread_data->cond_var.wait(lock);
					DEV_ASSERT(singleton->exit_threads || thread_data->signaled);
				}
			}
			// lock解锁
			// 执行任务
			if (task_to_process) {
				singleton->_process_task(task_to_process);
			}
		}
	}
	void WorkerThreadPool::_process_task(Task* p_task) {
		// 获得threadid，根据id找thread(pool_thread_index)
		// Thread::get_caller_id就能获得当前线程id吗？
		int pool_thread_index = thread_ids[Thread::get_caller_id()];
		ThreadData& curr_thread = threads[pool_thread_index];
		Task* prev_task = nullptr; // In case this is recursively called.
		//bool safe_for_nodes_backup = is_current_thread_safe_for_nodes();

		{
			// Tasks must start with this unset. They are free to set-and-forget otherwise.
			//set_current_thread_safe_for_nodes(false);
			// Since the WorkerThreadPool is started before the script server,
			// its pre-created threads can't have ScriptServer::thread_enter() called on them early.
			// Therefore, we do it late at the first opportunity, so in case the task
			// about to be run uses scripting, guarantees are held.
			/*if (!curr_thread.ready_for_scripting && ScriptServer::are_languages_initialized()) {
				ScriptServer::thread_enter();
				curr_thread.ready_for_scripting = true;
			}*/
			task_mutex.lock();
			p_task->pool_thread_index = pool_thread_index;
			prev_task = curr_thread.current_task;
			curr_thread.current_task = p_task;
			task_mutex.unlock();
		}
		 // 组任务
		if (p_task->group) {
			// Handling a group
			bool do_post = false;

			while (true) {
				uint32_t work_index = p_task->group->index.postincrement();

				if (work_index >= p_task->group->max) {
					break;
				}
				if (p_task->native_group_func) {
					p_task->native_group_func(p_task->native_func_userdata, work_index);
				}
				else if (p_task->template_userdata) {
					p_task->template_userdata->callback_indexed(work_index);
				}
				else {
					//p_task->callable.call(work_index);
				}

				// This is the only way to ensure posting is done when all tasks are really complete.
				uint32_t completed_amount = p_task->group->completed_index.increment();

				if (completed_amount == p_task->group->max) {
					do_post = true;
				}
			}

			if (do_post && p_task->template_userdata) {
				memdelete(p_task->template_userdata); // This is no longer needed at this point, so get rid of it.
			}
			// 使用done_semaphore广播这个工作完成
			if (do_post) {
				p_task->group->done_semaphore.post();
				p_task->group->completed.set_to(true);
			}
			uint32_t max_users = p_task->group->tasks_used + 1; // Add 1 because the thread waiting for it is also user. Read before to avoid another thread freeing task after increment.
			uint32_t finished_users = p_task->group->finished.increment();

			if (finished_users == max_users) {
				// Get rid of the group, because nobody else is using it.
				task_mutex.lock();
				group_allocator.free(p_task->group);
				task_mutex.unlock();
			}

			// For groups, tasks get rid of themselves.

			task_mutex.lock();
			task_allocator.free(p_task);
		}
		else {
			if (p_task->native_func) {
				p_task->native_func(p_task->native_func_userdata);
			}
			else if (p_task->template_userdata) {
				p_task->template_userdata->callback();
				memdelete(p_task->template_userdata);
			}
			else {
				// p_task->callable.call();
			}
			// done
			task_mutex.lock();
			p_task->completed = true;
			p_task->pool_thread_index = -1;
			if (p_task->waiting_user) {
				p_task->done_semaphore.post(p_task->waiting_user);
			}
			// Let awaiters know.
			// 看哪个线程的awaited_task是这个
			for (uint32_t i = 0; i < threads.size(); i++) {
				if (threads[i].awaited_task == p_task) {
					threads[i].cond_var.notify_one();
					threads[i].signaled = true;
				}
			}
		}

		{
			curr_thread.current_task = prev_task;
			if (p_task->low_priority) {
				low_priority_threads_used--;

				if (_try_promote_low_priority_task()) {
					if (prev_task) { // Otherwise, this thread will catch it.
						_notify_threads(&curr_thread, 1, 0);
					}
				}
			}

			task_mutex.unlock();
		}

		//set_current_thread_safe_for_nodes(safe_for_nodes_backup);
	}
	// 低优先级，加入任务队列
	bool WorkerThreadPool::_try_promote_low_priority_task() {
		if (low_priority_task_queue.first()) {
			Task* low_prio_task = low_priority_task_queue.first()->self();
			low_priority_task_queue.remove(low_priority_task_queue.first());
			task_queue.add_last(&low_prio_task->task_elem);
			low_priority_threads_used++;
			return true;
		}
		else {
			return false;
		}
	}
	void WorkerThreadPool::_notify_threads(const ThreadData* p_current_thread_data, uint32_t p_process_count, uint32_t p_promote_count) {
		uint32_t to_process = p_process_count;
		uint32_t to_promote = p_promote_count;

		// This is where which threads are awaken is decided according to the workload.
		// Threads that will anyway have a chance to check the situation and process/promote tasks
		// are excluded from being notified. Others will be tried anyway to try to distribute load.
		// The current thread, if is a pool thread, is also excluded depending on the promoting/processing
		// needs because it will anyway loop again. However, it will contribute to decreasing the count,
		// which helps reducing sync traffic.

		uint32_t thread_count = threads.size();

		// First round:
		// 1. For processing: notify threads that are not running tasks, to keep the stacks as shallow as possible.
		// 2. For promoting: since it's exclusive with processing, we fin threads able to promote low-prio tasks now.
		for (uint32_t i = 0;
			i < thread_count && (to_process || to_promote);
			i++, notify_index = (notify_index + 1) % thread_count) {
			ThreadData& th = threads[notify_index];

			if (th.signaled) {
				continue;
			}
			if (th.current_task) {
				// Good thread for promoting low-prio?
				if (to_promote && th.awaited_task && th.current_task->low_priority) {
					if (likely(&th != p_current_thread_data)) {
						th.cond_var.notify_one();
					}
					th.signaled = true;
					to_promote--;
				}
			}
			else {
				if (to_process) {
					if (likely(&th != p_current_thread_data)) {
						th.cond_var.notify_one();
					}
					th.signaled = true;
					to_process--;
				}
			}
		}

		// Second round:
		// For processing: if the first round wasn't enough, let's try now with threads processing tasks but currently awaiting.
		for (uint32_t i = 0;
			i < thread_count && to_process;
			i++, notify_index = (notify_index + 1) % thread_count) {
			ThreadData& th = threads[notify_index];

			if (th.signaled) {
				continue;
			}
			if (th.awaited_task) {
				if (likely(&th != p_current_thread_data)) {
					th.cond_var.notify_one();
				}
				th.signaled = true;
				to_process--;
			}
		}
	}
	void WorkerThreadPool::finish() {
		if (threads.size() == 0) {
			return;
		}

		{
			MutexLock lock(task_mutex);
			SelfList<Task>* E = low_priority_task_queue.first();
			while (E) {
				L_CORE_ERROR(("Task waiting was never re-claimed: " + E->self()->description));
				E = E->next();
			}
		}

		{
			MutexLock lock(task_mutex);
			exit_threads = true;
		}
		for (ThreadData& data : threads) {
			data.cond_var.notify_one();
		}
		for (ThreadData& data : threads) {
			data.thread.wait_to_finish();
		}

		{
			MutexLock lock(task_mutex);
			for (KeyValue<TaskID, Task*>& E : tasks) {
				task_allocator.free(E.value);
			}
		}

		threads.clear();
	}

	Error WorkerThreadPool::wait_for_task_completion(TaskID p_task_id) {
		task_mutex.lock();
		Task** taskp = tasks.getptr(p_task_id);
		if (!taskp) {
			task_mutex.unlock();
			ERR_FAIL_V_MSG(ERR_INVALID_PARAMETER, "Invalid Task ID"); // Invalid task
		}
		Task* task = *taskp;
		// 已经完成
		if (task->completed) {
			if (task->waiting_pool == 0 && task->waiting_user == 0) {
				tasks.erase(p_task_id);
				task_allocator.free(task);
			}
			task_mutex.unlock();
			return OK;
		}
		// 是否是线程池中的线程
		ThreadData* caller_pool_thread = thread_ids.has(Thread::get_caller_id()) ? &threads[thread_ids[Thread::get_caller_id()]] : nullptr;
		if (caller_pool_thread && p_task_id <= caller_pool_thread->current_task->self) {
			// Deadlock prevention:
			// When a pool thread wants to wait for an older task, the following situations can happen:
			// 1. Awaited task is deep in the stack of the awaiter.
			// 2. A group of awaiter threads end up depending on some tasks buried in the stack
			//    of their worker threads in such a way that progress can't be made.
			// Both would entail a deadlock. Some may be handled here in the WorkerThreadPool
			// with some extra logic and bookkeeping. However, there would still be unavoidable
			// cases of deadlock because of the way waiting threads process outstanding tasks.
			// Taking into account there's no feasible solution for every possible case
			// with the current design, we just simply reject attempts to await on older tasks,
			// with a specific error code that signals the situation so the caller can handle it.
			task_mutex.unlock();
			return ERR_BUSY;
		}

		if (caller_pool_thread) {
			task->waiting_pool++;
		}
		else {
			task->waiting_user++;
		}

		task_mutex.unlock();

		if (caller_pool_thread) {
			while (true) {
				Task* task_to_process = nullptr;
				{
					MutexLock lock(task_mutex);
					bool was_signaled = caller_pool_thread->signaled;
					caller_pool_thread->signaled = false;

					if (task->completed) {
						// This thread was awaken also for some reason, but it's about to exit.
						// Let's find out what may be pending and forward the requests.
						if (!exit_threads && was_signaled) {
							uint32_t to_process = task_queue.first() ? 1 : 0;
							uint32_t to_promote = caller_pool_thread->current_task->low_priority && low_priority_task_queue.first() ? 1 : 0;
							if (to_process || to_promote) {
								// This thread must be left alone since it won't loop again.
								caller_pool_thread->signaled = true;
								_notify_threads(caller_pool_thread, to_process, to_promote);
							}
						}

						task->waiting_pool--;
						if (task->waiting_pool == 0 && task->waiting_user == 0) {
							tasks.erase(p_task_id);
							task_allocator.free(task);
						}

						break;
					}

					if (!exit_threads) {
						// This is a thread from the pool. It shouldn't just idle.
						// Let's try to process other tasks while we wait.
						// 加入低优先级任务
						if (caller_pool_thread->current_task->low_priority && low_priority_task_queue.first()) {
							if (_try_promote_low_priority_task()) {
								_notify_threads(caller_pool_thread, 1, 0);
							}
						}
						// 有任务
						if (singleton->task_queue.first()) {
							task_to_process = task_queue.first()->self();
							task_queue.remove(task_queue.first());
						}
						
						/*if (!task_to_process) {
							caller_pool_thread->awaited_task = task;

							if (flushing_cmd_queue) {
								flushing_cmd_queue->unlock();
							}
							caller_pool_thread->cond_var.wait(lock);
							if (flushing_cmd_queue) {
								flushing_cmd_queue->lock();
							}

							DEV_ASSERT(exit_threads || caller_pool_thread->signaled || task->completed);
							caller_pool_thread->awaited_task = nullptr;
						}*/
					}
				}

				if (task_to_process) {
					_process_task(task_to_process);
				}
			}
		}
		else {
			task->done_semaphore.wait();
			task_mutex.lock();
			task->waiting_user--;
			if (task->waiting_pool == 0 && task->waiting_user == 0) {
				tasks.erase(p_task_id);
				task_allocator.free(task);
			}
			task_mutex.unlock();
		}

		return OK;
	}

	WorkerThreadPool::GroupID WorkerThreadPool::_add_group_task(const Callable& p_callable, void (*p_func)(void*, uint32_t), void* p_userdata, BaseTemplateUserdata* p_template_userdata, int p_elements, int p_tasks, bool p_high_priority, const String& p_description) {
		ERR_FAIL_COND_V(p_elements < 0, INVALID_TASK_ID);
		if (p_tasks < 0) {
			p_tasks = MAX(1u, threads.size());
		}

		task_mutex.lock();
		Group* group = group_allocator.alloc();
		GroupID id = last_task++;
		group->max = p_elements;
		group->self = id;

		Task** tasks_posted = nullptr;
		if (p_elements == 0) {
			// Should really not call it with zero Elements, but at least it should work.
			group->completed.set_to(true);
			group->done_semaphore.post();
			group->tasks_used = 0;
			p_tasks = 0;
			if (p_template_userdata) {
				memdelete(p_template_userdata);
			}

		}
		else {
			group->tasks_used = p_tasks;
			tasks_posted = (Task**)alloca(sizeof(Task*) * p_tasks); // 只需要在栈上分配
			for (int i = 0; i < p_tasks; i++) {
				Task* task = task_allocator.alloc();
				task->native_group_func = p_func;
				task->native_func_userdata = p_userdata;
				task->description = p_description;
				task->group = group;
				task->callable = p_callable;
				task->template_userdata = p_template_userdata;
				tasks_posted[i] = task;
				// No task ID is used.
			}
		}

		groups[id] = group;

		_post_tasks_and_unlock(tasks_posted, p_tasks, p_high_priority);

		return id;
	}
	// @TODO read it
	void WorkerThreadPool::wait_for_group_task_completion(GroupID p_group) {
		task_mutex.lock();
		Group** groupp = groups.getptr(p_group);
		task_mutex.unlock();
		if (!groupp) {
			ERR_FAIL_MSG("Invalid Group ID.");
		}

		{
			Group* group = *groupp;

			/*if (flushing_cmd_queue) {
				flushing_cmd_queue->unlock();
			}*/
			group->done_semaphore.wait();
			/*if (flushing_cmd_queue) {
				flushing_cmd_queue->lock();
			}*/

			uint32_t max_users = group->tasks_used + 1; // Add 1 because the thread waiting for it is also user. Read before to avoid another thread freeing task after increment.
			uint32_t finished_users = group->finished.increment(); // fetch happens before inc, so increment later.

			if (finished_users == max_users) {
				// All tasks using this group are gone (finished before the group), so clear the group too.
				task_mutex.lock();
				group_allocator.free(group);
				task_mutex.unlock();
			}
		}

		task_mutex.lock(); // This mutex is needed when Physics 2D and/or 3D is selected to run on a separate thread.
		groups.erase(p_group);
		task_mutex.unlock();
	}

}