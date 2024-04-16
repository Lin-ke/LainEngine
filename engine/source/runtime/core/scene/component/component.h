#pragma once
#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/core/object/object.h"
#include "core/object/refcounted.h"
namespace lain
{
    class GObject;
    // Component
    REFLECTION_TYPE(Component)
        CLASS(Component : public Object, WhiteListFields)
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


        struct ComparatorByIndexCompt {
            bool operator()(const Component* p_left, const Component* p_right) const {
                return p_left->m_index < p_right->m_index;
            }
        };

    };

} // namespace Piccolo
#endif // !__COMPONENT_H__
