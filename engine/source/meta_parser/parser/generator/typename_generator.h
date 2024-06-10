#pragma once
#include "generator/generator.h"
namespace Generator
{
    class TypenameGenerator : public GeneratorInterface
    {
    public:
        TypenameGenerator() = delete;
        TypenameGenerator(std::string source_directory, std::function<std::string(std::string)> get_include_function);

        virtual int generate(std::string path, SchemaMoudle schema) override;

        virtual void finish() override;

        virtual ~TypenameGenerator() override;

    protected:
        virtual void prepareStatus(std::string path) override;

        virtual std::string processFileName(std::string path) override;

    private:
        Mustache::data m_class_defines{ Mustache::data::type::list };
        Mustache::data m_enum_defines{ Mustache::data::type::list };

        Mustache::data m_include_headfiles{ Mustache::data::type::list };
    };
} // namespace Generator
