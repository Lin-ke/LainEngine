#pragma once
#ifndef REFLECTION_H
#define REFLECTION_H
#include "runtime/core/meta/json.h"
#include "runtime/core/templates/vector.h"
#include "core/string/ustring.h"
#include "reflection_marcos.h"
#include "core/os/memory.h"
#include "core/templates/hash_map.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
namespace lain
{


    template<typename T, typename U, typename = void>
    struct is_safely_castable : std::false_type
    {};
    // 编译器转换，但是引擎中大量的是运行期转换
    template<typename T, typename U>
    struct is_safely_castable<T, U, std::void_t<decltype(static_cast<U>(std::declval<T>()))>> : std::true_type
    {};

    namespace Reflection
    {
        class TypeMeta;
        class FieldAccessor;
        class MethodAccessor;
        class ArrayAccessor;
        class ReflectionInstance;
    } // namespace Reflection
    typedef std::function<void(void*, void*)>      SetFuncion;
    typedef std::function<void*(void*)>            GetFuncion;
    typedef std::function<const char*()>           GetNameFuncion;
    typedef std::function<void(int, void*, void*)> SetArrayFunc;
    typedef std::function<void*(int, void*)>       GetArrayFunc;
    typedef std::function<int(void*)>              GetSizeFunc;
    typedef std::function<bool()>                  GetBoolFunc;
    typedef std::function<void(void*)>             InvokeFunction;

    typedef std::function<void*(const Json&)>                           ConstructorWithJson;
    typedef std::function<Json(void*)>                                  WriteJsonByName;
    typedef std::function<int(Reflection::ReflectionInstance*&, void*)> GetBaseClassReflectionInstanceListFunc;

    typedef std::function<void(const Json&, void*)> SerialRead;
    typedef std::function<Json(void*)> SerialWrite;
    typedef std::function<void*(int)>               AllocMemArrFunc;
    typedef std::function<void*(void*)>                  AllocMemFunc;
    typedef std::function<size_t()>                 GetSizeOfFunc;




    typedef std::tuple<SetFuncion, GetFuncion, GetNameFuncion, GetNameFuncion, GetNameFuncion, GetBoolFunc>
                                                       FieldFunctionTuple;
    typedef std::tuple<GetNameFuncion, InvokeFunction> MethodFunctionTuple;
    typedef std::tuple<GetBaseClassReflectionInstanceListFunc, ConstructorWithJson, WriteJsonByName, SerialRead, AllocMemFunc, AllocMemArrFunc, GetSizeOfFunc> ClassFunctionTuple;

    typedef std::tuple<SetArrayFunc, GetArrayFunc, GetSizeFunc, GetNameFuncion, GetNameFuncion>      ArrayFunctionTuple;
    
    class StringName;
    namespace Reflection
    {
        
        
        class TypeMetaRegisterinterface
        {
        public:
            static void registerToClassMap(const char* name, ClassFunctionTuple* value);

            static void registerToFieldMap(const char* name, FieldFunctionTuple* value);

            static void registerToMethodMap(const char* name, MethodFunctionTuple* value);
            static void registerToArrayMap(const char* name, ArrayFunctionTuple* value);
            static void registerToEnumMap(const char* name, const HashMap<StringName, int>& value);
            static void registerToAnounymousEnumMap(const char* name, const HashMap<StringName, int>& value);


            static void unregisterAll();
        };
        class TypeMeta
        {
            friend class FieldAccessor;
            friend class ArrayAccessor;
            friend class TypeMetaRegisterinterface;

        public:
            TypeMeta();

            // static void Register();

            static TypeMeta newMetaFromName(String type_name);
            static ReflectionInstance newFromJson(const Json& json_context);
            static bool     writeToInstanceFromNameAndJson(const String & type_name, const Json& json_context, void* instance);
            static bool               newArrayAccessorFromName(const String & array_type_name, ArrayAccessor& accessor);
            static ReflectionInstance newFromNameAndJson(const String & type_name, const Json& json_context);
            static Json               writeByName(const String & type_name, void* instance);
            static bool is_valid_type(const char* p_typename);
            static bool is_valid_type(const String& p_typename);

