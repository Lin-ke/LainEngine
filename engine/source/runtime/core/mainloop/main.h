#pragma once
#ifndef __MAIN_H__
#define __MAIN_H__
#include "base.h"
namespace lain {

class Main {
	// static
	static uint64_t last_ticks;
	static uint32_t frames;
	static int iterating;

public:
	static bool Loop();
	static Error Initialize(int argc, char* argv[]);
	L_INLINE bool IsInloop() { return (iterating > 0); }
};
}

#endif