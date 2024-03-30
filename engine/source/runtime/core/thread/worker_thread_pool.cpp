#include "worker_thread_pool.h"
namespace lain {
	WorkerThreadPool::WorkerThreadPool() {
		singleton = this;
	}

	WorkerThreadPool::~WorkerThreadPool() {
		finish();
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
				L_CORE_ERROR("Task waiting was never re-claimed: " + E->self()->description);
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
}