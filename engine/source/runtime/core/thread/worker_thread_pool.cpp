#include "worker_thread_pool.h"
namespace lain {
WorkerThreadPool::Task *const WorkerThreadPool::ThreadData::YIELDING = (Task *)1;
thread_local uintptr_t WorkerThreadPool::unlockable_mutexes[MAX_UNLOCKABLE_MUTEXES] = {};

WorkerThreadPool *WorkerThreadPool::singleton = nullptr;

WorkerThreadPool::WorkerThreadPool() {
	singleton = this;
}

WorkerThreadPool::~WorkerThreadPool() {
	finish();
}

void WorkerThreadPool::_lock_unlockable_mutexes() {
	for (uint32_t i = 0; i < MAX_UNLOCKABLE_MUTEXES; i++) {
		if (unlockable_mutexes[i]) {
			if ((((uintptr_t)unlockable_mutexes[i]) & 1) == 0) {
				((Mutex *)unlockable_mutexes[i])->lock();
			} else {
				((BinaryMutex *)(unlockable_mutexes[i] & ~1))->lock();
			}
		}
	}
}

void WorkerThreadPool::_unlock_unlockable_mutexes() {
	for (uint32_t i = 0; i < MAX_UNLOCKABLE_MUTEXES; i++) {
		if (unlockable_mutexes[i]) {
			if ((((uintptr_t)unlockable_mutexes[i]) & 1) == 0) {
				((Mutex *)unlockable_mutexes[i])->unlock();
			} else {
				((BinaryMutex *)(unlockable_mutexes[i] & ~1))->unlock(); // 用最后一位标记是否是BinaryMutex
			}
		}
	}
}

WorkerThreadPool::TaskID WorkerThreadPool::add_native_task(void (*p_func)(void*), void* p_userdata, bool p_high_priority, const String& p_description) {
  return _add_task(Callable(), p_func, p_userdata, nullptr, p_high_priority, p_description);
}
WorkerThreadPool::TaskID WorkerThreadPool::add_task(const Callable &p_action, bool p_high_priority, const String &p_description) {
	return _add_task(p_action, nullptr, nullptr, nullptr, p_high_priority, p_description);
}

bool WorkerThreadPool::is_task_completed(TaskID p_task_id) const {
	task_mutex.lock();
	const Task *const *taskp = tasks.getptr(p_task_id);
	if (!taskp) {
		task_mutex.unlock();
		ERR_FAIL_V_MSG(false, "Invalid Task ID"); // Invalid task
	}

	bool completed = (*taskp)->completed;
	task_mutex.unlock();

	return completed;
}

WorkerThreadPool::TaskID WorkerThreadPool::_add_task(const Callable &p_callable, void (*p_func)(void *), void *p_userdata, BaseTemplateUserdata *p_template_userdata, bool p_high_priority, const String &p_description) {
	task_mutex.lock();
	// Get a free task
	Task *task = task_allocator.alloc();
	TaskID id = last_task++;
	task->self = id;
	task->callable = p_callable;
	task->native_func = p_func;
	task->native_func_userdata = p_userdata;
	task->description = p_description;
	task->template_userdata = p_template_userdata;
	tasks.insert(id, task);

	_post_tasks_and_unlock(&task, 1, p_high_priority);

	return id;
}

void WorkerThreadPool::_post_tasks_and_unlock(Task **p_tasks, uint32_t p_count, bool p_high_priority) {
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

	ThreadData *caller_pool_thread = thread_ids.has(Thread::get_caller_id()) ? &threads[thread_ids[Thread::get_caller_id()]] : nullptr;

	for (uint32_t i = 0; i < p_count; i++) {
		p_tasks[i]->low_priority = !p_high_priority;
		if (p_high_priority || low_priority_threads_used < max_low_priority_threads) {
			task_queue.add_last(&p_tasks[i]->task_elem);
			if (!p_high_priority) {
				low_priority_threads_used++;
			}
			to_process++;
		} else {
			// Too many threads using low priority, must go to queue.
			low_priority_task_queue.add_last(&p_tasks[i]->task_elem);
			to_promote++;
		}
	}

	_notify_threads(caller_pool_thread, to_process, to_promote);

	task_mutex.unlock();
}

uint32_t WorkerThreadPool::thread_enter_unlock_allowance_zone(Mutex *p_mutex) {
	return _thread_enter_unlock_allowance_zone(p_mutex, false);
}

uint32_t WorkerThreadPool::thread_enter_unlock_allowance_zone(BinaryMutex *p_mutex) {
	return _thread_enter_unlock_allowance_zone(p_mutex, true);
}

void WorkerThreadPool::thread_exit_unlock_allowance_zone(uint32_t p_zone_id) {}

// 在等待p_tasks完成的过程中p_caller_pool_thread也可以执行其他任务
void WorkerThreadPool::_wait_collaboratively(ThreadData* p_caller_pool_thread, Task* p_task) {
	// Keep processing tasks until the condition to stop waiting is met.
#define IS_WAIT_OVER (unlikely(p_task == ThreadData::YIELDING) ? p_caller_pool_thread->yield_is_over : p_task->completed)
	while (true) {
		Task *task_to_process = nullptr;
		bool relock_unlockables = false;
		{
			MutexLock lock(task_mutex);
			bool was_signaled = p_caller_pool_thread->signaled;
			p_caller_pool_thread->signaled = false;

			if (IS_WAIT_OVER) {
				p_caller_pool_thread->yield_is_over = false;
				if (!exit_threads && was_signaled) {
					// This thread was awaken for some additional reason, but it's about to exit.
					// Let's find out what may be pending and forward the requests.
					uint32_t to_process = task_queue.first() ? 1 : 0;
					uint32_t to_promote = p_caller_pool_thread->current_task->low_priority && low_priority_task_queue.first() ? 1 : 0;
					if (to_process || to_promote) {
						// This thread must be left alone since it won't loop again.
						p_caller_pool_thread->signaled = true;
						_notify_threads(p_caller_pool_thread, to_process, to_promote);
					}
				}

				break;
			}

			if (!exit_threads) {
				if (p_caller_pool_thread->current_task->low_priority && low_priority_task_queue.first()) {
					if (_try_promote_low_priority_task()) {
						_notify_threads(p_caller_pool_thread, 1, 0);
					}
				}

				if (singleton->task_queue.first()) {
					task_to_process = task_queue.first()->self();
					task_queue.remove(task_queue.first());
				}

				if (!task_to_process) {
					p_caller_pool_thread->awaited_task = p_task;

					_unlock_unlockable_mutexes();
					relock_unlockables = true;
					p_caller_pool_thread->cond_var.wait(lock);

					DEV_ASSERT(exit_threads || p_caller_pool_thread->signaled || IS_WAIT_OVER);
					p_caller_pool_thread->awaited_task = nullptr;
				}
			}
		}

		if (relock_unlockables) {
			_lock_unlockable_mutexes();
		}

		if (task_to_process) {
			_process_task(task_to_process);
		}
	}

#undef IS_WAIT_OVER
}

uint32_t WorkerThreadPool::_thread_enter_unlock_allowance_zone(void* p_mutex, bool p_is_binary) {
  for (uint32_t i = 0; i < MAX_UNLOCKABLE_MUTEXES; i++) {
    if (unlikely((unlockable_mutexes[i] & ~1) == (uintptr_t)p_mutex)) {
      // Already registered in the current thread.
      return UINT32_MAX;
    }
    if (!unlockable_mutexes[i]) {
      unlockable_mutexes[i] = (uintptr_t)p_mutex;
      if (p_is_binary) {
        unlockable_mutexes[i] |= 1; // 用最后一位标记（可能会导致bug？）
      }
      return i;
    }
  }
  ERR_FAIL_V_MSG(UINT32_MAX, "No more unlockable mutex slots available. Engine bug.");
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
// 为什么都要靠singleton->来访问？
void WorkerThreadPool::_thread_function(void *p_user) {
	ThreadData *thread_data = (ThreadData *)p_user;
	while (true) {
		Task *task_to_process = nullptr;
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
			} else { // 没有任务，等待
				thread_data->cond_var.wait(lock);
				L_PRINT("notified", Thread::get_caller_id(), thread_data->index, thread_data->thread.get_id());
				DEV_ASSERT(singleton->exit_threads || thread_data->signaled); // 虚假唤醒
			}
		}
		// lock解锁
		// 执行任务
		if (task_to_process) {
			singleton->_process_task(task_to_process);
		}
	}
}
void WorkerThreadPool::_process_task(Task *p_task) {
	// 获得threadid，根据id找thread(pool_thread_index)
	// Thread::get_caller_id就能获得当前线程id吗？
	int pool_thread_index = thread_ids[Thread::get_caller_id()];
	ThreadData &curr_thread = threads[pool_thread_index];
	Task *prev_task = nullptr; // In case this is recursively called.
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
				break; // done，这边都是再来执行发现已经执行完的线程
			}
			if (p_task->native_group_func) {
				p_task->native_group_func(p_task->native_func_userdata, work_index);
			} else if (p_task->template_userdata) {
				p_task->template_userdata->callback_indexed(work_index);
			} else {
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
			p_task->group->completed.set_to(true);
			p_task->group->done_semaphore.post();
		}
		// 考虑到可能有一个额外的线程等着
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
	} else {
		if (p_task->native_func) {
			p_task->native_func(p_task->native_func_userdata);
		} else if (p_task->template_userdata) {
			p_task->template_userdata->callback();
			memdelete(p_task->template_userdata);
		} else {
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
		Task *low_prio_task = low_priority_task_queue.first()->self();
		low_priority_task_queue.remove(low_priority_task_queue.first());
		task_queue.add_last(&low_prio_task->task_elem);
		low_priority_threads_used++;
		return true;
	} else {
		return false;
	}
}
void WorkerThreadPool::_notify_threads(const ThreadData *p_current_thread_data, uint32_t p_process_count, uint32_t p_promote_count) {
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
		ThreadData &th = threads[notify_index];

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
		} else {
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
		ThreadData &th = threads[notify_index];

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
		SelfList<Task> *E = low_priority_task_queue.first();
		while (E) {
			L_CORE_ERROR(("Task waiting was never re-claimed: " + E->self()->description));
			E = E->next();
		}
	}

	{
		MutexLock lock(task_mutex);
		exit_threads = true;
	}
	for (ThreadData &data : threads) {
		data.cond_var.notify_one();
	}
	for (ThreadData &data : threads) {
		data.thread.wait_to_finish();
	}

	{
		MutexLock lock(task_mutex);
		for (KeyValue<TaskID, Task *> &E : tasks) {
			task_allocator.free(E.value);
		}
	}

	threads.clear();
}

