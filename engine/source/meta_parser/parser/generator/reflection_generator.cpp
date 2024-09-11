#include "common/precompiled.h"
#include "generator/reflection_generator.h"
#include "language_types/class.h"
#include "template_manager/template_manager.h"

#include <map>
#include <set>
#include <iostream>
#include <initializer_list>
#include <sstream>
#include <regex>

namespace Generator
{

    ReflectionGenerator::ReflectionGenerator(std::string                             source_directory,
                                             std::function<std::string(std::string)> get_include_function) :
        GeneratorInterface(source_directory + "/_generated/reflection", source_directory, get_include_function)
    {
        prepareStatus(m_out_path);
    }
    void ReflectionGenerator::prepareStatus(std::string path)
    {
        GeneratorInterface::prepareStatus(path);
        TemplateManager::getInstance()->loadTemplates(m_root_path, "commonReflectionFile");
        TemplateManager::getInstance()->loadTemplates(m_root_path, "allReflectionFile");
        return;
    }

    std::string ReflectionGenerator::processFileName(std::string path)
    {
        auto relativeDir = fs::path(path).filename().replace_extension("reflection.gen.h").string();
        return m_out_path + "/" + relativeDir;
    }

    int ReflectionGenerator::generate(std::string path, SchemaMoudle schema)
    {
        
        std::string    file_path = processFileName(path);

        Mustache::data mustache_data;
        Mustache::data include_headfiles(Mustache::data::type::list);
        Mustache::data class_defines(Mustache::data::type::list);

        include_headfiles.push_back(
            Mustache::data("headfile_name", Utils::makeRelativePath(m_root_path, path).string()));

        std::map<std::string, bool> class_names;
        bool need_generate = false;
        // class defs
        for (auto class_temp : schema.classes)
        {// 这里是该文件中所有的需要compile的类
            
            if (!class_temp->shouldCompile())
                continue;
            need_generate = true;
            class_names.insert_or_assign(class_temp->getClassName(), false);
            class_names[class_temp->getClassName()] = true;

            std::vector<std::string>                                   field_names;
            std::map<std::string, std::tuple<std::string,std::string, std::string>> vector_map; // vector_type_name, array_useful_name, element_type
            std::map<std::string, std::string> fixed_array_map;

            Mustache::data class_def;
            Mustache::data vector_defines(Mustache::data::type::list);


            genClassRenderData(class_temp, class_def);
            for (auto field : class_temp->m_fields)
            {
            /*    L_PRINT("field_name", field->m_name);
                L_PRINT("field_parent", field->m_parent);
                L_PRINT("field_default", field->m_default);
                L_PRINT("field_type", field->m_type);*/


                if (!field->shouldCompile())
                    continue;
                field_names.emplace_back(field->m_name);
                
                int array_type = Utils::is_name_vector(field->m_type);
                if (array_type)
                {
                    std::string array_useful_name = field->m_name_with_namespace;

                    Utils::formatQualifiedName(array_useful_name);
                    
                    std::string item_type = field->m_type;
                    if(array_type == -1) // fixed
                    {
                        item_type = Utils::getNameWithoutBracket(item_type);
                        fixed_array_map[field->m_type] = Utils::getFixArraySize(field->m_type);
                    }
                    else
                        item_type = Utils::getNameWithoutContainer(item_type);

                    vector_map[field->m_type] = std::tuple(array_useful_name, item_type, field->m_type);

                    //L_PRINT("ADD PAIR", array_useful_name, item_type);
                }
            }
            
            if (vector_map.size() > 0)
            {
                if (nullptr == class_def.get("vector_exist"))
                {
                    class_def.set("vector_exist", true);
                }
                
                
                for (auto vector_item : vector_map)
                {
                    std::string    array_useful_name = std::get<0>(vector_item.second);
                    std::string    item_type         = std::get<1>(vector_item.second);
                    std::string    array_type = std::get<2>(vector_item.second);
                    Mustache::data vector_define;
                    vector_define.set("vector_useful_name", array_useful_name);
                    vector_define.set("vector_type_name", vector_item.first);
                    vector_define.set("vector_element_type_name", item_type);
                    vector_define.set("vector_is_cow_vector", (array_type.find("Vector") == 0));
                    bool is_fixed_array = fixed_array_map.find(vector_item.first) != fixed_array_map.end();
                    vector_define.set("vector_is_fixed_array", is_fixed_array);
                    if(is_fixed_array)
                        vector_define.set("vector_fixed_size", fixed_array_map[vector_item.first]);
                    else
                        vector_define.set("vector_fixed_size", "0");

                    vector_defines.push_back(vector_define);
                }
            }
            class_def.set("vector_defines", vector_defines);
            if (!class_temp->m_enums.empty()) {
                if (nullptr == class_def.get("enum_exists"))
                    class_def.set("enum_exists", true);
            }
           
            
            class_defines.push_back(class_def);
        }
        if (need_generate) {

            mustache_data.set("class_defines", class_defines);
            mustache_data.set("include_headfiles", include_headfiles);

            std::string tmp = Utils::convertNameToUpperCamelCase(fs::path(path).stem().string(), "_");
            mustache_data.set("sourefile_name_upper_camel_case", tmp);

            std::string render_string =
                TemplateManager::getInstance()->renderByTemplate("commonReflectionFile", mustache_data);
            Utils::saveFile(render_string, file_path);

            m_sourcefile_list.emplace_back(tmp);

            m_head_file_list.emplace_back(Utils::makeRelativePath(m_root_path, file_path).string());
        }

        return 0;
    }

    void ReflectionGenerator::finish()
    {
        Mustache::data mustache_data;
        Mustache::data include_headfiles = Mustache::data::type::list;
        Mustache::data sourefile_names    = Mustache::data::type::list;

        for (auto& head_file : m_head_file_list)
        {
            include_headfiles.push_back(Mustache::data("headfile_name", head_file));
        }
        for (auto& sourefile_name_upper_camel_case : m_sourcefile_list)
        {
            sourefile_names.push_back(Mustache::data("sourefile_name_upper_camel_case", sourefile_name_upper_camel_case));
        }
        mustache_data.set("include_headfiles", include_headfiles);
        mustache_data.set("sourefile_names", sourefile_names);
        std::string render_string =
            TemplateManager::getInstance()->renderByTemplate("allReflectionFile", mustache_data);
        Utils::saveFile(render_string, m_out_path + "/all_reflection.h");
    }

    ReflectionGenerator::~ReflectionGenerator() {}
} // namespace Generator