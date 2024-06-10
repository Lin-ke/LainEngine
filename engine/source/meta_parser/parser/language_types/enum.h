#pragma 
#ifndef __ENUM_H__
#define __ENUM_H__
#include "common/precompiled.h"
#include "type_info.h"
class Class;
class ENUM_ : public TypeInfo {
public:
	ENUM_(const Cursor& cursor, const Namespace& current_namespace, Class* parent = nullptr);
    bool shouldCompile(void) const { return isAccessible(); }
    Class* m_parent;
    std::map<std::string, int> m_enums;
    std::string m_name; // WrapMode
    std::string m_display_name;
    std::string m_qualified_name; // lain::Image::CompressMode
    std::string m_default;
    std::string m_name_with_namespace; // Class::Image::CompressMode  (without lain)
    static int anonymous_count;

    bool isAccessible(void) const;
    void generateData(Mustache::data&) const;
};
#endif // !__ENUM_H__
