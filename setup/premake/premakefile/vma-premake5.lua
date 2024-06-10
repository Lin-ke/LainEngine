project "vma"
    kind "StaticLib"
    language "C++"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    includedirs {
        "./",
        "./include/",
        "%{IncludeDir.VulkanSDK}"
    }

    

    files {
        "./**.h",
        "./**.c",
        "./**.cpp",

    }
