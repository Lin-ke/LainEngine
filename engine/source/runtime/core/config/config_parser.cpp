#include "config_parser.h"
#include "core/meta/reflection/reflection.h"
#include "core/meta/serializer/serializer.h"
#include "core/variant/variant_parser.h"
#include <regex>
namespace lain {
    Error ConfigFile::ParseFile(Ref<FileAccess> f)
    {
        String currentField;
        bool inValue;
        Error err = OK;
        while (!f->eof_reached()) {
            String line = f->get_line();
            if (line == "" || line.begins_with("//") ){
                // comment or null
                continue;
            }
            if (IsField(line)) {
                currentField = GetField(line);
                inValue = false;
                f->get_line();

            }
            else if (IsKeyValue(line)) {

                inValue = true;
                int delimiterPos = line.find("=", 0);
                String key = line.substr(0, delimiterPos); 
                key = key.trim();
                String value = line.substr(delimiterPos + 1, line.length()); 
                Variant variant_value = ConstructFromString(value);
                if (variant_value.get_type() == Variant::NIL) {
                    err = ERR_PARSE_ERROR;
                }
                if (variant_value.get_type() == Variant::Type::NIL) {
                    L_CORE_WARN("NIL config meet: " + currentField + "/" + key);
                }
                values[currentField][key] = variant_value;
                
            }
            else {
                // Parsing error
                err =  ERR_PARSE_ERROR;
            }

        }
        return err;
    }
    Variant ConfigFile::ConstructFromString(const String& p_str, int recursize_depth, bool error_print)
    {
        std::string error;
        if (unlikely(recursize_depth > 128)) {
            if(error_print)
            L_CORE_ERROR("than max recursize depth");
            return Variant();
        }
        if (p_str == "")
            return Variant(String(""));
        if (p_str.begins_with("Packed")) {

            i32 brankpos = p_str.rfind("[");
            i32 rbrankpos = p_str.rfind("]");
            if (brankpos == -1 || rbrankpos == -1 ) {
                error = "not valid PackedString";
                if (error_print)
                L_CORE_ERROR(error);
                return Variant();
            }
            auto&& json = Json::parse(p_str.substr(brankpos, rbrankpos - brankpos + 1).utf8().get_data(), error);
            if (!error.empty())
            {
                if (error_print)
                L_CORE_ERROR("parse json file {} failed!", p_str);
                return Variant();
            }
            if (p_str.begins_with("PackedString")) {
                return load<Vector<String>>(json);
            }
            else if (p_str.begins_with("PackedFloat32")) {
                return load<Vector<float>>(json);
            }
            else if (p_str.begins_with("PackedFloat64")) {
                return load<Vector<double>>(json);
            }
            else if (p_str.begins_with("PackedInt")) {
                return load<Vector<int32_t>>(json);
            }
        }

        else if (p_str.begins_with("\"")) {
            String p_str_ = p_str.trim();
            if (p_str_.length() < 2 && p_str_.rfind("\"") != p_str_.length() - 1) {
                if(error_print)
                    L_CORE_ERROR("not valid String");
                return Variant();
            }
            return Variant(p_str_.substr(1, p_str_.length() - 2));
        }
        else { // Other class?
            std::string p_stdstring = p_str.trim().utf8().get_data();
            if (p_str.begins_with("{")) {
                auto&& json = Json::parse(p_stdstring, error);
                if (json["$typeName"].is_string()) {
                    auto instance_ptr = JsonToObj(json, p_stdstring, error);
                    if (!error.empty()) {
                        if (error_print)
                            L_CORE_ERROR("Meta not valid. Reflection to " + json["$typeName"].string_value() + " failed." + error);
                        return Variant();
                    }
                    return Variant(instance_ptr);
                }
                else {
                    // dictionary
                }
            }

            else if (IsNumericExpression(p_stdstring)) {
                double string_double_value = std::stod(p_stdstring);
                if ((int)(string_double_value) == string_double_value) {
                    return Variant((int64_t)string_double_value);
                }
                return Variant(string_double_value);
            }



        }
        if (!error.empty())
        {

            if(error_print)
                L_CORE_ERROR(error);
            L_CORE_WARN("parse json file {} failed!", p_str);
            return Variant();
        }
    }

    bool ConfigFile::IsNumericExpression(const std::string& expression)
    {
        // 正则表达式模式，用于匹配数字类型的表达式
        std::regex pattern("^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$");

        // 使用 std::regex_match() 函数进行匹配
        return std::regex_match(expression, pattern);
    }

    Error ConfigFile::Save(const String& p_path) {
        Error err;
        Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE, &err);

        if (err) {
            return err;
        }

        return _internalSave(file);
    }
    Error ConfigFile::Load(const String& p_path) {
        Error err;
        Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ, &err);

        if (f.is_null()) {
            return err;
        }

        return ParseFile(f);
    }

    Error ConfigFile::_internalSave(Ref<FileAccess> file) {
        bool first = true;
        for (const KeyValue<String, HashMap<String, Variant>>& E : values) {
            if (first) {
                first = false;
            }
            else {
                file->store_string("\n");
            }
            if (!E.key.is_empty()) {
                file->store_string("[" + E.key.replace("]", "\\]") + "]\n\n");
            }

            for (const KeyValue<String, Variant>& F : E.value) {
                String vstr = ConfigFile::WriteConfigVariant(F.value);
                file->store_string(F.key.property_name_encode() + "=" + vstr + "\n");
            }
        }

        return OK;
    }
    String ConfigFile::WriteConfigVariant(const Variant& p_var) {
        String r_str = "";
        if (VariantWriter::write_to_string(p_var, r_str) == OK) {
            return r_str;
        }
        L_CORE_ERROR("error converting variant to String");
        return "";
    }
}