            static size_t getSizeOfByName(const char* name);
            static size_t getSizeOfByName(const String & name);
            static void* memnewByName(const char* name, void* target = nullptr);
            static void* memnewarrByName(const char* name, int num);
            static StringName GetEnumString(const StringName& p_class, int value);
            static int GetEnumValue(const StringName& p_class, const StringName& p_name);


            String getTypeName() const;

            int getFieldsList(FieldAccessor*& out_list) const;
            int getMethodsList(MethodAccessor*& out_list) const;

            int getBaseClassReflectionInstanceList(ReflectionInstance*& out_list, void* instance) const;

            FieldAccessor getFieldByName(const char* name) const;
            MethodAccessor getMethodByName(const char* name) const;

            bool isValid() const { return m_is_valid; }

            TypeMeta& operator=(const TypeMeta& dest);
           
            
        private:
            TypeMeta(String type_name);

        private:
            Vector<FieldAccessor>   m_fields;
            Vector<MethodAccessor> m_methods;
            String              m_type_name;

            bool m_is_valid;
        };

        class FieldAccessor
        {
            friend class TypeMeta;

        public:
            FieldAccessor();

            void* get(void* instance);
            void  set(void* instance, void* value);

            TypeMeta getOwnerTypeMeta();

            /**
             * param: TypeMeta out_type
             *        a reference of TypeMeta
             *
             * return: bool value
             *        true: it's a reflection type
             *        false: it's not a reflection type
             */
            bool        getTypeMeta(TypeMeta& field_type);
            const char* getFieldName() const;
            const char* getFieldTypeName();
            bool        isArrayType();
            bool        isValid();


            FieldAccessor& operator=(const FieldAccessor& dest);

        private:
            FieldAccessor(FieldFunctionTuple* functions);

        private:
            FieldFunctionTuple* m_functions;
            const char*         m_field_name;
            const char*         m_field_type_name;
        };
        class MethodAccessor
        {
            friend class TypeMeta;

        public:
            MethodAccessor();

            void invoke(void* instance);

            const char* getMethodName() const;

            MethodAccessor& operator=(const MethodAccessor& dest);
            
            bool isValid();
        private:
            MethodAccessor(MethodFunctionTuple* functions);

        private:
            MethodFunctionTuple* m_functions;
            const char*          m_method_name;
        };
        /**
         *  Function reflection is not implemented, so use this as an Vector accessor
         */
        class ArrayAccessor
        {
            friend class TypeMeta;

        public:
            ArrayAccessor();
            const char* getArrayTypeName();
            const char* getElementTypeName();
            void        set(int index, void* instance, void* element_value);

            void* get(int index, void* instance);
            int   getSize(void* instance);

            ArrayAccessor& operator=(ArrayAccessor& dest);

        private:
            ArrayAccessor(ArrayFunctionTuple* array_func);

        private:
            ArrayFunctionTuple* m_func;
            const char*         m_array_type_name;
            const char*         m_element_type_name;
        };

        // @TODO 怎么正确管理reflection instance的生命周期
        class ReflectionInstance
        {
        public:
            ReflectionInstance(TypeMeta meta, void* instance) : m_meta(meta), m_instance(instance) {} // 带一个指针的类型信息 @TODO可以看看Java这种语言是怎么做的
            //@TODO vector是cow的；还有个指针，应该没有额外开销，这里以后看看
            ReflectionInstance (ReflectionInstance&& r_val) noexcept: m_meta(r_val.m_meta), m_instance(r_val.m_instance) {
                r_val.m_instance = nullptr; // is it necessary?
            }
            ReflectionInstance(Json json_context) : m_meta(), m_instance(nullptr) {
                *this = TypeMeta::newFromJson(json_context);
            }
            ReflectionInstance(const ReflectionInstance& l_val) : m_meta(l_val.m_meta), m_instance(l_val.m_instance){}


            ReflectionInstance() : m_meta(), m_instance(nullptr) {}

            ReflectionInstance& operator=(ReflectionInstance& dest);

