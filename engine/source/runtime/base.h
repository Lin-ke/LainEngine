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
// error.h in typedefs.h
#include "core/typedefs.h"
#include <core/log/log.h>
#include <sstream>
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
	std::ostringstream  ss;
	std::initializer_list <int> { ([&args, &ss] {ss << (args) << " "; }(), 0)...};
	L_INFO(ss.str());
}

template <typename ... Types>
void  L_PERROR(const Types&... args)
{
	std::ostringstream  ss;
	std::initializer_list <int> { ([&args, &ss] {ss << (args) << " "; }(), 0)...};
	L_ERROR(ss.str());
}


template <typename ... Types>
L_INLINE void L_CORE_PRINT(const Types&... args)
{
	std::initializer_list <int> { ([&args] {L_CORE_INFO(args); }(), 0)...};
}

# define L_JSON(x) L_PRINT(__FUNCTION__, __LINE__, "json of ",#x,lain::Serializer::write(x).dump());
# define L_CORE_JSON(x) L_CORE_PRINT("json of " + std::string(#x) + " " + (lain::Serializer::write(x).dump()));


//#define refcount(type, x) (*(reinterpret_cast<s_u32*> (const_cast<type*>(x.ptr())) - 2)).get()
# define refcount(x) x._refcount()
void _global_lock();
void _global_unlock();


#endif // __BASE__

// 成员变量 首字母标识
// 函数：大驼峰， 私有函数前面加_
//