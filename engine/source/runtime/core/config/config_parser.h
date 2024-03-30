#pragma once
#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include "project_settings.h"
namespace lain {

    class ConfigParser {
    public:
        Ref<FileAccess> f;
        void ParseFile();
        HashMap<String, Variant> m_hashmap;
        ConfigParser(Ref<FileAccess> p_f) {
            f = p_f;
        }
    private:


        bool IsField(const String& line) {
            return line.size() > 2 && line.front() == '[' && line.back() == ']';
        }

        String GetField(const String& line) {
            return line.substr(1, line.size() - 2);
        }
        // 不允许一个字符串占多行
        // 不允许名中带有引号
        bool IsKeyValue(const String& line) {
            int equalpos = line.find_char('=');
            if (equalpos == -1) return false;
            int firstquote = line.find_char('"');
            if (firstquote < equalpos) return false;
            return true;

        }
        template< typename T>
        Variant load(Json json) {
            T v;
            Serializer::read(json, v);
            return Variant(v);
        }
        template<typename T>
        Variant load(T& v, Json json) {
            std::string error;
            Serializer::read(json, v);
            return Variant(v);
        }
        Reflection::ReflectionInstance* JsonToObj(Json json, const std::string& p_stdstring, std::string& error) {
            Reflection::ReflectionInstance meta_instance = Reflection::TypeMeta::newFromNameAndJson(json["$typeName"].string_value(), Json::parse(p_stdstring, error));
            if (!error.empty()) {
                return nullptr;
            }
            Reflection::ReflectionInstance* instance_ptr = nullptr;
            memnew_placement(instance_ptr, Reflection::ReflectionInstance(meta_instance));
            return instance_ptr;
        }
        // 基本类包括：Vector<Variant>，即[]；double ； String
        Variant ConstructFromString(const String& p_str, int recursize_depth = 0, bool error_print = true);

        std::string trim(const std::string& str) {
            size_t begin = str.find_first_not_of(" ");
            if (begin == std::string::npos) {
                return "";
            }

            size_t end = str.find_last_not_of(" ");
            if (end == std::string::npos) {
                return "";
            }

            return str.substr(begin, end - begin + 1);
        }

        bool IsNumericExpression(const std::string& expression);
    };
}
#endif // !CONFIG_PARSER_H
