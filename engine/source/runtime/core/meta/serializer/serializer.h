#pragma once
#include "base.h"
#include "runtime/core/meta/json.h"
#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/core/templates/vector.h"
#include "runtime/core/string/ustring.h"
#include "core/os/memory.h"
#include <vector>
#include <cassert>
#include <string>
namespace lain
{
    template<typename...>
    inline constexpr bool always_false = false;

    class Serializer
    {
    public:
        template<typename T>
        static Json writePointer(T* instance)
        {
            return Json::object {{"$typeName", Json {"*"}}, {"$context", Serializer::write(*instance)}};
        }

        template<typename T>
        static T*& readPointer(const Json& json_context, T*& instance)
        {
            assert(instance == nullptr);
            std::string type_name = json_context["$typeName"].string_value();
            assert(!type_name.empty());
            if ('*' == type_name[0])
            {
                instance = memnew(T);
                read(json_context["$context"], *instance);
            }
            else
            {
                instance = static_cast<T*>(
                    Reflection::TypeMeta::newFromNameAndJson(type_name, json_context["$context"]).m_instance);
            }
            return instance;
        }

        template<typename T>
        static Json write(const Reflection::ReflectionPtr<T>& instance)
        {
            T*          instance_ptr = static_cast<T*>(instance.operator->());
            std::string type_name    = instance.getTypeName().utf8().get_data();
            return Json::object {{"$typeName", Json(type_name)},
                                  {"$context", Reflection::TypeMeta::writeByName(type_name, instance_ptr)}};
        }

        template<typename T>
        static T*& read(const Json& json_context, Reflection::ReflectionPtr<T>& instance)
        {
            std::string type_name = json_context["$typeName"].string_value();
            instance.setTypeName(type_name);
            return readPointer(json_context, instance.getPtrReference());
        }
        // pointer T*
        template<typename T>
        static Json write(const T& instance)
        {
            if constexpr (std::is_pointer<T>::value)
            {
                return writePointer((T)instance);
            }
            else
            {
#ifdef _MSC_VER
                static_assert(always_false<T>,__FUNCSIG__);
#else
                static_assert(always_false<T>, __PRETTY_FUNCTION__);
#endif            
                
                return Json();
            }
        }

        template<typename T>
        static T& read(const Json& json_context, T& instance)
        {
            if constexpr (std::is_pointer<T>::value)
            {
                return readPointer(json_context, instance);
            }
            else
            {
                static_assert(always_false<T>, __FUNCSIG__);
                return instance;
            }
        }

        template<typename T>
        static Json write(const lain::Vector<T>& instance) {
            Json::array array_json;
            for (int i = 0; i < instance.size();i++) {
                array_json.emplace_back(write(instance[i]));
            }
            return array_json;
        }
        template<typename T>
        static Vector<T>& read(const Json& json_context, lain::Vector<T>& instance) {
            if (!json_context.is_array()) {
                return instance;
            }
            Json::array json_array = json_context.array_items();
            instance.resize(static_cast<int>(json_array.size()));
            for (int i = 0; i < json_array.size(); i++) {
                T newT;
                Serializer::read(json_array[i], instance.write[i]);
                instance.set(i,newT); // 这里交给了T的=实现。
			}

            return instance;
        }

        template<typename ELEM>
        static Json write(const std::vector<ELEM>& cont)
        {
            Json::array v;
            for (auto&& element : cont)
            {
                v.emplace_back(Serializer::write(element));
            }
            return v;
        }
        template<typename T>
        static std::vector<T>& read(const Json& json_context, std::vector<T>& instance)
        {
            if (!json_context.is_array()) {
                return instance;
            }
            Json::array json_array = json_context.array_items();
            instance.resize(json_array.size());
            for (int i = 0; i < json_array.size(); i++) {
                Serializer::read(json_array[i], instance[i]);
            }

            return instance;
        }
        

    };

    // implementation of base types
    template<>
    Json Serializer::write(const char& instance);
    template<>
    char& Serializer::read(const Json& json_context, char& instance);

    template<>
    Json Serializer::write(const int& instance);
    template<>
    int& Serializer::read(const Json& json_context, int& instance);

    template<>
    Json Serializer::write(const unsigned int& instance);
    template<>
    unsigned int& Serializer::read(const Json& json_context, unsigned int& instance);

    template<>
    Json Serializer::write(const float& instance);
    template<>
    float& Serializer::read(const Json& json_context, float& instance);

    template<>
    Json Serializer::write(const double& instance);
    template<>
    double& Serializer::read(const Json& json_context, double& instance);

    template<>
    Json Serializer::write(const bool& instance);
    template<>
    bool& Serializer::read(const Json& json_context, bool& instance);

    template<>
    Json Serializer::write(const std::string& instance);
    template<>
    std::string& Serializer::read(const Json& json_context, std::string& instance);

    template<>
    Json Serializer::write(const String& instance);
    template<>
    String& Serializer::read(const Json& json_context, String& instance);
    // vector
    
    template<>
    Json Serializer::write(const ui64& instance);
    template<>
    ui64& Serializer::read(const Json& json_context, ui64& instance);

    // template<>
    // Json Serializer::write(const Reflection::object& instance);
    // template<>
    // Reflection::object& Serializer::read(const Json& json_context, Reflection::object& instance);

    ////////////////////////////////////
    ////sample of generation coder
    ////////////////////////////////////
    // class test_class
    //{
    // public:
    //     int a;
    //     unsigned int b;
    //     std::vector<int> c;
    // };
    // class ss;
    // class jkj;
    // template<>
    // Json Serializer::write(const ss& instance);
    // template<>
    // Json Serializer::write(const jkj& instance);

    /*REFLECTION_TYPE(jkj)
    CLASS(jkj,Fields)
    {
        REFLECTION_BODY(jkj);
        int jl;
    };

    REFLECTION_TYPE(ss)
    CLASS(ss:public jkj,WhiteListFields)
    {
        REFLECTION_BODY(ss);
        int jl;
    };*/

    ////////////////////////////////////
    ////template of generation coder
    ////////////////////////////////////
    // template<>
    // Json Serializer::write(const test_class& instance);
    // template<>
    // test_class& Serializer::read(const Json& json_context, test_class& instance);

    //
    ////////////////////////////////////
} // namespace lain
