#pragma once
#ifndef REFLECTION_REGISTER_H
#define REFLECTION_REGISTER_H
namespace lain
{
    namespace Reflection
    {
        class TypeMetaRegister
        {
        public:
            static void metaRegister();
            static void metaUnregister();
            static void EnumMetaRegister();
        };
    } // namespace Reflection
} // namespace lain
#endif
