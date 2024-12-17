#include "scene_tree.h"
#include "core/scene/component/component.h"
#include "core/scene/object/gobject.h"
#include "core/scene/packed_scene.h"
#include "core/thread/worker_thread_pool.h"
#include "scene/main/viewport.h"
namespace lain {
SceneTree* SceneTree::singleton = nullptr;
SceneTree::SceneTree() {
  if (singleton == nullptr) {
    singleton = this;
  }
  current_scene = nullptr;
  root = memnew(Viewport);  // 这是个window
  root->set_name("root");
  root->set_process_mode(TickObject::PROCESS_MODE_ALWAYS);
  if (!root->get_world_3d().is_valid()) {
    root->set_world_3d(Ref<World3D>(memnew(World3D)));
  }
  // 在这里设置屏幕相关数据
  // MSAA等等
  process_groups.push_back(&default_process_group);
}
SceneTree::~SceneTree() {
  singleton = nullptr;
}
void SceneTree::initialize() {
  ERR_FAIL_NULL(root);
  MainLoop::initialize();
  root->_set_tree(this);
}

SceneTree::Group* SceneTree::add_to_group(const StringName& p_group, GObject* p_node) {
  _THREAD_SAFE_METHOD_

  HashMap<StringName, Group>::Iterator E = group_map.find(p_group);
  if (!E) {
    E = group_map.insert(p_group, Group());
  }

  ERR_FAIL_COND_V_MSG(E->value.nodes.has(p_node), &E->value, "Already in group: " + p_group + ".");
  E->value.nodes.push_back(p_node);
  //E->value.last_tree_version=0;
  E->value.changed = true;
  return &E->value;
}
void SceneTree::remove_from_group(const StringName &p_group, GObject *p_node) {
	_THREAD_SAFE_METHOD_

	HashMap<StringName, Group>::Iterator E = group_map.find(p_group);
	ERR_FAIL_COND(!E);

	E->value.nodes.erase(p_node);
	if (E->value.nodes.is_empty()) {
		group_map.remove(E);
	}
}


void SceneTree::finalize() {
  if (root) {
    root->_set_tree(nullptr);
    root->_propagate_after_exit_tree();
    memdelete(root);  //delete root
    root = nullptr;

    // In case deletion of some objects was queued when destructing the `root`.
    // E.g. if `queue_free()` was called for some node outside the tree when handling NOTIFICATION_PREDELETE for some node in the tree.
    //_flush_delete_queue();
  }
  MainLoop::finalize();
}

bool SceneTree::process(double p_time) {
  root_lock++;

  if (MainLoop::process(p_time)) {
    _quit = true;
  }
  
  process_time = p_time;

  /*if (multiplayer_poll) {
			multiplayer->poll();
			for (KeyValue<GObjectPath, Ref<MultiplayerAPI>>& E : custom_multiplayers) {
				E.value->poll();
			}
		}

		emit_signal(SNAME("process_frame"));*/

  //MessageQueue::get_singleton()->flush(); //small little hack

  flush_transform_notifications();

  _process(false);

  //_flush_ugc();
  //MessageQueue::get_singleton()->flush(); //small little hack
  // 这个可以传给一些需要根据changed transform 调整 RS内部资源的类
  flush_transform_notifications();  //transforms after world update, to avoid unnecessary enter/exit notifications

  root_lock--;

  /*_flush_delete_queue();

		if (unlikely(pending_new_scene)) {
			_flush_scene_change();
		}*/

  //process_timers(p_time, false); //go through timers

  //process_tweens(p_time, false);

  //_call_idle_callbacks();

#ifdef TOOLS_ENABLED
#ifndef _3D_DISABLED
  if (Engine::get_singleton()->is_editor_hint()) {
    //simple hack to reload fallback environment if it changed from editor
    String env_path = GLOBAL_GET(SNAME("rendering/environment/defaults/default_environment"));
    env_path = env_path.strip_edges();  //user may have added a space or two
    String cpath;
    Ref<Environment> fallback = get_root()->get_world_3d()->get_fallback_environment();
    if (fallback.is_valid()) {
      cpath = fallback->get_path();
    }
    if (cpath != env_path) {
      if (!env_path.is_empty()) {
        fallback = ResourceLoader::load(env_path);
        if (fallback.is_null()) {
          //could not load fallback, set as empty
          ProjectSettings::get_singleton()->set("rendering/environment/defaults/default_environment", "");
        }
      } else {
        fallback.unref();
      }
      get_root()->get_world_3d()->set_fallback_environment(fallback);
    }
  }
#endif  // _3D_DISABLED
#endif  // TOOLS_ENABLED

  return _quit;
}

void SceneTree::_process(bool p_physics) {
  if (process_groups_dirty) {
    {
      // First, remove dirty groups.
      // This needs to be done when not processing to avoid problems.
      ProcessGroup** pg_ptr = (ProcessGroup**)process_groups.ptr();  // discard constness.
      uint32_t pg_count = process_groups.size();
      // # 去掉需要remove 的
      for (uint32_t i = 0; i < pg_count; i++) {
        if (pg_ptr[i]->removed) {
          // Replace removed with last.
          pg_ptr[i] = pg_ptr[pg_count - 1];
          // Retry
          i--;
          pg_count--;
        }
      }
      if (pg_count != process_groups.size()) {
        process_groups.resize(pg_count);
      }
    }
    {
      // Then, re-sort groups.
      process_groups.sort_custom<ProcessGroupSort>();
    }

    process_groups_dirty = false;
  }

  // Cache the group count, because during processing new groups may be added.
  // They will be added at the end, hence for consistency they will be ignored by this process loop.
  // No group will be removed from the array during processing (this is done earlier in this function by marking the groups dirty).
  uint32_t group_count = process_groups.size();

  if (group_count == 0) {
    return;
  }

  process_last_pass++;  // Increment pass
  uint32_t from = 0;
  uint32_t process_count = 0;
  nodes_removed_on_group_call_lock++;

  int current_order = process_groups[0]->owner ? process_groups[0]->owner->tickdata.process_thread_group_order : 0;
  bool current_threaded = process_groups[0]->owner ? process_groups[0]->owner->tickdata.process_thread_group == GObject::PROCESS_THREAD_GROUP_SUB_THREAD : false;

  for (uint32_t i = 0; i <= group_count; i++) {
    int order = i < group_count && process_groups[i]->owner ? process_groups[i]->owner->tickdata.process_thread_group_order : 0;
    bool threaded = i < group_count && process_groups[i]->owner ? process_groups[i]->owner->tickdata.process_thread_group == GObject::PROCESS_THREAD_GROUP_SUB_THREAD : false;
    // 结算之前的
    if (i == group_count || current_order != order || current_threaded != threaded) {
      if (process_count > 0) {
        // Proceed to process the group.
        bool using_threads =
            process_groups[from]->owner && process_groups[from]->owner->tickdata.process_thread_group == GObject::PROCESS_THREAD_GROUP_SUB_THREAD && !node_threading_disabled;

        if (using_threads) {
          local_process_group_cache.clear();
        }
        for (uint32_t j = from; j < i; j++) {
          if (process_groups[j]->last_pass == process_last_pass) {
            if (using_threads) {
              local_process_group_cache.push_back(process_groups[j]);
            } else {
              _process_group(process_groups[j], p_physics);
            }
          }
        }

        if (using_threads) {
          WorkerThreadPool::GroupID id =
              WorkerThreadPool::get_singleton()->add_template_group_task(this, &SceneTree::_process_groups_thread, p_physics, local_process_group_cache.size(), -1, true);
          WorkerThreadPool::get_singleton()->wait_for_group_task_completion(id);
        }
      }

      if (i == group_count) {
        // This one is invalid, no longer process
        break;
      }

      from = i;
      current_threaded = threaded;
      current_order = order;
    }

    if (process_groups[i]->removed) {
      continue;
    }

    ProcessGroup* pg = process_groups[i];

    // Validate group for processing
    bool process_valid = false;
    if (p_physics) {
      if (!pg->physics_nodes.is_empty()) {
        process_valid = true;
      } else if (pg == &default_process_group) {
        process_valid = true;
      }
      /*else if ((pg == &default_process_group || (pg->owner != nullptr && pg->owner->data.process_thread_messages.has_flag(GObject::FLAG_PROCESS_THREAD_MESSAGES_PHYSICS))) && pg->call_queue.has_messages()) {
					process_valid = true;
				}*/
    } else {

      if (!pg->nodes.is_empty()) {
        process_valid = true;
      } else if (pg == &default_process_group) {
        process_valid = true;
      }
      /*else if ((pg == &default_process_group || (pg->owner != nullptr && pg->owner->data.process_thread_messages.has_flag(GObject::FLAG_PROCESS_THREAD_MESSAGES))) && pg->call_queue.has_messages()) {
					process_valid = true;
				}*/
    }

    if (process_valid) {
      pg->last_pass = process_last_pass;  // Enable for processing
      process_count++;
    }
  }

  nodes_removed_on_group_call_lock--;
  if (nodes_removed_on_group_call_lock == 0) {
    nodes_removed_on_group_call.clear();
  }
}

void SceneTree::_process_groups_thread(uint32_t p_index, bool p_physics) {
  GObject::current_process_thread_group = local_process_group_cache[p_index]->owner;
  _process_group(local_process_group_cache[p_index], p_physics);
  GObject::current_process_thread_group = nullptr;
}

void SceneTree::_process_group(ProcessGroup* p_group, bool p_physics) {
  // When reading this function, keep in mind that this code must work in a way where
  // if any node is removed, this needs to continue working.

  //p_group->call_queue.flush(); // Flush messages before processing.

  Vector<TickObject*>& nodes = p_physics ? p_group->physics_nodes : p_group->nodes;
  if (nodes.is_empty()) {
    return;
  }

  if (p_physics) {
    if (p_group->physics_node_order_dirty) {
      nodes.sort_custom<TickObject::ComparatorWithPhysicsPriority>();
      p_group->physics_node_order_dirty = false;
    }
  } else {
    if (p_group->node_order_dirty) {
      nodes.sort_custom<TickObject::ComparatorWithPriority>();
      p_group->node_order_dirty = false;
    }
  }

  // Make a copy, so if nodes are added/removed from process, this does not break
  Vector<TickObject*> nodes_copy = nodes;

  uint32_t node_count = nodes_copy.size();
  TickObject** nodes_ptr = (TickObject**)nodes_copy.ptr();  // Force cast, pointer will not change.

  for (uint32_t i = 0; i < node_count; i++) {
    TickObject* n = nodes_ptr[i];
    if (nodes_removed_on_group_call.has(n)) {
      // GObject may have been removed during process, skip it.
      // Keep in mind removals can only happen on the main thread.
      continue;
    }
    // can_process will check is_inside_tree
    // is virtual function works here?
    if (!n->can_process()) {
      continue;
    }

    if (p_physics) {
      if (n->is_physics_processing_internal()) {
        n->notification(GObject::NOTIFICATION_INTERNAL_PHYSICS_PROCESS);
      }
      if (n->is_physics_processing()) {
        n->notification(GObject::NOTIFICATION_PHYSICS_PROCESS);
      }
    } else {
      if (n->is_processing_internal()) {
        n->notification(GObject::NOTIFICATION_INTERNAL_PROCESS);
      }
      if (n->is_processing()) {
        n->notification(GObject::NOTIFICATION_PROCESS);
      }
    }
  }

  //p_group->call_queue.flush(); // Flush messages also after processing (for potential deferred calls).
}

bool SceneTree::ProcessGroupSort::operator()(const ProcessGroup* p_left, const ProcessGroup* p_right) const {
  int left_order = p_left->owner ? p_left->owner->tickdata.process_thread_group_order : 0;
  int right_order = p_right->owner ? p_right->owner->tickdata.process_thread_group_order : 0;

  if (left_order == right_order) {
    int left_threaded = p_left->owner != nullptr && p_left->owner->tickdata.process_thread_group == GObject::PROCESS_THREAD_GROUP_SUB_THREAD ? 0 : 1;
    int right_threaded = p_right->owner != nullptr && p_right->owner->tickdata.process_thread_group == GObject::PROCESS_THREAD_GROUP_SUB_THREAD ? 0 : 1;
    return left_threaded < right_threaded;
  } else {
    return left_order < right_order;
  }
}

void SceneTree::_remove_process_group(TickObject* p_node) {
  _THREAD_SAFE_METHOD_
  ProcessGroup* pg = (ProcessGroup*)p_node->tickdata.process_group;
  ERR_FAIL_NULL(pg);
  ERR_FAIL_COND(pg->removed);
  pg->removed = true;
  pg->owner = nullptr;
  p_node->tickdata.process_group = nullptr;
  process_groups_dirty = true;
}

void SceneTree::_add_process_group(TickObject* p_node) {
  _THREAD_SAFE_METHOD_
  ERR_FAIL_NULL(p_node);

  ProcessGroup* pg = memnew(ProcessGroup);

  pg->owner = p_node;
  p_node->tickdata.process_group = pg;

  process_groups.push_back(pg);

  process_groups_dirty = true;
  // 并不加入
}

void SceneTree::_remove_node_from_process_group(TickObject* p_node, TickObject* p_owner) {
  _THREAD_SAFE_METHOD_
  ProcessGroup* pg = p_owner ? (ProcessGroup*)p_owner->tickdata.process_group : &default_process_group;

  if (p_node->is_processing() || p_node->is_processing_internal()) {
    bool found = pg->nodes.erase(p_node);
    ERR_FAIL_COND(!found);
  }

  if (p_node->is_physics_processing() || p_node->is_physics_processing_internal()) {
    bool found = pg->physics_nodes.erase(p_node);
    ERR_FAIL_COND(!found);
  }
}

void SceneTree::_add_node_to_process_group(TickObject* p_node, TickObject* p_owner) {
  _THREAD_SAFE_METHOD_
  ProcessGroup* pg = p_owner ? (ProcessGroup*)p_owner->tickdata.process_group : &default_process_group;

  if (p_node->is_processing() || p_node->is_processing_internal()) {
    pg->nodes.push_back(p_node);
    pg->node_order_dirty = true;
  }

  if (p_node->is_physics_processing() || p_node->is_physics_processing_internal()) {
    pg->physics_nodes.push_back(p_node);
    pg->physics_node_order_dirty = true;
  }
}

void SceneTree::flush_transform_notifications() {
  _THREAD_SAFE_METHOD_

  SelfList<GObject>* n = xform_change_list.first();
  while (n) {
    GObject* node = n->self();
    SelfList<GObject>* nx = n->next();
    xform_change_list.remove(n);
    n = nx;
    node->notification(NOTIFICATION_TRANSFORM_CHANGED);
  }
}
/// signal method
void SceneTree::node_removed(GObject* p_node) {
  if (current_scene == p_node) {
    current_scene = nullptr;
  }
}
}  // namespace lain