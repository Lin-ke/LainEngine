#pragma once
#ifndef META_SERIALIZER_H
#define META_SERIALIZER_H
#include "base.h"
#include "core/os/memory.h"
#include "core/templates/hash_map.h"
#include "core/variant/dictionary.h"
#include "runtime/core/meta/json.h"
#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/core/string/ustring.h"
#include "runtime/core/templates/vector.h"
namespace lain {
class Object;
template <typename...>
inline constexpr bool always_false = false;
// 应该建立一个 Map:<String(type_name), read\write> 这个是有的，然后向内部注入有关只serializer不反射的类
class Serializer {
 public:
  template <typename T>
  static Json writePointer(T* instance) {
    if constexpr (std::is_base_of_v<Object, T>) {
      return Json::object{{"$typeName", Json{CSTR(instance->get_class())}}, {"$context", Serializer::write(*instance)}};
    }
    return Json::object{{"$typeName", Json{"*"}}, {"$context", Serializer::write(*instance)}};
  }

  template <>
  static Json writePointer(Object* instance);

  template <typename T>
  static T*& readPointer(const Json& json_context, T*& instance) {
    assert(instance == nullptr);
    String type_name = json_context["$typeName"].string_value();
    assert(!type_name.is_empty());
    if ('*' == type_name[0]) {
      instance = memnew(T);
      read(json_context["$context"], *instance);
    } else {
      instance = static_cast<T*>(Reflection::TypeMeta::newFromNameAndJson(type_name, json_context["$context"]).m_instance);
    }
    return instance;
  }

  template <typename T>
  static Json write(const Reflection::ReflectionPtr<T>& instance) {
    T* instance_ptr = static_cast<T*>(instance.operator->());
    String type_name = instance.getTypeName().utf8().get_data();
    return Json::object{{"$typeName", Json(type_name)}, {"$context", Reflection::TypeMeta::writeByName(type_name, instance_ptr)}};
  }

  template <typename T>
  static void read(const Json& json_context, Reflection::ReflectionPtr<T>& instance) {
    String type_name = json_context["$typeName"].string_value();
    instance.setTypeName(type_name);
    readPointer(json_context, instance.getPtrReference());
  }
  // pointer T*
  template <typename T>
  static Json write(const T& instance) {
    if constexpr (std::is_pointer<T>::value) {
      return writePointer((T)instance);
    } else {
#ifdef _MSC_VER
      static_assert(always_false<T>, __FUNCSIG__);
#else
      static_assert(always_false<T>, __PRETTY_FUNCTION__);
#endif

      return Json();
    }
  }

  template <typename T>
  static void read(const Json& json_context, T& instance) {
    if constexpr (std::is_pointer<T>::value) {
      return readPointer(json_context, instance);
    } else {
      static_assert(always_false<T>, __FUNCSIG__);
      return instance;
    }
  }

  template <typename T>
  static Json write(const Vector<T>& instance) {
    Json::array array_json;
    for (int i = 0; i < instance.size(); i++) {
      array_json.push_back(write(instance[i]));
    }
    return array_json;
  }
  template <typename T>
  static void read(const Json& json_context, lain::Vector<T>& instance) {
    if (!json_context.is_array()) {
      return instance;
    }
    Json::array json_array = json_context.array_items();
    instance.resize(static_cast<int>(json_array.size()));
    for (int i = 0; i < json_array.size(); i++) {
      T newT;
      Serializer::read(json_array[i], instance.write[i]);
      instance.set(i, newT);  // 这里交给了T的=实现。
    }

    return instance;
  }

  template <typename ELEM>
  static Json write(const std::vector<ELEM>& cont) {
    Json::array v;
    for (auto&& element : cont) {
      v.emplace_back(Serializer::write(element));
    }
    return v;
  }
  template <typename T>
  static void read(const Json& json_context, std::vector<T>& instance) {
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

  template <typename K, typename V>
  static void read(const Json& json_context, HashMap<K, V>& instance) {
    for (const auto& pair : json_context.object_items()) {
      K k;
      V v;
      Serializer::read(pair.key, k);
      Serializer::read(pair.value, v);
      instance[k] = v;
    }
    return ;
  }

  template <typename K, typename V>
  static Json write(const HashMap<K, V>& instance) {
    Json::object ret_context;
    // 这里需要避免双引号
    for (const KeyValue<K, V>& E : instance) {
      Json obj = Serializer::write(E.key);
      if (obj.is_string()) {
        ret_context.insert(obj.string_value(), Serializer::write(E.value));
      } else {
        ret_context.insert(Serializer::write(E.key).dump(), Serializer::write(E.value));
      }
    }
    return ret_context;
  }
};
// implementation of base types
template <>
Json Serializer::write(const char& instance);
template <>
void Serializer::read(const Json& json_context, char& instance);

template <>
Json Serializer::write(const int& instance);
template <>
void Serializer::read(const Json& json_context, int& instance);

template <>
Json Serializer::write(const unsigned int& instance);
template <>
void Serializer::read(const Json& json_context, unsigned int& instance);

template <>
Json Serializer::write(const float& instance);
template <>
void Serializer::read(const Json& json_context, float& instance);

template <>
Json Serializer::write(const double& instance);
template <>
void Serializer::read(const Json& json_context, double& instance);

template <>
Json Serializer::write(const bool& instance);
template <>
void Serializer::read(const Json& json_context, bool& instance);

template <>
Json Serializer::write(const String& instance);
template <>
void Serializer::read(const Json& json_context, String& instance);

template<>
Json Serializer::write(const StringName& instance);
template<>
void Serializer::read(const Json& json_context, StringName& instance);
template <>
Json Serializer::write(const String& instance);
template <>
void Serializer::read(const Json& json_context, String& instance);
template <>
Json Serializer::write(const Dictionary& instance);
template <>
void Serializer::read(const Json& json_context, Dictionary& instance);
template<>
void Serializer::read(const Json& json_context, GObjectPath& instance);
template<>
Json Serializer::write(const GObjectPath& instance);
// vector

template <>
Json Serializer::write(const ui64& instance);
template <>
void Serializer::read(const Json& json_context, ui64& instance);
template <>
Json Serializer::write(const Variant& instance);
template <>
void Serializer::read(const Json& json_context, Variant& instance);
template <>
Json Serializer::write(const Array& instance);
template <>
void Serializer::read(const Json& json_context, Array& instance);

// template<>
// Json Serializer::write(const Reflection::object& instance);
// template<>
// void Serializer::read(const Json& json_context, Reflection::object& instance);

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
// void Serializer::read(const Json& json_context, test_class& instance);

//
////////////////////////////////////
}  // namespace lain

#endif