            ReflectionInstance& operator=(ReflectionInstance&& dest);
            String writeByName(void * p_instance) {
                auto json =  TypeMeta::writeByName(m_meta.getTypeName(), p_instance);
                return json.dump();
            }
            void* ptr() { return m_instance; }
            StringName get_name(){
                return StringName(m_meta.getTypeName());
            }
            

        public:
            TypeMeta m_meta;
            void*    m_instance;
        };

        template<typename T>
        class ReflectionPtr
        {
            template<typename U>
            friend class ReflectionPtr;

        public:
            ReflectionPtr(String type_name, T* instance) : m_type_name(type_name), m_instance(instance) {}
            ReflectionPtr() : m_type_name(), m_instance(nullptr) {}


            ReflectionPtr(const ReflectionPtr& dest) : m_type_name(dest.m_type_name), m_instance(dest.m_instance) {}

            template<typename U /*, typename = typename std::enable_if<std::is_safely_castable<T*, U*>::value>::type */>
            ReflectionPtr<T>& operator=(const ReflectionPtr<U>& dest)
            {
                if (this == static_cast<void*>(&dest))
                {
                    return *this;
                }
                m_type_name = dest.m_type_name;
                m_instance  = static_cast<T*>(dest.m_instance);
                return *this;
            }

            template<typename U /*, typename = typename std::enable_if<std::is_safely_castable<T*, U*>::value>::type*/>
            ReflectionPtr<T>& operator=(ReflectionPtr<U>&& dest)
            {
                if (this == static_cast<void*>(&dest))
                {
                    return *this;
                }
                m_type_name = dest.m_type_name;
                m_instance  = static_cast<T*>(dest.m_instance);
                return *this;
            }

            ReflectionPtr<T>& operator=(const ReflectionPtr<T>& dest)
            {
                if (this == &dest)
                {
                    return *this;
                }
                m_type_name = dest.m_type_name;
                m_instance  = dest.m_instance;
                return *this;
            }

            ReflectionPtr<T>& operator=(ReflectionPtr<T>&& dest)
            {
                if (this == &dest)
                {
                    return *this;
                }
                m_type_name = dest.m_type_name;
                m_instance  = dest.m_instance;
                return *this;
            }

            String getTypeName() const { return m_type_name; }

            void setTypeName(String name) { m_type_name = name; }

            bool operator==(const T* ptr) const { return (m_instance == ptr); }

            bool operator!=(const T* ptr) const { return (m_instance != ptr); }

            bool operator==(const ReflectionPtr<T>& rhs_ptr) const { return (m_instance == rhs_ptr.m_instance); }

            bool operator!=(const ReflectionPtr<T>& rhs_ptr) const { return (m_instance != rhs_ptr.m_instance); }

            template<
                typename T1 /*, typename = typename std::enable_if<std::is_safely_castable<T*, T1*>::value>::type*/>
            explicit operator T1*()
            {
                return static_cast<T1*>(m_instance);
            }

            template<
                typename T1 /*, typename = typename std::enable_if<std::is_safely_castable<T*, T1*>::value>::type*/>
            operator ReflectionPtr<T1>()
            {
                return ReflectionPtr<T1>(m_type_name, (T1*)(m_instance));
            }

            template<
                typename T1 /*, typename = typename std::enable_if<std::is_safely_castable<T*, T1*>::value>::type*/>
            explicit operator const T1*() const
            {
                return static_cast<T1*>(m_instance);
            }

            template<
                typename T1 /*, typename = typename std::enable_if<std::is_safely_castable<T*, T1*>::value>::type*/>
            operator const ReflectionPtr<T1>() const
            {
                return ReflectionPtr<T1>(m_type_name, (T1*)(m_instance));
            }

            T* operator->() { return m_instance; }

            T* operator->() const { return m_instance; }

            T& operator*() { return *(m_instance); }

            T* getPtr() { return m_instance; }

            T* getPtr() const { return m_instance; }

            const T& operator*() const { return *(static_cast<const T*>(m_instance)); }

            T*& getPtrReference() { return m_instance; }

            operator bool() const { return (m_instance != nullptr); }

        private:
            String m_type_name {""};
            typedef T   m_type;
            T*          m_instance {nullptr};
        };

    } // namespace Reflection

} // namespace lain
#endif