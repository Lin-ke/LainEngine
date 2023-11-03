#pragma once
#ifdef L_DEBUG
	#if defined(L_PLATFORM_WINDOWS)
		#define L_DEBUGBREAK() __debugbreak()
	#elif defined(L_PLATFORM_LINUX)
		#include <signal.h>
		#define L_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
#endif
#define L_ENABLE_ASSERTS
#else
#define L_DEBUGBREAK()
#endif

#define L_EXPAND_MACRO(x) x
#define L_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)


#include "core/log/log.h"
