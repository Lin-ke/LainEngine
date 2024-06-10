#pragma once
#include "precompiled.h"

class Class;
class Global;
class Function;
class Enum;
class ENUM_;
struct SchemaMoudle
{
    std::string name;

    std::vector<std::shared_ptr<Class>> classes;
    std::vector<std::shared_ptr<ENUM_>> enums;


};