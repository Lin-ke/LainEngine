#pragma once
#ifndef __PROJECT_SETTINGS_H__
#define __PROJECT_SETTINGS_H__
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "core/templates/hash_map.h"
#include "core/string/string_name.h"
#include "core/os/file_access.h"
#include "core/meta/reflection/reflection.h"
#include <regex>
namespace lain {
	class ProjectSettings {
public:
	HashSet<String> custom_features;
	HashMap<StringName, std::vector<Pair<StringName, StringName>>> feature_overrides;
	static ProjectSettings* p_singleton;
	Error Initialize(const String p_path);
	String project_data_dir_name;
	String resource_path = "";
	bool _set(const StringName& p_name, const Variant& p_value);
	bool _get(const StringName& p_name, Variant& r_ret) const;
	Variant GetWithOverride(const StringName& p_name) const;
	String GlobalizePath(const String& path) const;
	/// <summary>
	/// features
	/// </summary>
	/// <returns></returns>
	const PackedStringArray get_required_features();
	static ProjectSettings* GetSingleton() {
		return p_singleton;
	}
	ProjectSettings() {
		p_singleton = this;
	}
	~ProjectSettings() {
		p_singleton = nullptr;
	}
	String GetResourcePath() const { return resource_path; }
	 String GetProjectDataName() const { return project_data_dir_name; }
	 String GetProjectDataPath() const { return "res://" + project_data_dir_name; }
private:
	Error _initialize(const String p_path, bool p_ignore_override = true);
	// load settings
	Error _load_settings_text_or_binary(const String& p_text_path, const String& p_binary_path);
	Error _load_settings_text(const String& text_path);
	Error _load_settings_binary(const String& binary_path);
	
	};

    class ConfigParser {
    public:
         Ref<FileAccess> f;
         void ParseFile(const String& filename) {
            std::ifstream file(CSTR(filename));
            std::string line;

            String currentField;
            bool inValue;
            std::getline(file, line);
            while (!file.eof()) {
                if (IsField(line)) {
                    currentField = GetField(line);
                    inValue = false;
                    std::getline(file, line);
                }
                else if (IsKeyValue(line)) {
                    
                    inValue = true;
                    int delimiterPos = line.find("=", 0);
                    String key = line.substr(0, delimiterPos).c_str(); key = key.trim();
                    String value = line.substr(delimiterPos+1, line.length()).c_str();
                    while (std::getline(file, line)) {
                        if (IsKeyValue(line) || IsField(line) ||  line == "") break;
                        value += line.c_str();
                    }
                    Variant variant_value = ConstructFromString(value);
                    if (variant_value.get_type() == Variant::Type::NIL) {
                        L_CORE_WARN("NIL config meet: " + currentField + "/" + key);
                    }
                    m_hashmap[currentField + "/" + key] =  variant_value;
                    
                }

            }
        }

        

    private:
         HashMap<String, Variant> m_hashmap;

         bool IsField(const std::string& line) {
            return line.size() > 2 && line.front() == '[' && line.back() == ']';
        }

         String GetField(const String& line) {
            return line.substr(1, line.size() - 2);
        }
         // 不允许一个字符串占多行
         // 不允许名中带有引号
         bool IsKeyValue(const std::string& line) {
            int equalpos =  line.find('=');
            if (equalpos == std::string::npos) return false;
            int firstquote = line.find('"');
            if (firstquote < equalpos) return false;
            return true;
            
        }
         // 基本类包括：Vector<Variant>，即[]；double ； String
         Variant ConstructFromString(const String& p_str, int recursize_depth =0 ) {
             if (recursize_depth > 128) {
                 L_CORE_ERROR("than max recursize depth");
                 return Variant();
             }
             if (p_str == "") 
                 return Variant();
             if (p_str.begins_with("Packed")) {
                 
            }
             else if (p_str.begins_with("\"")) {
                 return Variant(p_str);
             }
             else {
                 std::string p_stdstring = p_str.utf8().get_data();
                 std::string error;
                 if (p_str.begins_with("{") || p_str.begins_with("[") ) {
                     auto&& json = Json::parse(p_stdstring, error);

                 }
                 else if (p_str.contains("{")) {
                     int brevepos = p_stdstring.find("{");
                     std::string class_name = trim(p_stdstring.substr(0, brevepos));

                     std::string class_data = p_stdstring.substr(brevepos, p_stdstring.length() - brevepos);
                     auto meta = Reflection::TypeMeta::newMetaFromName(class_name);
                     if (!meta.isValid()) {
                         error = "Meta not valid. Reflection to " + class_name + " failed.";
                         return Variant();
                     }
                     // do something
                     
                     // 这里应该建立这个类型的对象
                 }  
                 else if (IsNumericExpression(p_stdstring)) {
                     double string_double_value = std::stod(p_stdstring);
                     if ((int)(string_double_value) == string_double_value) {
                         return Variant((int)string_double_value);
                     }
                     return Variant(string_double_value);
                 }


                if (!error.empty())
                {
                    L_CORE_WARN("parse json file {} failed!", p_str);
                     return Variant();
                 }
             }

         }

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

         bool IsNumericExpression(const std::string& expression) {
             // 正则表达式模式，用于匹配数字类型的表达式
             std::regex pattern("^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$");

             // 使用 std::regex_match() 函数进行匹配
             return std::regex_match(expression, pattern);
         }

    };

}

#endif // !_PROJECT_SETTINGS_H__
