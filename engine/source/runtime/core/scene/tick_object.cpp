#include "tick_object.h"
#include "core/scene/scene_tree.h"
namespace lain {
thread_local TickObject* TickObject::current_process_thread_group = nullptr;

void TickObject::set_process_priority(int p_priority) {
  if (tickdata.process_priority == p_priority) {
    return;
  }
  if (!is_inside_tree()) {
    // Not yet in the tree; trivial update.
    tickdata.process_priority = p_priority;
    return;
  }

  if (is_any_processing()) {
    _remove_from_process_thread_group();
  }

  tickdata.process_priority = p_priority;

  if (is_any_processing()) {
    _add_to_process_thread_group();
  }
}
void TickObject::_remove_from_process_thread_group() {
  get_tree()->_remove_node_from_process_group(this, tickdata.process_thread_group_owner);
}
void TickObject::_add_to_process_thread_group() {
  get_tree()->_add_node_to_process_group(this, tickdata.process_thread_group_owner);
}
void TickObject::_remove_process_group() {
  get_tree()->_remove_process_group(this);
}
void TickObject::_add_process_group() {
  get_tree()->_add_process_group(this);
}
bool TickObject::can_process() const {
  ERR_FAIL_COND_V(!is_inside_tree(), false);
  return _can_process(get_tree()->is_paused());
}
bool TickObject::_can_process(bool p_is_paused) const {
  ProcessMode process_mode;

  if (tickdata.process_mode == PROCESS_MODE_INHERIT) {
    if (!tickdata.process_owner) {
      process_mode = PROCESS_MODE_PAUSABLE;
    } else {
      process_mode = tickdata.process_owner->tickdata.process_mode;
    }
  } else {
    process_mode = tickdata.process_mode;
  }

  // The owner can't be set to inherit, must be a bug.
  ERR_FAIL_COND_V(process_mode == PROCESS_MODE_INHERIT, false);

  if (process_mode == PROCESS_MODE_DISABLED) {
    return false;
  } else if (process_mode == PROCESS_MODE_ALWAYS) {
    return true;
  }

  if (p_is_paused) {
    return process_mode == PROCESS_MODE_WHEN_PAUSED;
  } else {
    return process_mode == PROCESS_MODE_PAUSABLE;
  }
}

void TickObject::set_process(bool p_process) {
  //ERR_THREAD_GUARD
  if (tickdata.process == p_process) {
    return;
  }

  if (!is_inside_tree()) {
    tickdata.process = p_process;
    return;
  }

  if (is_any_processing()) {
    _remove_from_process_thread_group();  // 这并不会导致ptg的移除
  }

  tickdata.process = p_process;

  if (is_any_processing()) {
    _add_to_process_thread_group();
  }
}


}  // namespace lain