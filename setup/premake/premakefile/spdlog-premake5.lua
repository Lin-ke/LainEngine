project "spdlog"
    kind "StaticLib"
    language "C"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    includedirs {
        "include"
    }

    files {
        "src/**.cpp",
        "include/**.h"
    }
    defines "SPDLOG_COMPILED_LIB"
