#pragma once
#ifndef __PROJECT_SETTINGS_H__
#define __PROJECT_SETTINGS_H__
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "core/templates/hash_map.h"
#include "core/string/string_name.h"
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

    class KeyValueParser {
    public:
         void parseFile(const String& filename) {
            std::ifstream file(CSTR(filename));
            std::string line;

            String currentField;

            while (std::getline(file, line)) {
                if (isField(line)) {
                    currentField = getField(line);
                }
                else if (isKeyValue(line)) {
                    auto keyValue = getKeyValue(line);
                    String key = currentField + "/" + keyValue.first;
                    m_hashmap[key] = keyValue.second;
                }
            }
        }

        std::string getValue(const std::string& field, const std::string& key) {
            std::string fullKey = field + "/" + key;
            if (m_hashmap.find(fullKey) != m_hashmap.end()) {
                return m_hashmap[fullKey];
            }
            else {
                return "";  // 如果键不存在，则返回空字符串
            }
        }

    private:
         HashMap<String, Variant> m_hashmap;

         bool isField(const std::string& line) {
            return line.size() > 2 && line.front() == '[' && line.back() == ']';
        }

         String getField(const String& line) {
            return line.substr(1, line.size() - 2);
        }

         bool isKeyValue(const std::string& line) {
            return line.find('=') != std::string::npos;
        }

         Pair<String, Variant> getKeyValue(const String& line) {
            int delimiterPos = line.find("=", 0);
            String key = line.substr(0, delimiterPos).trim();
            Variant value = constructFromString(line.substr(delimiterPos + 1).trim());
            return Pair(key, value);
        }
         Variant constructFromString(const String& p_str) {
             if (p_str.begins_with("PackedStringArray")) {

            }
             else if (p_str.begins_with("\"")) {

             }
             
         }

    };

}

#endif // !_PROJECT_SETTINGS_H__
