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


#include <core/log/log.h>

#define L_CORE_TRACE(...)	Lain::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define L_CORE_INFO(...)	Lain::Log::GetCoreLogger()->info(__VA_ARGS__)
#define L_CORE_WARN(...)	Lain::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define L_CORE_ERROR(...)	Lain::Log::GetCoreLogger()->error(__VA_ARGS__)
#define L_CORE_FATAL(...)	Lain::Log::GetCoreLogger()->fatal(__VA_ARGS__)

#define L_TRACE(...)	Lain::Log::GetClientLogger()->trace(__VA_ARGS__)
#define L_INFO(...)	Lain::Log::GetClientLogger()->info(__VA_ARGS__)
#define L_WARN(...)	Lain::Log::GetClientLogger()->warn(__VA_ARGS__)
#define L_ERROR(...)	Lain::Log::GetClientLogger()->error(__VA_ARGS__)
#define L_FATAL(...)	Lain::Log::GetClientLogger()->fatal(__VA_ARGS__)

// inline:
// # define L_INLINE _FORCE_INLINE_
# define L_INLINE inline

