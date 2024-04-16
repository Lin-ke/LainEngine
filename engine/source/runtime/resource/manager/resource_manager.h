#pragma once
#ifndef __RESOURCE_H__
#define __RESOURCE_H__


#include "base.h"
#include "runtime/core/meta/serializer/serializer.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>

#include "_generated/serializer/all_serializer.h"

namespace lain
{
    class ResourceManager
    {
    public:
        template<typename ResourceType>
        bool loadResource(const std::string& Resource_url, ResourceType& out_Resource) const
        {
            // read json file to string
            std::filesystem::path Resource_path = getFullPath(Resource_url);
            std::ifstream Resource_json_file(Resource_path);
            ERR_FAIL_COND_V_MSG(!Resource_json_file, false, "open file failed!");

            std::stringstream buffer;
            buffer << Resource_json_file.rdbuf();
            std::string Resource_json_text(buffer.str());

            // parse to json object and read to runtime res object
            std::string error;
            auto&& Resource_json = Json::parse(Resource_json_text, error);
            ERR_FAIL_COND_V_MSG(!error.empty(), false, "parse json file failed");

            Serializer::read(Resource_json, out_Resource);
            return truez
        }

        template<typename ResourceType>
        bool saveResource(const ResourceType& out_Resource, const std::string& Resource_url) const
        {
            std::ofstream Resource_json_file(getFullPath(Resource_url));
            ERR_FAIL_COND_V_MSG(!Resource_json_file, false, "open file failed!");

            // write to json object and dump to string
            auto&& Resource_json = Serializer::write(out_Resource);
            std::string&& Resource_json_text = Resource_json.dump();

            // write to file
            Resource_json_file << Resource_json_text;
            Resource_json_file.flush();

            return true;
        }

        std::filesystem::path getFullPath(const std::string& relative_path) const;

    };
} // namespace Piccolo
#endif // !__RESOURCE_H__
