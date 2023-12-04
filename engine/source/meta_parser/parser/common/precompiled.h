#pragma once

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <clang-c/Index.h>
namespace fs = std::filesystem;

#include "meta/meta_data_config.h"
#include "meta/meta_utils.h"

#include "mustache.hpp"

namespace Mustache = kainjow::mustache;

//#define L_DEBUG_GENERATOR true
////#define L_DEBUG_GENERATOR false
//#define L_PRINT(...) if(L_DEBUG_GENERATOR) l_print(__FUNCTION__, __VA_ARGS__);
//template <typename ... Types>
//void  l_print(const Types&... args)
//{
//    std::ostringstream  ss;
//    std::initializer_list <int> { ([&args, &ss] {ss << (args) << "\t"; }(), 0)...};
//    std::cout << ss.str() << std::endl;
//}
