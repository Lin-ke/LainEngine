#pragma once
#ifndef WORKER_THREAD_POOL_H
#define WORKER_THREAD_POOL_H

#include "core/io/rid.h"
#include "core/object/safe_refcount.h"
#include "core/os/condition_variable.h"
#include "core/os/memory.h"
#include "core/os/os.h"
#include "core/os/semaphore.h"
#include "core/os/thread.h"
#include "core/templates/local_vector.h"
#include "core/templates/paged_allocator.h"
#include "core/templates/self_list.h"

namespace lain {
class CommandQueueMT;
class WorkerThreadPool : public Object {
  LCLASS(WorkerThreadPool, Object);

 public:
  enum { INVALID_TASK_ID = -1 };

  typedef int64_t TaskID;
  typedef int64_t GroupID;

 private:
  struct Task;

  struct BaseTemplateUserdata {
    virtual void callback() {}
    virtual void callback_indexed(uint32_t p_index) {}
    virtual ~BaseTemplateUserdata() {}
  };
	// ÿ��ִ��ֻ��index������ִ�д�������PTG�ĸ���
  struct Group {
    GroupID self = -1;
    SafeNumeric<uint32_t> index; // ÿ��ִ�е�index
    SafeNumeric<uint32_t> completed_index;
    uint32_t max = 0; // �м�������
    Semaphore done_semaphore;  // ���̵߳�������ź�����
    SafeFlag completed;
    SafeNumeric<uint32_t> finished;
    uint32_t tasks_used = 0; // �߳�ִ�е�����
  };

  struct Task {
    TaskID self = -1;
    Callable callable;
    void (*native_func)(void*) = nullptr;
    void (*native_group_func)(void*, uint32_t) = nullptr;
    void* native_func_userdata = nullptr;
    String description;
    Semaphore done_semaphore;  // For user threads awaiting.
    bool completed = false;
	bool pending_notify_yield_over = false;

    Group* group = nullptr;  // �������ɶ���߳�ִ��
    SelfList<Task> task_elem;
    uint32_t waiting_pool = 0;
    uint32_t waiting_user = 0;
    bool low_priority = false;
    BaseTemplateUserdata* template_userdata = nullptr;
    int pool_thread_index = -1;

    void free_template_userdata();
    Task() : task_elem(this) {}
  };
  static const uint32_t TASKS_PAGE_SIZE = 1024;
  static const uint32_t GROUPS_PAGE_SIZE = 256;
  PagedAllocator<Task, false, TASKS_PAGE_SIZE> task_allocator;
  PagedAllocator<Group, false, GROUPS_PAGE_SIZE> group_allocator;
  SelfList<Task>::List low_priority_task_queue;
  SelfList<Task>::List task_queue;

  BinaryMutex task_mutex;
  struct ThreadData {
	static Task *const YIELDING; // Too bad constexpr doesn't work here.
	// ����ֵ���޸ģ����ǲ��������ָ��仯

    uint32_t index = 0;
    Thread thread;
    bool ready_for_scripting = false;
    bool signaled = false;
	bool yield_is_over = false;

    Task* current_task = nullptr;
    Task* awaited_task = nullptr;  // Null if not awaiting the condition variable. Special value for idle-waiting.


    ConditionVariable cond_var;    // �������������ÿ���̳߳��У����notify_one����֪ͨ���߳�
  };

  TightLocalVector<ThreadData> threads;
  bool exit_threads = false;

  HashMap<Thread::ID, int> thread_ids; // id��index
  HashMap<TaskID, Task*, HashMapHasherDefault, HashMapComparatorDefault<TaskID>, PagedAllocator<HashMapElement<TaskID, Task*>, false, TASKS_PAGE_SIZE>> tasks;
  HashMap<GroupID, Group*, HashMapHasherDefault, HashMapComparatorDefault<GroupID>, PagedAllocator<HashMapElement<GroupID, Group*>, false, GROUPS_PAGE_SIZE>> groups;

  uint32_t max_low_priority_threads = 0;
  uint32_t low_priority_threads_used = 0;
  uint32_t notify_index = 0;  // For rotating across threads, no help distributing load.

  uint64_t last_task = 1;

  static void _thread_function(void* p_user);

  void _process_task(Task* task);

  void _post_tasks_and_unlock(Task** p_tasks, uint32_t p_count, bool p_high_priority);
  void _notify_threads(const ThreadData* p_current_thread_data, uint32_t p_process_count, uint32_t p_promote_count);

