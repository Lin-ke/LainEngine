#include "common/precompiled.h"

#include "class.h"
#include "field.h"

Field::Field(const Cursor& cursor, const Namespace& current_namespace, Class* parent) :
    TypeInfo(cursor, current_namespace), m_is_const(cursor.getType().IsConst()), m_parent(parent),
    m_name(cursor.getSpelling()), m_display_name(Utils::getNameWithoutFirstM(m_name)),
    m_type(Utils::getTypeNameWithoutNamespace(cursor.getType()))
{
    Utils::replaceAll(m_type, " ", "");
    Utils::replaceAll(m_type, "lain::", "");
    auto ret_string = Utils::getStringWithoutQuot(m_meta_data.getProperty("default"));
    m_default       = ret_string;

    std::string with_father_namespace = "";
    Namespace father_namespace = m_parent->m_namespace;
    for (int i = 0; i < father_namespace.size(); ++i) {
        if (i == 0 && father_namespace[i] == "lain") continue;
        with_father_namespace = with_father_namespace + father_namespace[i] + "::";
    }
    with_father_namespace = with_father_namespace + m_parent->getClassName() + "::" + m_name;

    m_name_with_namespace = with_father_namespace ;
}

bool Field::shouldCompile(void) const { return isAccessible(); }

bool Field::isAccessible(void) const
{
    return ((m_parent->m_meta_data.getFlag(NativeProperty::Fields) ||
             m_parent->m_meta_data.getFlag(NativeProperty::All)) &&
            !m_meta_data.getFlag(NativeProperty::Disable)) ||
           (m_parent->m_meta_data.getFlag(NativeProperty::WhiteListFields) &&
            m_meta_data.getFlag(NativeProperty::Enable));
}
