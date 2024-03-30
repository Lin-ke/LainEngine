-- contians different compile methods

project "ALL_BUILD"
      targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
      objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
      kind "StaticLib"
      language "C++"
      cppdialect "C++17"
project "ZERO_CHECK"
-- make dependencies
      targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
      objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
      kind "StaticLib"
      language "C++"
      cppdialect "C++17"
      links {
            "spdlog",
            "glfw",
            "json11",
            "glad",
            "imgui",
            "mbedtls"
      }
project "INSTALL"
      kind "StaticLib"
      language "C++"
      cppdialect "C++17"
      targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
      objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")