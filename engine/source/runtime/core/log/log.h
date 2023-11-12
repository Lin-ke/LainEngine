#pragma once
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)
#include <memory>

namespace lain {

    class Log {
    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;

    public:
        static void Init();

        static std::shared_ptr<spdlog::logger>& GetCoreLogger();
        static std::shared_ptr<spdlog::logger>& GetClientLogger();
        /*void Logf(const char* p_format, ...);*/
        enum ErrorType {
            ERR_ERROR,
            ERR_WARNING,
            ERR_SCRIPT,
            ERR_SHADER
        };
        
    };
    
}