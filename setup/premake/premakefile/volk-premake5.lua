project "volk"
    kind "StaticLib"
    language "C++"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    includedirs {
        "./",
        "%{IncludeDir.VulkanSDK}"
    }

    defines {
        "VK_KHR_win32_surface",
        "VK_USE_PLATFORM_WIN32_KHR",
        "VK_NO_PROTOTYPES",
    }

    files {
        "./volk.h",
        "./volk.c"
    }
