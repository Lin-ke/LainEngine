#pragma once
#include "runtime/core/meta/reflection/reflection.h"
#include "core/scene/object/gobject_path.h"
#include "core/variant/variant.h"

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

    REFLECTION_TYPE(GObjectDefinitionRes)
        CLASS(GObjectDefinitionRes, Fields)
    {
        REFLECTION_BODY(GObjectDefinitionRes);

    public:
        Vector<Component> m_components;
    };
    

    REFLECTION_TYPE(ExtRes)
        CLASS(ExtRes, Fields) {

        REFLECTION_BODY(ExtRes);
    public:
        StringName m_type;
        String  m_uid;
        String  m_def_path;
        String  m_id;
    };
       


    REFLECTION_TYPE(GObjectInstanceRes)
        CLASS(GObjectInstanceRes, Fields)
    {
        REFLECTION_BODY(GObjectInstanceRes);

    public:
        int         m_index;
        StringName      m_name;
        StringName      m_type;
        String      m_parent;
        String      m_root_path; // nodepath to root
        String      m_owner;
        String      m_definition; // definition path
        Variant     m_instance;  // 是外部的实例化
        //Groups 
        Vector <Reflection::ReflectionPtr<Component>> m_instanced_components;
    };
} // namespace Piccolo
