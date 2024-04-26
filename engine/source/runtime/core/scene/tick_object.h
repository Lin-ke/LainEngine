#pragma once
#ifndef TICK_OBJECT_H
#define TICK_OBJECT_H
#include "core/object/object.h"
namespace lain {

    class GObject;
    REFLECTION_TYPE(TickObject)
    CLASS(TickObject: public Object, WhiteListFields) {
        REFLECTION_BODY(TickObject)

        LCLASS(TickObject, Object);

        friend class SceneTree;

    protected:
        enum ProcessThreadGroup {
            PROCESS_THREAD_GROUP_INHERIT,
            PROCESS_THREAD_GROUP_MAIN_THREAD,
            PROCESS_THREAD_GROUP_SUB_THREAD,
        }; // �����߳�
        enum ProcessMode : unsigned int {
            PROCESS_MODE_INHERIT, // same as parent node
            PROCESS_MODE_PAUSABLE, // process only if not paused
            PROCESS_MODE_WHEN_PAUSED, // process only if paused
            PROCESS_MODE_ALWAYS, // process always
            PROCESS_MODE_DISABLED, // never process
        };// ������
        struct ComparatorWithPriority {
            bool operator()(const TickObject* p_a, const TickObject* p_b) const { return p_b->tickdata.process_priority > p_a->tickdata.process_priority; }
        };
        // ���û�����ȼ��ͱ��ּ���˳�򣿵����и����˳�򣬲�֪���Ƿ���Ҫ����
        struct ComparatorWithPhysicsPriority {
            bool operator()(const TickObject* p_a, const TickObject* p_b) const { return p_b->tickdata.physics_process_priority > p_a->tickdata.physics_process_priority; }
        };


        struct TickData {

            // �ڵ㴦��˳��
              /// process bools
            bool physics_process : 1;
            bool process : 1;

            bool physics_process_internal : 1; // internal�������øýڵ���ڲ������ڲ���������������� _process ���ö�������
            bool process_internal : 1;
            int process_thread_group_order = 0;
            ProcessThreadGroup process_thread_group = PROCESS_THREAD_GROUP_INHERIT;
            GObject* process_thread_group_owner = nullptr;
            void* process_group = nullptr; // to avoid cyclic dependency
            // �Ƿ���
            ProcessMode process_mode : ProcessMode::PROCESS_MODE_PAUSABLE; // mode
            GObject* process_owner = nullptr;
            // ����ӿ�
            int process_priority = 0;
            int physics_process_priority = 0;

        } tickdata;
        // must
        virtual bool can_process() const { return true; };
        L_INLINE bool is_physics_processing_internal() const { return tickdata.physics_process_internal; }
        L_INLINE bool is_physics_processing() const { return tickdata.physics_process; }
        L_INLINE bool is_processing_internal() const { return tickdata.process_internal; }
        L_INLINE bool is_processing() const { return tickdata.process; }
        L_INLINE bool is_any_processing() const {
            return tickdata.process || tickdata.process_internal || tickdata.physics_process || tickdata.physics_process_internal;
        }
    private:
        bool _can_process() const;
    };
}

#endif