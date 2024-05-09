#include "common/precompiled.h"

#include "class.h"
#include "enum.h"
BaseClass::BaseClass(const Cursor& cursor) : name(Utils::getTypeNameWithoutNamespace(cursor.getType())) {}

Class::Class(const Cursor& cursor, const Namespace& current_namespace, int p_class_namespace_start) :
    TypeInfo(cursor, current_namespace), m_name(cursor.getDisplayName()),
    m_qualified_name(Utils::getTypeNameWithoutNamespace(cursor.getType())),
    m_display_name(Utils::getNameWithoutFirstM(m_qualified_name)),
    m_class_namespace_start(p_class_namespace_start)
{
    Utils::replaceAll(m_name, " ", "");
    Utils::replaceAll(m_name, "lain::", ""); // basic namespace 
    switch (cursor.getKind()) {
    case CXCursor_StructDecl: {
        m_is_struct = true;
    }
                            break;
    case CXCursor_ClassDecl: {
        m_is_struct = false;
    }
                           break;
    default:
        m_is_struct = false;
    }
    for (auto& child : cursor.getChildren())
    {
        switch (child.getKind())
        {
            case CXCursor_CXXBaseSpecifier: {
                auto base_class = new BaseClass(child);

                m_base_classes.emplace_back(base_class);
            }
            break;
            // field
            case CXCursor_FieldDecl:
                m_fields.emplace_back(new Field(child, current_namespace, this));
                break;
            // method
            case CXCursor_CXXMethod:
                m_methods.emplace_back(new Method(child, current_namespace, this));
                break;
            case CXCursor_EnumDecl: {
                std::vector<std::string> namespace_with_class = current_namespace;
                namespace_with_class.push_back(m_name);
                m_enums.emplace_back(new ENUM_(child, namespace_with_class, this));
            }
                break;
            case CXCursor_ClassDecl:
            case CXCursor_StructDecl:

            {
                std::vector<std::string> namespace_with_class = current_namespace;
                namespace_with_class.push_back(m_name);
                // 加入schema
                auto class_ptr = std::make_shared<Class>(child, 
                    namespace_with_class, static_cast<int>(current_namespace.size()));
                MetaParser::getSingleton()->addClassIntoSchema(class_ptr);
            } break;
            default:
                break;
        }
    }
}


bool Class::shouldCompile(void) const { return shouldCompileFields()|| shouldCompileMethods(); }

bool Class::shouldCompileFields(void) const
{
    return m_meta_data.getFlag(NativeProperty::All) || m_meta_data.getFlag(NativeProperty::Fields) ||
           m_meta_data.getFlag(NativeProperty::WhiteListFields);
}

bool Class::shouldCompileMethods(void) const{
    
    return m_meta_data.getFlag(NativeProperty::All) || m_meta_data.getFlag(NativeProperty::Methods) ||
           m_meta_data.getFlag(NativeProperty::WhiteListMethods);
}
// 好像没啥关系
bool Class::shouldCompileSubclass(void) const {

    return m_meta_data.getFlag(NativeProperty::All) || m_meta_data.getFlag(NativeProperty::Subclass) ||
        m_meta_data.getFlag(NativeProperty::WhiteListSubclass);
}
//bool Class::shouldCompileInner(void) const {
//
//}

std::string Class::getClassName(void) { return m_name; }

bool Class::isAccessible(void) const { return m_enabled; }