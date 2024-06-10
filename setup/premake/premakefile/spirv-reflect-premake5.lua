project "spirv-reflect"
    kind "StaticLib"
    language "C++"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    includedirs {
        "./"
    }

    files {
        "include/**.h",
        "./spirv_reflect.h",
        "./spirv_reflect.cpp"
    }

    filter "configurations:Debug"
         symbols "On"
         staticruntime "off"
         runtime "Debug"

          
    filter "configurations:Release"
         optimize "On"
         staticruntime "off"
         runtime "Release"