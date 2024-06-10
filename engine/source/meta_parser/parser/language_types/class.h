#pragma once

#include "type_info.h"

#include "field.h"
#include "method.h"
#include "enum.h"

struct BaseClass
{
    BaseClass(const Cursor& cursor);

    std::string name;
    // @TODO add namespace 
};

class Class : public TypeInfo
{
    // to access m_qualifiedName
    friend class Field;
    friend class ENUM_;
    friend class Method;
    friend class MetaParser;

public:
    Class(const Cursor& cursor, const Namespace& current_namespace, int p_class_namespace_start=-1);
    virtual bool shouldCompile(void) const;

    bool shouldCompileFields(void) const;
    bool shouldCompileMethods(void) const;
    bool shouldCompileEnum(void) const;

    template<typename T>
    using SharedPtrVector = std::vector<std::shared_ptr<T>>;

    std::string getClassName(void);
    std::string getNamespace(void);


    SharedPtrVector<BaseClass> m_base_classes;

public:
    std::string m_name;

    std::string m_qualified_name;

    std::string m_name_with_namespace;
    
    bool should_compile_fields;
    bool should_compile_methods;

    
    int m_class_namespace_start = -1; // namespace[m_class_namespace_start] = class_namespace

    SharedPtrVector<Field> m_fields;
    SharedPtrVector<Method> m_methods;
    SharedPtrVector<ENUM_> m_enums;
    SharedPtrVector<Class> m_subclass;


    std::string m_display_name;
    bool   m_is_struct;
    bool isAccessible(void) const;
};
