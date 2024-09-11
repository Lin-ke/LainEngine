#pragma once
#ifndef TEST_MARCROS_H
#define TEST_MARCROS_H
#define TEST_STRINGIFY(x) #x
#define TEST_STRINGIFY2(x) TEST_STRINGIFY(x)

#define TEST_CONCAT(a, b) a##b
#define TEST_CONCAT2(x,y) TEST_CONCAT(x,y)
//TEST_STRINGIFY2(TEST_CONCAT2(x, TEST_CONCAT2(!= , y)))
#define TEQ(x, y) if(x!=y) {L_PERROR("err find", #x, "!=", #y);}
//#include "test/doctest.h"
// @TODO 完善DocTEST
namespace lain {
	namespace test {
	}
}
#endif // !TEST_MARCROS_H
