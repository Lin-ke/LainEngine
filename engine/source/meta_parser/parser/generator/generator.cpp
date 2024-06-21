#include "common/precompiled.h"

#include "generator/generator.h"
#include "language_types/class.h"

namespace Generator
{
    void GeneratorInterface::prepareStatus(std::string path)
    {
        if (!fs::exists(path))
        {
            fs::create_directories(path);
        }
    }
    void GeneratorInterface::genClassRenderData(std::shared_ptr<Class> class_temp, Mustache::data& class_def)
    {
        class_def.set("class_name", class_temp->getClassName());
        class_def.set("class_base_class_size", std::to_string(class_temp->m_base_classes.size()));
        class_def.set("class_need_register", true);
        class_def.set("class_is_struct", class_temp->m_is_struct);
        auto current_namespace = class_temp->getCurrentNamespace();
        if (class_temp->m_class_namespace_start != -1) {
            class_def.set("class_is_in_class", true);
            class_def.set("class_father_namespace", current_namespace[current_namespace.size() - 1]);
        }
        else {
            class_def.set("class_is_in_class", false);
            class_def.set("class_father_namespace", "");
        }
        std::string with_father_namespace = "";
        for (int i = 0; i < current_namespace.size(); ++i) {
            if (i == 0 && current_namespace[i] == "lain") continue;
            with_father_namespace = with_father_namespace + current_namespace[i] + "::";
        }
        with_father_namespace += class_temp->getClassName();
        class_def.set("class_with_namespace", with_father_namespace);

        if (class_temp->m_base_classes.size() > 0)
        {
            Mustache::data class_base_class_defines(Mustache::data::type::list);
            class_def.set("class_has_base", true);
            for (int index = 0; index < class_temp->m_base_classes.size(); ++index)
            {
                Mustache::data class_base_class_def;
                class_base_class_def.set("class_base_class_name", class_temp->m_base_classes[index]->name);
                class_base_class_def.set("class_base_class_index", std::to_string(index));
                class_base_class_defines.push_back(class_base_class_def);
            }
            class_def.set("class_base_class_defines", class_base_class_defines);
        }

        Mustache::data class_field_defines = Mustache::data::type::list;
        genClassFieldRenderData(class_temp, class_field_defines);
        class_def.set("class_field_defines", class_field_defines);

        
        Mustache::data class_method_defines = Mustache::data::type::list;
        genClassMethodRenderData(class_temp, class_method_defines);
        class_def.set("class_method_defines", class_method_defines);
    }
    void GeneratorInterface::genClassFieldRenderData(std::shared_ptr<Class> class_temp, Mustache::data& feild_defs)
    {

        for (auto& field : class_temp->m_fields)
        {
            if (!field->shouldCompile())
                continue;
            Mustache::data field_define;

            field_define.set("class_field_name", field->m_name);
            field_define.set("class_field_type", field->m_type);
            field_define.set("class_field_display_name", field->m_display_name);
            int is_vector = Utils::is_name_vector(field->m_type);
            field_define.set("class_field_is_vector", is_vector != 0);
            bool is_cow_vector = is_vector == 2;;
            field_define.set("class_field_is_cow_vector", is_cow_vector);
            bool is_fixed = is_vector == -1;
            field_define.set("class_field_is_fixed_array", is_fixed);
            if (is_fixed) {
                std::string array_type = Utils::getNameWithoutBracket(field->m_type);
                field_define.set("class_field_array_base_type", array_type);
                std::string array_useful_name = field->m_type;
                field_define.set("class_field_array_useful_name", Utils::formatQualifiedName(field->m_name_with_namespace));
                field_define.set("class_field_fixed_array_size", Utils::getFixArraySize(field->m_type));
            }
            else {
                field_define.set("class_field_array_base_type", ""); // is it necessary?
                field_define.set("class_field_array_useful_name", "");
                field_define.set("class_field_fixed_array_size", "");
            }

            feild_defs.push_back(field_define);
        }
    }

    void GeneratorInterface::genClassMethodRenderData(std::shared_ptr<Class> class_temp, Mustache::data& method_defs)
    {
       for (auto& method : class_temp->m_methods)
        {
            if (!method->shouldCompile())
                continue;
            Mustache::data method_define;

            method_define.set("class_method_name", method->m_name);   
            method_defs.push_back(method_define);
        }
    }
} // namespace Generator
