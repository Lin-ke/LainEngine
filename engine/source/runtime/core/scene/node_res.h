#pragma once
#ifndef __RES_Node_H__
#define __RES_Node_H__
#endif // !__RES_Node_H__

#include "runtime/core/meta/reflection/reflection.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"

namespace lain
{
    class Component;

    REFLECTION_TYPE(ComponentDefinitionRes)
        CLASS(ComponentDefinitionRes, Fields)
    {
        REFLECTION_BODY(ComponentDefinitionRes);

    public:
        String m_type_name;
        String m_component;
    };

    REFLECTION_TYPE(NodeDefinitionRes)
        CLASS(NodeDefinitionRes, Fields)
    {
        REFLECTION_BODY(NodeDefinitionRes);

    public:
        Vector<Reflection::ReflectionPtr<Component>> m_components;
    };

    REFLECTION_TYPE(NodeInstanceRes)
        CLASS(NodeInstanceRes, Fields)
    {
        REFLECTION_BODY(NodeInstanceRes);

    public:
        String              m_name;
        String              m_definition;

        Vector<Reflection::ReflectionPtr<Component>> m_instanced_components;
    };
} 
