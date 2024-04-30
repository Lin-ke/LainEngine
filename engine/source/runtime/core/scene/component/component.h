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
        GObject*               m_parent=nullptr;
        bool                   m_is_dirty{ false };
        bool                   m_is_scale_dirty{ false };
        int                    m_index = -1;
        bool                   m_inside_tree{ false };
        
    public:
        Component() = default;
        virtual ~Component() {}

        L_INLINE virtual bool is_inside_tree() const override { return m_inside_tree; }
        L_INLINE void set_inside_tree(bool p_inside) { m_inside_tree = p_inside; }
        L_INLINE virtual SceneTree* get_tree() const override { return m_parent->get_tree(); }
        
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
