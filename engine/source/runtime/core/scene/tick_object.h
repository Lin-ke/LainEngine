#pragma once
#ifndef TICK_OBJECT_H
#define TICK_OBJECT_H
#include "core/object/object.h"
namespace lain {

    REFLECTION_TYPE(TickObject)
    CLASS(TickObject: public Object, WhiteListFields) {
        REFLECTION_BODY(TickObject)

        LCLASS(TickObject, Object);

        friend class SceneTree;

    public:
        enum ProcessThreadGroup {
            PROCESS_THREAD_GROUP_INHERIT,
            PROCESS_THREAD_GROUP_MAIN_THREAD,
            PROCESS_THREAD_GROUP_SUB_THREAD,
        }; // 处理线程
        enum ProcessMode : unsigned int {
            PROCESS_MODE_INHERIT, // same as parent node
            PROCESS_MODE_PAUSABLE, // process only if not paused
            PROCESS_MODE_WHEN_PAUSED, // process only if paused
            PROCESS_MODE_ALWAYS, // process always
            PROCESS_MODE_DISABLED, // never process
        };// 处理方法
        struct ComparatorWithPriority {
            bool operator()(const TickObject* p_a, const TickObject* p_b) const { return p_b->tickdata.process_priority > p_a->tickdata.process_priority; }
        };
        // 如果没有优先级就保持加入顺序？但是有个深度顺序，不知道是否重要？？
        struct ComparatorWithPhysicsPriority {
            bool operator()(const TickObject* p_a, const TickObject* p_b) const { return p_b->tickdata.physics_process_priority > p_a->tickdata.physics_process_priority; }
        };


        struct TickData {

            // 节点处理顺序
              /// process bools
            bool physics_process : 1;
            bool process : 1;

            bool physics_process_internal : 1; // internal：则启用该节点的内部处理。内部处理独立于正常的 _process 调用而发生，
            bool process_internal : 1;
            int process_thread_group_order = 0;
            ProcessThreadGroup process_thread_group = PROCESS_THREAD_GROUP_INHERIT;
            TickObject* process_thread_group_owner = nullptr;
            void* process_group = nullptr; // to avoid cyclic dependency
            // 是否处理
            ProcessMode process_mode : ProcessMode::PROCESS_MODE_PAUSABLE; // mode
            TickObject* process_owner = nullptr;
            // 排序接口
            int process_priority = 0;
            int physics_process_priority = 0;

        } tickdata;
        // must
        bool can_process() const;
        L_INLINE virtual bool is_inside_tree() const { return false; }
        L_INLINE virtual String get_description() const {return "TickObject";}
        L_INLINE bool is_physics_processing_internal() const { return tickdata.physics_process_internal; }
        L_INLINE bool is_physics_processing() const { return tickdata.physics_process; }
        L_INLINE bool is_processing_internal() const { return tickdata.process_internal; }
        L_INLINE bool is_processing() const { return tickdata.process; }
        L_INLINE bool is_any_processing() const {
            return tickdata.process || tickdata.process_internal || tickdata.physics_process || tickdata.physics_process_internal;
        }

        static thread_local TickObject* current_process_thread_group;

        _FORCE_INLINE_ static bool is_group_processing() { return current_process_thread_group; }
        
        L_INLINE void set_ptg_owner(TickObject* p_owner) { tickdata.process_thread_group_owner = p_owner; }
        void set_process_priority(int p_priority);
        virtual SceneTree* get_tree()const { return nullptr; }
        void set_process(bool p_process);
        // 没办法内联，因为get_tree() 
        void _remove_from_process_thread_group();
        void _add_to_process_thread_group();
        void _remove_process_group();
        void _add_process_group();
        bool _can_process(bool) const;

        L_INLINE bool is_accessible_from_caller_thread() const {
			return current_process_thread_group == tickdata.process_thread_group_owner;
        }
    	_FORCE_INLINE_ bool is_readable_from_caller_thread() const {
		if (current_process_thread_group == nullptr) {
			// No thread processing.
			// Only accessible if node is outside the scene tree
			// or access will happen from a node-safe thread.
			// return is_current_thread_safe_for_nodes() || unlikely(!data.inside_tree);
            return unlikely(!is_inside_tree());
		} else {
			// Thread processing.
			return true;
		}
	}

    };
}
#ifdef DEBUG_ENABLED
#define ERR_THREAD_GUARD ERR_FAIL_COND_MSG(!is_accessible_from_caller_thread(), vformat("Caller thread can't call this function in this node (%s). Use call_deferred() or call_thread_group() instead.", get_description()));
#define ERR_THREAD_GUARD_V(m_ret) ERR_FAIL_COND_V_MSG(!is_accessible_from_caller_thread(), (m_ret), vformat("Caller thread can't call this function in this node (%s). Use call_deferred() or call_thread_group() instead.", get_description()));
#define ERR_MAIN_THREAD_GUARD ERR_FAIL_COND_MSG(is_inside_tree() && !is_current_thread_safe_for_nodes(), vformat("This function in this node (%s) can only be accessed from the main thread. Use call_deferred() instead.", get_description()));
#define ERR_MAIN_THREAD_GUARD_V(m_ret) ERR_FAIL_COND_V_MSG(is_inside_tree() && !is_current_thread_safe_for_nodes(), (m_ret), vformat("This function in this node (%s) can only be accessed from the main thread. Use call_deferred() instead.", get_description()));
#define ERR_READ_THREAD_GUARD ERR_FAIL_COND_MSG(!is_readable_from_caller_thread(), vformat("This function in this node (%s) can only be accessed from either the main thread or a thread group. Use call_deferred() instead.", get_description()));
#define ERR_READ_THREAD_GUARD_V(m_ret) ERR_FAIL_COND_V_MSG(!is_readable_from_caller_thread(), (m_ret), vformat("This function in this node (%s) can only be accessed from either the main thread or a thread group. Use call_deferred() instead.", get_description()));
#else
#define ERR_THREAD_GUARD
#define ERR_THREAD_GUARD_V(m_ret)
#define ERR_MAIN_THREAD_GUARD
#define ERR_MAIN_THREAD_GUARD_V(m_ret)
#define ERR_READ_THREAD_GUARD
#define ERR_READ_THREAD_GUARD_V(m_ret)
#endif


#endif