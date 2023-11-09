#pragma once
# ifndef __BASE__
# define __BASE__
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

#include <core/log/log.h>

#define L_CORE_TRACE(...)	lain::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define L_CORE_INFO(...)	lain::Log::GetCoreLogger()->info(__VA_ARGS__)
#define L_CORE_WARN(...)	lain::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define L_CORE_ERROR(...)	lain::Log::GetCoreLogger()->error(__VA_ARGS__)
#define L_CORE_FATAL(...)	lain::Log::GetCoreLogger()->fatal(__VA_ARGS__)

#define L_TRACE(...)	lain::Log::GetClientLogger()->trace(__VA_ARGS__)
#define L_INFO(...)	lain::Log::GetClientLogger()->info(__VA_ARGS__)
#define L_WARN(...)	lain::Log::GetClientLogger()->warn(__VA_ARGS__)
#define L_ERROR(...)	lain::Log::GetClientLogger()->error(__VA_ARGS__)
#define L_FATAL(...)	lain::Log::GetClientLogger()->fatal(__VA_ARGS__)

// inline:
// # define L_INLINE __forceinline 
# define L_INLINE __forceinline 



template <typename ... Types>
void  L_PRINT(const Types&... args)
{
	std::initializer_list <int> { ([&args] {L_INFO(args); }(), 0)...};
}

template <typename ... Types>
L_INLINE void L_CORE_PRINT(const Types&... args)
{
	std::initializer_list <int> { ([&args] {L_CORE_INFO(args); }(), 0)...};
}

# define L_JSON(x) L_PRINT("json of " + std::string(#x) + " " + (lain::Serializer::write(x).dump()));
# define L_CORE_JSON(x) L_CORE_PRINT("json of " + std::string(#x) + " " + (lain::Serializer::write(x).dump()));

#endif // __BASE__