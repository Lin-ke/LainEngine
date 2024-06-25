#include "serializer.h"
#include <assert.h>
#include "core/templates/hash_map.h"
#include "core/object/object.h"
#include "core/variant/variant.h"
#include "_generated/serializer/all_serializer.h"
#include "core/variant/variant_helper.h"
namespace lain
{

    template<>
    Json Serializer::write(const char& instance)
    {
        return Json(instance);
    }
    template<>
    static Json Serializer::writePointer(Object* instance)
    {
        return Json::object{ {"$typeName", Json {CSTR(instance->get_class())}}, {"$context", Serializer::write(*instance)} };
    }

    template<>
    char& Serializer::read(const Json& json_context, char& instance)
    {
        assert(json_context.is_number());
        return instance = static_cast<char>(json_context.number_value());
    }

    template<>
    Json Serializer::write(const int& instance)
    {
        return Json(instance);
    }
    template<>
    int& Serializer::read(const Json& json_context, int& instance)
    {
        assert(json_context.is_number());
        return instance = static_cast<int>(json_context.number_value());
    }

    template<>
    Json Serializer::write(const unsigned int& instance)
    {
        return Json(static_cast<int>(instance));
    }
    template<>
    Json Serializer::write(const ui64& instance)
    {
        return Json(static_cast<double>(instance));
    }
    template<>
    ui64& Serializer::read(const Json& json_context, ui64& instance) {
        assert(json_context.is_number());
        return instance = static_cast<ui64>(json_context.number_value());
    }
    template<>
    unsigned int& Serializer::read(const Json& json_context, unsigned int& instance)
    {
        assert(json_context.is_number());
        return instance = static_cast<unsigned int>(json_context.number_value());
    }

    template<>
    Json Serializer::write(const float& instance)
    {
        return Json(instance);
    }
    template<>
    float& Serializer::read(const Json& json_context, float& instance)
    {
        assert(json_context.is_number());
        return instance = static_cast<float>(json_context.number_value());
    }

    template<>
    Json Serializer::write(const double& instance)
    {
        return Json(instance);
    }
    template<>
    double& Serializer::read(const Json& json_context, double& instance)
    {
        assert(json_context.is_number());
        return instance = static_cast<float>(json_context.number_value());
    }

    template<>
    Json Serializer::write(const bool& instance)
    {
        return Json(instance);
    }
    template<>
    bool& Serializer::read(const Json& json_context, bool& instance)
    {
        assert(json_context.is_bool());
        return instance = json_context.bool_value();
    }

    template<>
    Json Serializer::write(const std::string& instance)
    {
        return Json(instance);
    }
    template<>
    std::string& Serializer::read(const Json& json_context, std::string& instance)
    {
        assert(json_context.is_string());
        return instance = json_context.string_value();
    }

