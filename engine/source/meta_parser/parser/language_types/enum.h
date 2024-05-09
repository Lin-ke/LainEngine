#pragma 
#ifndef __ENUM_H__
#define __ENUM_H__
#include "common/precompiled.h"
#include "type_info.h"
class Class;
class ENUM_ : public TypeInfo {
public:
	ENUM_(const Cursor& cursor, const Namespace& current_namespace, Class* parent = nullptr);
	bool shouldCompile(void) const;
    Class* m_parent;
    std::map<int, std::string> m_enums;
    std::string m_name;
    std::string m_qualified_name;
    std::string m_default;

    bool isAccessible(void) const;
};
#endif // !__ENUM_H__
