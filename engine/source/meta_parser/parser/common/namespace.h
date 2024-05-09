#pragma once

#include <filesystem>
#include <vector>

typedef std::vector<std::string> Namespace;

#include <sstream>
#include <ostream>
#include <initializer_list>
#include <iostream>
template<typename ... T>
void L_PRINT(const T& ... args) {
    std::ostringstream  ss;

    std::initializer_list <int> { ([&args, &ss] {
        {
            ss << args << " ";
        }
        }(), 0)...};
    std::cout << ss.str() << std::endl;
}
