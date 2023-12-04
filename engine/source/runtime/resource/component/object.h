#pragma once
#ifndef __RES_OBJECT_H__
#define __RES_OBJECT_H__
#endif // !__RES_OBJECT_H__

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

    REFLECTION_TYPE(ObjectDefinitionRes)
        CLASS(ObjectDefinitionRes, Fields)
    {
        REFLECTION_BODY(ObjectDefinitionRes);

    public:
        Vector<Reflection::ReflectionPtr<Component>> m_components;
    };

    REFLECTION_TYPE(ObjectInstanceRes)
        CLASS(ObjectInstanceRes, Fields)
    {
        REFLECTION_BODY(ObjectInstanceRes);

    public:
        String              m_name;
        String              m_definition;

        Vector<Reflection::ReflectionPtr<Component>> m_instanced_components;
    };
} 
