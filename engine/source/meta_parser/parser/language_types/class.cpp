#include "common/precompiled.h"

#include "class.h"
#include "enum.h"
BaseClass::BaseClass(const Cursor& cursor) : name(Utils::getTypeNameWithoutNamespace(cursor.getType())) {}

void modify_meta_info(MetaInfo& lhs, const MetaInfo& rhs) {
    if (rhs.getFlag(NativeProperty::All)) {
        lhs.setFlag(NativeProperty::All);
        return;
    }
    if (rhs.getFlag(NativeProperty::Fields)) {
        lhs.setFlag(NativeProperty::Fields);
    }
    if (rhs.getFlag(NativeProperty::Methods)) {
        lhs.setFlag(NativeProperty::Methods);
    }
    if (rhs.getFlag(NativeProperty::WhiteListFields)) {
        lhs.setFlag(NativeProperty::WhiteListFields);
    }
    if (rhs.getFlag(NativeProperty::WhiteListMethods)) {
        lhs.setFlag(NativeProperty::WhiteListMethods);
    }
    return;
}

Class::Class(const Cursor& cursor, const Namespace& current_namespace, int p_class_namespace_start) :
    TypeInfo(cursor, current_namespace), m_name(cursor.getDisplayName()),
    m_qualified_name(Utils::getTypeNameWithoutNamespace(cursor.getType())),
    m_display_name(Utils::getNameWithoutFirstM(m_qualified_name)),
    m_class_namespace_start(p_class_namespace_start)
{
    bool perhaps_need_hack = !shouldCompile();

    Utils::replaceAll(m_name, " ", "");
    Utils::replaceAll(m_name, "lain::", ""); // basic namespace 

    std::string with_father_namespace = "";
    for (int i = 0; i < current_namespace.size(); ++i) {
        if (i == 0 && current_namespace[i] == "lain") continue;
        with_father_namespace = with_father_namespace + current_namespace[i] + "::";
    }
    with_father_namespace += m_name;
    m_name_with_namespace = with_father_namespace;

    
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
                auto base_class = new BaseClass(child); // @TODO add namespace

                m_base_classes.emplace_back(base_class);
            }
            break;
            // field
            case CXCursor_FieldDecl:
            {
                m_fields.emplace_back(new Field(child, current_namespace, this));
                if (perhaps_need_hack) {
                    MetaInfo meta(child);
                    modify_meta_info(m_meta_data, meta);
                    perhaps_need_hack = !shouldCompile();
                }
                // annotation

            }
                break;
            // method
            case CXCursor_CXXMethod:
            {
                m_methods.emplace_back(new Method(child, current_namespace, this));
                if (perhaps_need_hack) {
                    MetaInfo meta(child);
                    modify_meta_info(m_meta_data, meta);
                    perhaps_need_hack = !shouldCompile();
                }
            }
                break;
            case CXCursor_EnumDecl: {
                // 没有命名空间的问题

                std::shared_ptr<ENUM_>p_enum = std::make_shared<ENUM_>(child, current_namespace, this);
                m_enums.emplace_back(p_enum); // 遇到以enum定义的类可能有用
                MetaParser::getSingleton()->addEnumIntoSchema(p_enum);
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
            
            default: { // hack
               
            }break;
            
        }
       
    }
    if (m_name == "AABB") {
        L_PRINT("AABB find", shouldCompile());
    }

    
    // 加入一点Hack
    // metadata改在child methods与 child field中找
    // 这样可以不在定义上写CLASS 导致 代码提示编译器出奇怪问题
   /* if (m_name == "Object") {
        for ( auto& key : m_methods) {
            L_PRINT(key.get()->m_name);
            for (auto& k : key.get()->m_meta_data.m_properties) {
                L_PRINT(k.first, k.second);
            }
        }
        for (auto& key : m_fields) {
            L_PRINT(key.get()->m_name);
            for (auto& k : key.get()->m_meta_data.m_properties) {
                L_PRINT(k.first, k.second);
            }
        }
    }
    L_PRINT(m_name, shouldCompile());*/
}

bool Class::shouldCompile(void) const { return shouldCompileFields() || shouldCompileMethods(); }

bool Class::shouldCompileFields(void) const
{
    return m_meta_data.getFlag(NativeProperty::All) || m_meta_data.getFlag(NativeProperty::Fields) ||
           m_meta_data.getFlag(NativeProperty::WhiteListFields);
    
}

bool Class::shouldCompileMethods(void) const{
    
    return m_meta_data.getFlag(NativeProperty::All) || m_meta_data.getFlag(NativeProperty::Methods) ||
           m_meta_data.getFlag(NativeProperty::WhiteListMethods);
}

std::string Class::getClassName(void) { return m_name; }
std::string Class::getNamespace(void) { return m_name; }


bool Class::isAccessible(void) const { return m_enabled; }