    template<>
    Json Serializer::write(const String& instance)
    {
        return Json(std::string(instance.utf8().get_data()));
    }
    template<>
    String& Serializer::read(const Json& json_context, String& instance)
    {
        assert(json_context.is_string());
        // 这里有一点左值右值的问题
        instance = json_context.string_value().c_str();
        return instance;
    }
    template<>
    Json Serializer::write(const Dictionary& instance)
    {
        Json::object  ret_context;
        List<Variant> keys;
        instance.get_key_list(&keys);
        for (const Variant& E : keys) {
            ret_context.insert_or_assign(CSTR(E.operator String()), Serializer::write(instance[E]));
        }
        return ret_context;
    }
    template<>
    Dictionary& Serializer::read(const Json& json_context, Dictionary& instance)
    {
        assert(json_context.is_object());
        // 这里有一点左值右值的问题
        for (const auto& pair : json_context.object_items()) {
            Variant key(pair.first.c_str());
            Variant value;
            value = Serializer::read(pair.second, value);
            instance[key] = value;
        }
        return instance;
    }
   // variant 和 reflectptr是一样的
     template<>
     Json Serializer::write(const Variant& instance)
    {
         switch (instance.get_type()) {
             // basic types
         case Variant::NIL:
             return "null";
         case Variant::BOOL:
             return Json(instance.operator bool());
         case Variant::INT:
             return Json(instance.operator int());
         case Variant::FLOAT:
             return Json(instance.operator float());
         case Variant::PACKED_INT32_ARRAY: // vector
         case Variant::PACKED_INT64_ARRAY:
         case Variant::PACKED_FLOAT32_ARRAY:
         case Variant::PACKED_FLOAT64_ARRAY:
         case Variant::PACKED_STRING_ARRAY:
         case Variant::ARRAY:   // vector<variant>
             return Serializer::write(instance.operator Array());
         case Variant::STRING:
            return Json(CSTR(*reinterpret_cast<const String *>(instance._data._mem)));
         case Variant::OBJECT:
         {
             
             Object* obj = instance.operator Object * ();
             Reflection::ReflectionPtr rptr = Reflection::ReflectionPtr(obj->get_c_class(), obj);
             return Serializer::write(rptr);
         }
         default:
         {
             // plan A :从typename到Type，然后转
             if (VariantHelper::is_serializable(instance)) {
                 const char* type_name = Variant::get_c_type_name(instance.get_type());
                 return Json::object{ 
                     {"$typeName", Json(type_name)},
                     {"$context", Reflection::TypeMeta::writeByName(type_name, const_cast<void*>(reinterpret_cast<const void*>(instance._data._mem)) )} };
                 
             }
             return Serializer::write(String(instance));
         }
         }
    }
     template<>
     Variant& Serializer::read(const Json& json_context, Variant& instance)
    {
         switch (json_context.type()) {
         case Json::STRING:
         {
             instance.set_type(Variant::STRING);
             instance = String(json_context.string_value());
             return instance;
         }
         case Json::BOOL:
         {
             instance.set_type(Variant::BOOL);
             instance = json_context.bool_value();
             return instance;
         }
         case Json::NUMBER:
         {
             double num = json_context.number_value();
             if (static_cast<int>(num) == num) {
                 instance.set_type(Variant::INT);
                 instance = static_cast<int>(num);
             }
             else {

                 instance.set_type(Variant::FLOAT);
                 instance = json_context.number_value();
             }

             return instance;
         }
         case Json::ARRAY:
         {
             Json::array json_array = json_context.array_items();
             Array variant_arr;
             variant_arr.resize(static_cast<int>(json_array.size()));
             for (int i = 0; i < json_array.size(); i++) {
                 Serializer::read(json_array[i], variant_arr[i]);
             }
              instance = variant_arr;
              return instance;
         }

         case Json::OBJECT:
         {
             std::string type_name = json_context["$typeName"].string_value();
             ERR_FAIL_COND_V_MSG(!VariantHelper::is_serializable_type(type_name.c_str()), instance, "type cant be reflected");
             Reflection::TypeMeta::writeToInstanceFromNameAndJson(type_name, json_context["$context"], instance._data._mem);
            instance.type = VariantHelper::get_type_from_name(type_name.c_str());
            ERR_FAIL_COND_V_MSG(instance.type == Variant::NIL, instance, type_name.c_str());
            
           
             return instance;
         }
         default:
             return instance;
         }

    	return instance;
    }
     template<>
     Json Serializer::write(const Array& instance) {
         Json::array array_json;
         for (int i = 0; i < instance.size(); i++) {
             array_json.emplace_back(write(instance[i]));
         }
         return array_json;
    }
     template<> // array 是variant<int>需要写一下
     Array& Serializer::read(const Json& json_context, Array& instance) {
         if (!json_context.is_array()) {
             return instance;
         }
         Json::array json_array = json_context.array_items();
         instance.resize(static_cast<int>(json_array.size()));
         for (int i = 0; i < json_array.size(); i++) {
             Serializer::read(json_array[i], instance[i]);
         }

         return instance;
     }


    //////////////////////////////////
    // template of generation coder
    //////////////////////////////////

    /*template<>
    Json Serializer::write(const ss& instance)
    {
        return Json();
    }
    template<>
    Json Serializer::write(const jkj& instance)
    {
        return Json();
    }

    template<>
    Json Serializer::write(const test_class& instance)
    {
        Json::array aa;
        for(auto& item: instance.c)
        {
            aa.emplace_back(Serializer::write(item));
        }
        ss jj;
        reflection::object* jjj1 = &jj;
        auto kkk = Serializer::write(jjj1);
        auto jjj = kkk.dump();
        return Json::object{
            {"a",Serializer::write(instance.a)},
            {"b",Serializer::write(instance.b)},
            {"c",aa}
        };
    }
    template<>
    test_class& Serializer::read(const Json& json_context, test_class& instance)
    {
        assert(json_context.is_object());
        Serializer::read(json_context["a"], instance.a);
        Serializer::read(json_context["b"], instance.b);

        assert(json_context["c"].is_array());
        Json::array cc = json_context["c"].array_items();
        instance.c.resize(cc.size());
        for (size_t index=0; index < cc.size();++index)
        {
            Serializer::read(cc[index], instance.c[index]);
        }
        return instance;
    }*/

    ////////////////////////////////////
} // namespace lain