#pragma once
#ifndef __GOBJECT_H__
#define __GOBJECT_H__
#include "core/object/object.h"
#include "runtime/core/meta/reflection/reflection.h"
#include "gobject_path.h"
namespace lain {
    class Component;
    REFLECTION_TYPE(GObjectDefinitionRes)
        CLASS(GObjectDefinitionRes, Fields)
    {
        REFLECTION_BODY(GObjectDefinitionRes);

    public:
        Vector<Reflection::ReflectionPtr<Component>> m_components;
    };
    REFLECTION_TYPE(GObject)
        CLASS(GObject: public Object, WhiteListFields) {
        REFLECTION_BODY(GObject);

        LCLASS(GObject, Object);
    public:
        enum InternalMode {
            INTERNAL_MODE_DISABLED,
            INTERNAL_MODE_FRONT,
            INTERNAL_MODE_BACK,
        }; // 在siblings强制靠前/靠后
        InternalMode internal_mode = INTERNAL_MODE_DISABLED;
        mutable int index = -1; // relative to front, normal or back.
        int depth = -1;
        // {
        StringName name;
        String m_scene_file_path;
        GObject* m_parent;
        HashMap<StringName, GObject*> children;
        Vector<Reflection::ReflectionPtr<Component>> m_components; // Reflection::ReflectionPtr<Component>


        /// process bools
        bool physics_process : 1;
        bool process : 1;

        bool physics_process_internal : 1;
        bool process_internal : 1;

        bool input : 1;
        bool shortcut_input : 1;
        bool unhandled_input : 1;
        bool unhandled_key_input : 1;
        mutable GObjectPath* path_cache = nullptr;
        // }  data

        Vector<Reflection::ReflectionPtr<Component>> GetComponents() { return m_components; }
        template<typename TComponent>
        TComponent* TryGetComponent(const std::string& compenent_type_name)
        {
            for (auto& component : m_components)
            {
                if (component.getTypeName() == compenent_type_name)
                {
                    return static_cast<TComponent*>(component.operator->());
                }
            }

            return nullptr;
        }
        GObject* get_parent() const;
        GObjectPath get_path() const;


    private:
        struct ComparatorByIndex {
            bool operator()(const GObject* p_left, const GObject* p_right) const {
                static const uint32_t order[3] = { 1, 0, 2 };
                uint32_t order_left = order[p_left->internal_mode];
                uint32_t order_right = order[p_right->internal_mode];
                if (order_left == order_right) {
                    return p_left->index < p_right->index;
                }
                return order_left < order_right;
            }
        };

	};
}

#endif