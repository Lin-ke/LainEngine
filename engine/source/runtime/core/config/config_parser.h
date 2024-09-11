#pragma once
#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H
#include "core/io/file_access.h"
#include "core/io/dir_access.h"
#include "core/io/resource.h"

namespace lain {
    // class VariantWriter {
    // public:
    //     typedef Error(*StoreStringFunc)(void* ud, const String& p_string);
    //     typedef String(*EncodeResourceFunc)(void* ud, const Ref<Resource>& p_resource);

    //     static Error write(const Variant& p_variant, StoreStringFunc p_store_string_func, void* p_store_string_ud, EncodeResourceFunc p_encode_res_func, void* p_encode_res_ud, int p_recursion_count = 0);
    //     static Error write_to_string(const Variant& p_variant, String& r_string, EncodeResourceFunc p_encode_res_func = nullptr, void* p_encode_res_ud = nullptr);
    // };
    class ConfigFile: public RefCounted {
    public:
        Error ParseFile(Ref<FileAccess> f);
        HashMap<String, HashMap<String, Variant>> values;
        ConfigFile() {}
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
            return line.size() > 2 && line.front() == '[' && line.end() == ']';
        }

        String GetField(const String& line) {
            return line.substr(1, line.size() - 3);
        }
        // 不允许一个字符串占多行
        // 不允许名中带有引号
        bool IsKeyValue(const String& line) {
            int equalpos = line.find_char('=');
            if (equalpos == -1) return false;
            int firstquote = line.find_char('"');
            if (firstquote != -1 && firstquote < equalpos) return false;
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

        
        bool IsNumericExpression(const std::string& expression);
    };
}
#endif // !CONFIG_PARSER_H