Error WorkerThreadPool::wait_for_task_completion(TaskID p_task_id) {
	task_mutex.lock();
	Task **taskp = tasks.getptr(p_task_id);
	if (!taskp) {
		task_mutex.unlock();
		ERR_FAIL_V_MSG(ERR_INVALID_PARAMETER, "Invalid Task ID"); // Invalid task
	}
	Task *task = *taskp;
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
	ThreadData *caller_pool_thread = thread_ids.has(Thread::get_caller_id()) ? &threads[thread_ids[Thread::get_caller_id()]] : nullptr;
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
	} else {
		task->waiting_user++;
	}


	task_mutex.unlock();
	if (caller_pool_thread) {
		_wait_collaboratively(caller_pool_thread, task);
		task_mutex.lock(); 
		task->waiting_pool--;
		if (task->waiting_pool == 0 && task->waiting_user == 0) {
			tasks.erase(p_task_id);
			task_allocator.free(task);
		}
	} else {
		task->done_semaphore.wait();
		task_mutex.lock();
		task->waiting_user--;
		if (task->waiting_pool == 0 && task->waiting_user == 0) {
			tasks.erase(p_task_id);
			task_allocator.free(task);
		}
	}

	task_mutex.unlock();
	return OK;
}

WorkerThreadPool::GroupID WorkerThreadPool::add_native_group_task(void (*p_func)(void *, uint32_t), void *p_userdata, int p_elements, int p_tasks, bool p_high_priority, const String &p_description) {
	return _add_group_task(Callable(), p_func, p_userdata, nullptr, p_elements, p_tasks, p_high_priority, p_description);
}

