#include "generator/typename_generator.h"
#include "common/precompiled.h"
#include "language_types/class.h"

namespace Generator
{
    TypenameGenerator::TypenameGenerator(std::string                             source_directory,
        std::function<std::string(std::string)> get_include_function) :
        GeneratorInterface(source_directory + "/_generated/typename", source_directory, get_include_function)
    {
        prepareStatus(m_out_path);
    }

    void TypenameGenerator::prepareStatus(std::string path)
    {
        GeneratorInterface::prepareStatus(path);
        TemplateManager::getInstance()->loadTemplates(m_root_path, "allTypename.h");
        TemplateManager::getInstance()->loadTemplates(m_root_path, "allTypename.ipp");
        TemplateManager::getInstance()->loadTemplates(m_root_path, "commonTypenameGenFile");
        return;
    }

    std::string TypenameGenerator::processFileName(std::string path)
    {
        auto relativeDir = fs::path(path).filename().replace_extension("typename.gen.h").string();
        return m_out_path + "/" + relativeDir;
    }
    int TypenameGenerator::generate(std::string path, SchemaMoudle schema)
    {
        std::string file_path = processFileName(path);

        Mustache::data muatache_data;
        Mustache::data include_headfiles(Mustache::data::type::list);
        Mustache::data class_defines(Mustache::data::type::list);
        bool need_generate = false;
        include_headfiles.push_back(
            Mustache::data("headfile_name", Utils::makeRelativePath(m_root_path, path).string()));
        for (auto class_temp : schema.classes)
        {
            if (class_temp->getCurrentNamespace()[0] != "lain")
                continue;
            need_generate = true;
            Mustache::data class_def;
            auto& current_namespace = class_temp->getCurrentNamespace();
            std::string with_father_namespace = "";
            for (int i = 0; i < current_namespace.size(); ++i) {
                if (i == 0 && current_namespace[i] == "lain") continue;
                with_father_namespace = with_father_namespace + current_namespace[i] + "::";
            }
            with_father_namespace += class_temp->getClassName();
            class_def.set("class_with_namespace", with_father_namespace);
            class_def.set("class_better_name", Utils::formatQualifiedName(with_father_namespace));
            class_defines.push_back(class_def);
            m_class_defines.push_back(class_def);
        }
        for (auto enum_temp : schema.enums) {
            if (!enum_temp->shouldCompile() || enum_temp->m_name == "") continue;
            need_generate = true;
            Mustache::data enum_def;
            enum_def.set("class_with_namespace", enum_temp->m_name_with_namespace);
            enum_def.set("class_better_name", Utils::formatQualifiedName(enum_temp->m_name_with_namespace));
            class_defines.push_back(enum_def);
            m_class_defines.push_back(enum_def);
        }
        if (need_generate) {
            muatache_data.set("class_defines", class_defines);
            muatache_data.set("include_headfiles", include_headfiles);
            std::string render_string =
                TemplateManager::getInstance()->renderByTemplate("commonTypenameGenFile", muatache_data);
            Utils::saveFile(render_string, file_path);

            m_include_headfiles.push_back(
                Mustache::data("headfile_name", Utils::makeRelativePath(m_root_path, file_path).string()));
        }
        
        return 0;
    }

    void TypenameGenerator::finish()
    {
        Mustache::data mustache_data;
        mustache_data.set("class_defines", m_class_defines);
        mustache_data.set("include_headfiles", m_include_headfiles);

        std::string render_string = TemplateManager::getInstance()->renderByTemplate("allTypename.h", mustache_data);
        Utils::saveFile(render_string, m_out_path + "/all_typename.h");
        render_string = TemplateManager::getInstance()->renderByTemplate("allTypename.ipp", mustache_data);
        Utils::saveFile(render_string, m_out_path + "/all_typename.ipp");
    }

    TypenameGenerator::~TypenameGenerator() {}
} // namespace Generator
