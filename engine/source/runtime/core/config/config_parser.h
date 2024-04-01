#pragma once
#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H
#include "core/io/file_access.h"
#include "core/io/dir_access.h"

namespace lain {
    
    class ConfigFile: public RefCounted {
    public:
        Error ParseFile(Ref<FileAccess> f);
        HashMap<String, HashMap<String, Variant>> values;
        ConfigFile() {}
        String WriteConfigVariant(const Variant& var);
        Error Save(const String& p_path);
        Error Load(const String& p_path);
        bool has_section(const String& p_section) const {
            return values.has(p_section);
        }

        Variant get_value(const String& p_section, const String& p_key, const Variant& p_default) const {
            if (!values.has(p_section) || !values[p_section].has(p_key)) {
                ERR_FAIL_COND_V_MSG(p_default.get_type() == Variant::NIL, Variant(),
                    vformat("Couldn't find the given section \"%s\" and key \"%s\", and no default was given.", p_section, p_key));
                return p_default;
            }

            return values[p_section][p_key];
        }
        void set_value(const String& p_section, const String& p_key, const Variant& p_value) {
            if (p_value.get_type() == Variant::NIL) { // Erase key.
                if (!values.has(p_section)) {
                    return;
                }

                values[p_section].erase(p_key);
                if (values[p_section].is_empty()) {
                    values.erase(p_section);
                }
            }
            else {
                if (!values.has(p_section)) {
                    // Insert section-less keys at the beginning.
                    values.insert(p_section, HashMap<String, Variant>(), p_section.is_empty());
                }

                values[p_section][p_key] = p_value;
            }
        }
    private:
        Error _internalSave(Ref<FileAccess> file);

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
