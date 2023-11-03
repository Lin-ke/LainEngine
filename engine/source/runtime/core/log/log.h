#pragma once
#include <base.h>
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)
#include <memory>

namespace Lain {

    class Log {
    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;

    public:
        static void Init();

        static std::shared_ptr<spdlog::logger>& GetCoreLogger();
        static std::shared_ptr<spdlog::logger>& GetClientLogger();
    };
}

// Log Macros
#define L_CORE_TRACE(...)	::Lain::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define L_CORE_INFO(...)	::Lain::Log::GetCoreLogger()->info(__VA_ARGS__)
#define L_CORE_WARN(...)	::Lain::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define L_CORE_ERROR(...)	::Lain::Log::GetCoreLogger()->error(__VA_ARGS__)
#define L_CORE_FATAL(...)	::Lain::Log::GetCoreLogger()->fatal(__VA_ARGS__)

#define L_TRACE(...)	::Lain::Log::GetClientLogger()->trace(__VA_ARGS__)
#define L_INFO(...)	::Lain::Log::GetClientLogger()->info(__VA_ARGS__)
#define L_WARN(...)	::Lain::Log::GetClientLogger()->warn(__VA_ARGS__)
#define L_ERROR(...)	::Lain::Log::GetClientLogger()->error(__VA_ARGS__)
#define L_FATAL(...)	::Lain::Log::GetClientLogger()->fatal(__VA_ARGS__)