#pragma once
#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/core/object/object.h"
#include "core/scene/object/gobject.h"
#include "core/object/refcounted.h"
namespace lain
{
    class GObject;
    // Component
    REFLECTION_TYPE(Component)
        CLASS(Component : public TickObject, WhiteListFields)
    {
        REFLECTION_BODY(Component)
        LCLASS(Component, Object);
        friend class GObject;

    protected:
        GObject*               m_parent;
        bool                   m_is_dirty{ false };
        bool                   m_is_scale_dirty{ false };
        int                    m_index = -1;
        
    public:
        Component() = default;
        virtual ~Component() {}

        L_INLINE bool is_physics_processing_internal() const { return tickdata.physics_process_internal; }
        L_INLINE bool is_physics_processing() const { return tickdata.physics_process; }
        L_INLINE bool is_processing_internal() const { return tickdata.process_internal; }
        L_INLINE bool is_processing() const { return tickdata.process; }
        L_INLINE bool is_any_processing() const {
            return tickdata.process || tickdata.process_internal || tickdata.physics_process || tickdata.physics_process_internal;
        }
        L_INLINE void _remove_process_group() {
            m_parent->get_tree()->_remove_process_group(this);
        }
        L_INLINE void _add_process_group() {
            m_parent->get_tree()->_add_process_group(this);
        }

        L_INLINE void _add_to_process_thread_group() {
            m_parent->get_tree()->_add_node_to_process_group(this, tickdata.process_thread_group_owner);
        }
        L_INLINE void _remove_from_process_thread_group() {
            m_parent->get_tree()->_remove_node_from_process_group(this, tickdata.process_thread_group_owner);
        }

        
        struct ComparatorByIndexCompt {
            bool operator()(const Component* p_left, const Component* p_right) const {
                return p_left->m_index < p_right->m_index;
            }
        };
    private:
        void _notification(int p_what);
    };

} // namespace Piccolo
#endif // !__COMPONENT_H__
