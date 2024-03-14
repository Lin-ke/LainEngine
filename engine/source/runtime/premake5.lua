
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
		"GLFW_INCLUDE_NONE"
      }
      includedirs
      {
         prjprefix,
         prjprefix .. "/runtime",
          "%{IncludeDir.spdlog}",
          "%{IncludeDir.glfw}",
          "%{IncludeDir.json11}",
         "%{IncludeDir.VulkanSDK}",

      }
      -- the links aligns with the project names in premakes.lua
      links
		{
         "spdlog",
         "glfw",
         "json11",
         "PreCompile"

		}
      
      filter "system:windows"
         systemversion "latest"
         defines {"L_PLATFORM_WINDOWS"}
         defines
         {
         }

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


      
      filter "configurations:Release"
         defines { "L_RELEASE" }
         optimize "On"
