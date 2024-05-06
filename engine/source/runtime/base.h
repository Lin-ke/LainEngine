#pragma once
# ifndef __BASE__
# define __BASE__

#define VERSION_BRANCH "0.1"
#define LAINDIRNAME "Lain"
#define BASENAMESPACE lain
//#define PROJECT_FILE_NAME "prj.txt"
//#define PROJECT_DATA_DIR_NAME_SUFFIX "prjdata"
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

#define CSTR(x) (x).utf8().get_data()


#define L_TRACE(...)	lain::Log::GetClientLogger()->trace(__VA_ARGS__)
#define L_INFO(...)	lain::Log::GetClientLogger()->info(__VA_ARGS__)
#define L_WARN(...)	lain::Log::GetClientLogger()->warn(__VA_ARGS__)
#define L_ERROR(...)	lain::Log::GetClientLogger()->error(__VA_ARGS__)

// inline:
// # define L_INLINE __forceinline 
# define L_INLINE __forceinline 

enum LogLevel
{
	LOGTRACE,
	LOGINFO,
	LOGWARN,
	LOGERROR,
};


template <typename ... Types>
void  L_APP_LOG(const LogLevel level = LOGINFO, const Types&... args)
{
	std::ostringstream  ss;
	std::initializer_list <int> { ([&args, &ss] {
		/*if constexpr (std::is_same_v<decltype(args), lain::String>) {
			ss << CSTR(args) << " ";
		}
		else*/
		{
			ss << args << " ";
		}
		}(), 0)...};
	switch (level) {
	case LOGINFO:
		L_INFO(ss.str());
		break;
	case LOGWARN:
		L_WARN(ss.str());
		break;
	case LOGERROR:
		L_ERROR(ss.str());
		break;
	case LOGTRACE:
		L_TRACE(ss.str());
		break;
	default:
		L_INFO(ss.str());
	}
}

template <typename ... Types>
void  L_CORE_LOG(const LogLevel level = LOGINFO, const Types&... args)
{
	std::ostringstream  ss;
	std::initializer_list <int> { ([&args, &ss] {
			/*if constexpr (std::is_same_v<decltype(args), lain::String>) {
				ss << CSTR(args) << " ";
			}
			else */
			{
				ss << args << " ";
			} 
		}(), 0)...};
	switch (level) {
	case LOGINFO:
		L_CORE_INFO(ss.str());
		break;
	case LOGWARN:
		L_CORE_WARN(ss.str());
		break;
	case LOGERROR:
		L_CORE_ERROR(ss.str());
		break;
	case LOGTRACE:
		L_CORE_TRACE(ss.str());
		break;
	default:
		L_CORE_INFO(ss.str());
	}
}

#define L_PRINT(...)	L_APP_LOG(LOGINFO,__FUNCTION__,"in line:",__LINE__, __VA_ARGS__);
#define L_PERROR(...)	L_APP_LOG(LOGERROR,__FUNCTION__,"in line:",__LINE__, __VA_ARGS__);
#define L_PWARNING(...)	L_APP_LOG(LOGWARN,__FUNCTION__,"in line:",__LINE__, __VA_ARGS__);
#define L_CORE_PRINT(...)	L_CORE_LOG(LOGINFO,__FUNCTION__,"in line:",__LINE__, __VA_ARGS__);


# define L_JSON(x) L_PRINT("json of ",#x,lain::Serializer::write(x).dump());
# define L_CORE_JSON(x) L_CORE_PRINT("json of " + std::string(#x) + " " + (lain::Serializer::write(x).dump()));

# define L_NAME(x) #x
#define __CONCAT(a, b) a##b
#define CONCAT2(a, b) __CONCAT(a, b)
#define CONCAT3(a,b,c) CONCAT2(CONCAT2(a,b),c)


//#define refcount(type, x) (*(reinterpret_cast<s_ui32*> (const_cast<type*>(x.ptr())) - 2)).get()
# define refcount(x) x._refcount()
void _global_lock();
void _global_unlock();

typedef uint64_t ui64;
typedef uint32_t ui32;
typedef uint16_t ui16;
typedef uint8_t ui8;

typedef int32_t  i32;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef int32_t  i32;
// godot's devbranch mark;
# define DEV_ENABLED
#endif // __BASE__

