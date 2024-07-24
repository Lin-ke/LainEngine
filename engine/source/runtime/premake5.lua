
project "LainRuntime"
      kind "StaticLib"
      language "C++"
      cppdialect "C++17"
      
      targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
      objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
      --  * will match against a single directory, ** will recurse into subdirectories as well
      files 
      {  ("**.h") ,
         ("**.cpp"),
         ("../_generated/*.h"),
         ("../_generated/*.cpp")
      }
      defines{
         "_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
      "VULKAN_ENABLED",
      "VK_NO_PROTOTYPES",
      "_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING",
      }
      includedirs
      {
         prjprefix,
         prjprefix .. "/runtime",
         "%{IncludeDir.spdlog}",
         "%{IncludeDir.glfw}",
         "%{IncludeDir.json11}",
         "%{IncludeDir.mbedtls}",
         "%{IncludeDir.VulkanSDK}",
         "%{IncludeDir.stb_image}",
         "%{IncludeDir.tinyobj}",
         "%{IncludeDir.spirv_reflect}",
         "%{IncludeDir.volk}",
         "%{IncludeDir.vma}",
         "%{IncludeDir.misc}",
         vdrprefix,

      }
      -- the links aligns with the project names in premakes.lua
      links
		{
         "spdlog",
         "%{Library.volk}",
         "glfw",
         "json11",
         "mbedtls",
         "spirv-reflect",
         "PreCompile",
      }
      
      filter "system:windows"
         systemversion "latest"
         defines {"L_PLATFORM_WINDOWS",
      "_WIN32"}
        buildoptions {"/Zi"}
         links
         {
            "%{Library.WinSock}",
            "%{Library.WinMM}",
            "%{Library.WinVersion}",
            "%{Library.BCrypt}",
         }
      


      
      filter "configurations:Debug"
         defines { "L_DEBUG" }
         symbols "On"
         staticruntime "off"
         runtime "Debug"

      
      filter "configurations:Release"
         defines { "L_RELEASE" }
         optimize "On"
         staticruntime "off"
         runtime "Release"
