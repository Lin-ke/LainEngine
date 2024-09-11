#pragma once

#include "type_info.h"

class Class;

class Field : public TypeInfo
{
    friend class Class;

public:
    Field(const Cursor& cursor, const Namespace& current_namespace, Class* parent = nullptr);

    virtual ~Field(void) {}

    bool shouldCompile(void) const;

public:
    bool m_is_const;

    Class* m_parent;

    std::string m_name;
    std::string m_display_name;
    std::string m_type;

    std::string m_default;
    std::string m_name_with_namespace; // 用这个能找到他

    bool isAccessible(void) const;
};