  bool _try_promote_low_priority_task();

  static WorkerThreadPool* singleton;
  static const constexpr uint32_t MAX_UNLOCKABLE_MUTEXES = 2;
  static thread_local uintptr_t unlockable_mutexes[MAX_UNLOCKABLE_MUTEXES]; // ��ֻ��һ�������� unlockable_mutexes��thread_local��

  TaskID _add_task(const Callable& p_callable, void (*p_func)(void*), void* p_userdata, BaseTemplateUserdata* p_template_userdata, bool p_high_priority,
                   const String& p_description);
  GroupID _add_group_task(const Callable& p_callable, void (*p_func)(void*, uint32_t), void* p_userdata, BaseTemplateUserdata* p_template_userdata, int p_elements,
                          int p_tasks, bool p_high_priority, const String& p_description);
  
  void _wait_collaboratively(ThreadData *p_caller_pool_thread, Task *p_task);
  static uint32_t _thread_enter_unlock_allowance_zone(void *p_mutex, bool p_is_binary);

  template <class C, class M, class U>
  struct TaskUserData : public BaseTemplateUserdata {
    C* instance;
    M method;
    U userdata;
    virtual void callback() override { (instance->*method)(userdata); }
  };

  template <class C, class M, class U>
  struct GroupUserData : public BaseTemplateUserdata {
    C* instance;
    M method;
    U userdata;
    virtual void callback_indexed(uint32_t p_index) override { (instance->*method)(p_index, userdata); }
  };
void _lock_unlockable_mutexes();
void _unlock_unlockable_mutexes();
 protected:
  static void _bind_methods();

 public:
  template <class C, class M, class U>
  TaskID add_template_task(C* p_instance, M p_method, U p_userdata, bool p_high_priority = false, const String& p_description = String()) {
    typedef TaskUserData<C, M, U> TUD;
    TUD* ud = memnew(TUD);
    ud->instance = p_instance;
    ud->method = p_method;
    ud->userdata = p_userdata;
    return _add_task(Callable(), nullptr, nullptr, ud, p_high_priority, p_description);
  }
  TaskID add_native_task(void (*p_func)(void*), void* p_userdata, bool p_high_priority = false, const String& p_description = String());
  TaskID add_task(const Callable& p_action, bool p_high_priority = false, const String& p_description = String());

  bool is_task_completed(TaskID p_task_id) const;
  Error wait_for_task_completion(TaskID p_task_id);

  template <class C, class M, class U>
  GroupID add_template_group_task(C* p_instance, M p_method, U p_userdata, int p_elements, int p_tasks = -1, bool p_high_priority = false,
                                  const String& p_description = String()) {
    typedef GroupUserData<C, M, U> GroupUD;
    GroupUD* ud = memnew(GroupUD);
    ud->instance = p_instance;
    ud->method = p_method;
    ud->userdata = p_userdata;
    return _add_group_task(Callable(), nullptr, nullptr, ud, p_elements, p_tasks, p_high_priority, p_description);
  }
  GroupID add_native_group_task(void (*p_func)(void*, uint32_t), void* p_userdata, int p_elements, int p_tasks = -1, bool p_high_priority = false,
                                const String& p_description = String());
  GroupID add_group_task(const Callable& p_action, int p_elements, int p_tasks = -1, bool p_high_priority = false, const String& p_description = String());
  uint32_t get_group_processed_element_count(GroupID p_group) const;
  bool is_group_task_completed(GroupID p_group) const;
  void wait_for_group_task_completion(GroupID p_group);
  // ֪ͨ��������̵߳�yield����
  void notify_yield_over(TaskID p_task_id);

  _FORCE_INLINE_ int get_thread_count() const { return threads.size(); }

  static WorkerThreadPool* get_singleton() { return singleton; }
  static int get_thread_index();

  static uint32_t thread_enter_unlock_allowance_zone(Mutex* p_mutex);
  static uint32_t thread_enter_unlock_allowance_zone(BinaryMutex* p_mutex);
  static void thread_exit_unlock_allowance_zone(uint32_t p_zone_id);


  void init(int p_thread_count = -1, float p_low_priority_task_ratio = 0.3);
  void finish();
  WorkerThreadPool();
  ~WorkerThreadPool();
};
}  // namespace lain
#endif