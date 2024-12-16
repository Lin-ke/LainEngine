#include "component.h"
namespace lain {
    void Component::_notification(int p_notification) {
        switch (p_notification) {
        case GObject::NOTIFICATION_PROCESS: {
        }break;
        case GObject::NOTIFICATION_ENTER_TREE: {
            ERR_FAIL_NULL(m_parent);
            TickData& parent_tickdata = m_parent->tickdata;
            if (tickdata.process_mode == PROCESS_MODE_INHERIT) {
                tickdata.process_owner = parent_tickdata.process_owner;
            }
            else {
                tickdata.process_owner = this;
            }
            if (tickdata.process_thread_group == PROCESS_THREAD_GROUP_INHERIT) {
                tickdata.process_thread_group_owner = parent_tickdata.process_thread_group_owner;
                tickdata.process_group = parent_tickdata.process_group;
            }
            else {
                tickdata.process_thread_group_owner = this;
                _add_process_group();
            }
            if (is_any_processing()) {
                _add_to_process_thread_group();
            }
            m_parent->get_tree()->nodes_in_tree_count++;
        } break;
        case GObject::NOTIFICATION_EXIT_TREE: {
            ERR_FAIL_NULL(m_parent);
            m_parent->get_tree()->nodes_in_tree_count--;
            if (is_any_processing()) {
                _remove_from_process_thread_group();
            }
            if (tickdata.process_thread_group_owner == this) {
                _remove_process_group();
            }
            tickdata.process_thread_group_owner = nullptr;
            tickdata.process_owner = nullptr;

        } break;
        case GObject::NOTIFICATION_PARENTED: {
            return;
        } break;


        default:{
            return;

        }
        }
    }
}