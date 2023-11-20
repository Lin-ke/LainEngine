#pragma once
#ifndef __RES_OBJECT_H__
#define __RES_OBJECT_H__
#endif // !__RES_OBJECT_H__

#include "runtime/core/meta/reflection/reflection.h"


#include <string>
#include <vector>

namespace lain
{
    class Component;

    REFLECTION_TYPE(ComponentDefinitionRes)
        CLASS(ComponentDefinitionRes, Fields)
    {
        REFLECTION_BODY(ComponentDefinitionRes);

    public:
        std::string m_type_name;
        std::string m_component;
    };

    REFLECTION_TYPE(ObjectDefinitionRes)
        CLASS(ObjectDefinitionRes, Fields)
    {
        REFLECTION_BODY(ObjectDefinitionRes);

    public:
        std::vector<Reflection::ReflectionPtr<Component>> m_components;
    };

    REFLECTION_TYPE(ObjectInstanceRes)
        CLASS(ObjectInstanceRes, Fields)
    {
        REFLECTION_BODY(ObjectInstanceRes);

    public:
        std::string              m_name;
        std::string              m_definition;

        std::vector<Reflection::ReflectionPtr<Component>> m_instanced_components;
    };
} // namespace Piccolo
