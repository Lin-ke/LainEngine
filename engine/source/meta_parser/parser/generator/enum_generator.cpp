#include "enum_generator.h"
#include "common/precompiled.h"
#include "template_manager/template_manager.h"
#include "language_types/enum.h"
namespace Generator {
    EnumGenerator::EnumGenerator(std::string                             source_directory,
        std::function<std::string(std::string)> get_include_function) :
        GeneratorInterface(source_directory + "/_generated/enums", source_directory, get_include_function)
    {
        prepareStatus(m_out_path);
    }

    void EnumGenerator::prepareStatus(std::string path)
    {
        GeneratorInterface::prepareStatus(path);
        TemplateManager::getInstance()->loadTemplates(m_root_path, "commonEnumFile");
        TemplateManager::getInstance()->loadTemplates(m_root_path, "allEnumFile");
        return;
    }
    std::string EnumGenerator::processFileName(std::string path)
    {
        auto relativeDir = fs::path(path).filename().replace_extension("enum.gen.h").string();
        return m_out_path + "/" + relativeDir;
    }
    int EnumGenerator::generate(std::string path, SchemaMoudle schema) {
        std::string    file_path = processFileName(path);
        Mustache::data mustache_data;
        Mustache::data include_headfiles(Mustache::data::type::list);
        Mustache::data enum_defines(Mustache::data::type::list);
        include_headfiles.push_back(
            Mustache::data("headfile_name", Utils::makeRelativePath(m_root_path, path).string()));
        bool need_generate = false;
        if (!schema.enums.empty()) {
            if (nullptr == mustache_data.get("enum_exists"))
                mustache_data.set("enum_exists", true);
        }
        for (auto enum_ : schema.enums) {

            if (!enum_->shouldCompile()) {
                continue;
            }
            need_generate = true;

            Mustache::data enum_define;
            enum_->generateData(enum_define);
            enum_defines.push_back(enum_define);
        }
        if (need_generate) {
            mustache_data.set("enum_defines", enum_defines);
            mustache_data.set("include_headfiles", include_headfiles);
            std::string tmp = Utils::convertNameToUpperCamelCase(fs::path(path).stem().string(), "_");
            mustache_data.set("sourefile_name_upper_camel_case", tmp);
            std::string render_string =
                TemplateManager::getInstance()->renderByTemplate("commonEnumFile", mustache_data);
            Utils::saveFile(render_string, file_path);
            m_sourcefile_list.emplace_back(tmp);

            m_head_file_list.emplace_back(Utils::makeRelativePath(m_root_path, file_path).string());
        }

        return 0;
    }
    void EnumGenerator::finish()
    {
        Mustache::data mustache_data;
        Mustache::data include_headfiles = Mustache::data::type::list;
        Mustache::data sourefile_names = Mustache::data::type::list;

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
            TemplateManager::getInstance()->renderByTemplate("allEnumFile", mustache_data);
        Utils::saveFile(render_string, m_out_path + "/all_enum.h");
    }

    EnumGenerator::~EnumGenerator() {}

}