#include "enum.h"
#include "class.h"
ENUM_::ENUM_(const Cursor& cursor, const Namespace& current_namespace, Class* parent)
:TypeInfo(cursor, current_namespace), m_name(cursor.getDisplayName()),
m_qualified_name(Utils::getTypeNameWithoutNamespace(cursor.getType())), m_parent(parent)
{
	auto& children = cursor.getChildren();
	for (auto& child : children) {
		switch (cursor.getKind()) {
		case CXCursor_EnumConstantDecl:
		{
			m_enums[cursor.getEnumValue()] = cursor.getEnumName();
		}
		break;
		default:
			break;
		}
	}
}
bool ENUM_::isAccessible(void) const { return m_parent->isAccessible(); }
