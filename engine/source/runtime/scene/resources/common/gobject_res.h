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
        Vector<Reflection::ReflectionPtr<Component>> m_components;
    };
    

    REFLECTION_TYPE(ExtRes)
        CLASS(ExtRes, Fields) {

        REFLECTION_BODY(ExtRes);
    public:
        StringName m_type = "";
        String  m_uid = "";
        String  m_def_path = "";
        String  m_id = "";
    };

    struct SubRes{
        META(Fields)
        String m_type = "";
        String m_id = "";
    };
       


    REFLECTION_TYPE(GObjectInstanceRes)
        CLASS(GObjectInstanceRes, Fields)
    {
        REFLECTION_BODY(GObjectInstanceRes);

    public:
        HashMap<String, Variant> m_variants; // type parent name owner 
        //Groups 
        Vector <Reflection::ReflectionPtr<Component>> m_instanced_components;
        HashMap<String, String> sub_res; // 
        HashMap<String, String> ext_res; //

    };
} // namespace Piccolo
