#include "enum.h"
#include "class.h"
ENUM_::ENUM_(const Cursor& cursor, const Namespace& current_namespace, Class* parent)
:TypeInfo(cursor, current_namespace), m_name(cursor.getDisplayName()),
m_qualified_name(Utils::getTypeNameWithoutNamespace(cursor.getType())), m_parent(parent)
{
    m_display_name = m_qualified_name;
    std::string with_father_namespace = "";
    Namespace father_namespace = m_parent->m_namespace;
    for (int i = 0; i < father_namespace.size(); ++i) {
        if (i == 0 && father_namespace[i] == "lain") continue;
        with_father_namespace = with_father_namespace + father_namespace[i] + "::";
    }
    if (m_name != "")
        with_father_namespace += m_parent->getClassName() + "::" + m_name;
    else
        with_father_namespace += m_parent->getClassName();
    m_name_with_namespace = with_father_namespace;

   /* if (m_name == "TextureFormat") {
        for (auto& child : cursor.getChildren()) {
            L_PRINT(child.getDisplayName());
        }
        L_PRINT(m_name, shouldCompile());
    }*/

	auto children = cursor.getChildren();
	for (auto& child : children) {
		switch (child.getKind()) {
		case CXCursor_EnumConstantDecl:
		{
            m_enums[child.getEnumName()] = child.getEnumValue();
		}
		break;
		default:
			break;
		}
	}
}

int ENUM_::anonymous_count = 0;


// 默认开启，除非META(DISABLE)
bool ENUM_::isAccessible(void) const { 
    return
        m_meta_data.getFlag(NativeProperty::Enable) && m_namespace[0]=="lain";
}

void ENUM_::generateData(Mustache::data& enum_define) const {

    if (m_name == "") { // 匿名枚举
        enum_define.set("enum_is_anonymous", true);
        std::string enum_useful_name = "AnonymousEnum" + std::to_string(anonymous_count++);
        enum_define.set("enum_useful_name", enum_useful_name);
    }
    else { // 非匿名枚举

        enum_define.set("enum_is_anonymous", false);
        Mustache::data enum_value_defines(Mustache::data::type::list);
        std::string enum_useful_name = m_qualified_name;
        Utils::formatQualifiedName(enum_useful_name);
        enum_define.set("enum_useful_name", enum_useful_name);
    }
    enum_define.set("enum_with_namespace", m_name_with_namespace);
    Mustache::data enum_values(Mustache::data::type::list);
    for (auto& pair : m_enums) {
        Mustache::data enum_value_kv;
        enum_value_kv.set("enum_value_name", pair.first);
        enum_value_kv.set("enum_value", std::to_string(pair.second));
        enum_values.push_back(enum_value_kv);
    }
    enum_define.set("enum_values_define", enum_values);
}
