#ifndef META_EXAMPLE_H
#define META_EXAMPLE_H
#pragma once
#include "base.h"
#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/core/templates/vector.h"
namespace lain
{
    REFLECTION_TYPE(BaseTest)
    CLASS(BaseTest, WhiteListFields)
    {
        REFLECTION_BODY(BaseTest);

    public:
        int               m_int = 0;
        std::vector<int*> m_int_vector;
    };

    REFLECTION_TYPE(Test1)
    CLASS(Test1 : public BaseTest, WhiteListFields)
    {
        REFLECTION_BODY(Test1);

    public:
        META(Enable)
        char m_char = '1';
    };

    REFLECTION_TYPE(Test2)
    CLASS(Test2 : public BaseTest, WhiteListFields)
    {
        REFLECTION_BODY(Test2);

    public:
        int m_int = 1;
        int bint = 2;

        std::vector<Reflection::ReflectionPtr<BaseTest>> m_test_base_array;
    };

    REFLECTION_TYPE(Test3)
        CLASS(Test3 : public BaseTest, WhiteListFields)
    {
        REFLECTION_BODY(Test3);

    public:
        int m_int = 3;
        int bint = 33;

        Vector<Vector<Reflection::ReflectionPtr<BaseTest>>> m_test_base_array;
    };

    void metaExample();
    
} // namespace lain

#endif