WorkerThreadPool::GroupID WorkerThreadPool::add_group_task(const Callable &p_action, int p_elements, int p_tasks, bool p_high_priority, const String &p_description) {
	return _add_group_task(p_action, nullptr, nullptr, nullptr, p_elements, p_tasks, p_high_priority, p_description);
}



WorkerThreadPool::GroupID WorkerThreadPool::_add_group_task(const Callable &p_callable, void (*p_func)(void *, uint32_t), void *p_userdata, BaseTemplateUserdata *p_template_userdata, int p_elements, int p_tasks, bool p_high_priority, const String &p_description) {
	ERR_FAIL_COND_V(p_elements < 0, INVALID_TASK_ID);
	if (p_tasks < 0) {
		p_tasks = MIN(p_elements, threads.size());
	}

	task_mutex.lock();
	Group *group = group_allocator.alloc();
	GroupID id = last_task++;
	group->max = p_elements;
	group->self = id;

	Task **tasks_posted = nullptr;
	if (p_elements == 0) {
		// Should really not call it with zero Elements, but at least it should work.
		group->completed.set_to(true);
		group->done_semaphore.post();
		group->tasks_used = 0;
		p_tasks = 0;
		if (p_template_userdata) {
			memdelete(p_template_userdata);
		}

	} else {
		group->tasks_used = p_tasks;
		tasks_posted = (Task **)alloca(sizeof(Task *) * p_tasks); // 只需要在栈上分配
		for (int i = 0; i < p_tasks; i++) {
			Task *task = task_allocator.alloc();
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

void WorkerThreadPool::wait_for_group_task_completion(GroupID p_group) {
	// group task 就
	task_mutex.lock();
	Group **groupp = groups.getptr(p_group);
	task_mutex.unlock();
	if (!groupp) {
		ERR_FAIL_MSG("Invalid Group ID.");
	}

	{
		Group *group = *groupp;

		_unlock_unlockable_mutexes();
		group->done_semaphore.wait(); // 在group task 的时候不协作
		_lock_unlockable_mutexes();

		uint32_t max_users = group->tasks_used + 1; // Add 1 because the thread waiting for it is also user. Read before to avoid another thread freeing task after increment.
		uint32_t finished_users = group->finished.increment(); // fetch happens before inc, so increment later.

		if (finished_users == max_users) { // 
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

void WorkerThreadPool::yield() {
	int th_index = get_thread_index();
	ERR_FAIL_COND_MSG(th_index == -1, "This function can only be called from a worker thread.");
	_wait_collaboratively(&threads[th_index], ThreadData::YIELDING); // 这样 is_work_over 被设置为 该线程的 yield_is_over
	// 除非 yield_is_over 被设置为true，否则不会退出
}

void WorkerThreadPool::notify_yield_over(TaskID p_task_id) {
	task_mutex.lock();
	Task **taskp = tasks.getptr(p_task_id);
	if (!taskp) {
		task_mutex.unlock();
		ERR_FAIL_MSG("Invalid Task ID.");
	}
	Task *task = *taskp;
	if (task->pool_thread_index == -1) { // Completed or not started yet.
		if (!task->completed) {
			// This avoids a race condition where a task is created and yield-over called before it's processed.
			task->pending_notify_yield_over = true;
		}
		task_mutex.unlock();
		return;
	}

	ThreadData &td = threads[task->pool_thread_index];
	td.yield_is_over = true; // 在这里设置
	td.signaled = true;
	td.cond_var.notify_one();

	task_mutex.unlock();
}

bool WorkerThreadPool::is_group_task_completed(GroupID p_group) const {
	task_mutex.lock();
	const Group *const *groupp = groups.getptr(p_group);
	if (!groupp) {
		task_mutex.unlock();
		ERR_FAIL_V_MSG(false, "Invalid Group ID");
	}
	bool completed = (*groupp)->completed.is_set();
	task_mutex.unlock();
	return completed;
}

int WorkerThreadPool::get_thread_index() {
	Thread::ID tid = Thread::get_caller_id();
	return singleton->thread_ids.has(tid) ? singleton->thread_ids[tid] : -1;
}


} //namespace lain