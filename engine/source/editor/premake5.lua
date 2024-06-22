project "Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"**.h",
		"**.cpp"
	}

	includedirs
	{
		"%{IncludeDir.spdlog}",
         "%{IncludeDir.glfw}",
         "%{IncludeDir.json11}",
         "%{IncludeDir.VulkanSDK}",
		prjprefix,
		(prjprefix .. "/editor"),
      	(prjprefix .. "/runtime"),
		vdrprefix,
	}

	links
	{
		"LainRuntime",
	}

	filter "system:windows"
	buildoptions {"/Zi"}
		systemversion "latest"
		defines {"L_PLATFORM_WINDOWS",
	"_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS"}


	filter "configurations:Debug"
		defines "L_DEBUG"
		symbols "on"
		staticruntime "off"
		runtime "Debug"

	filter "configurations:Release"
		defines "L_RELEASE"
		optimize "on"
		staticruntime "off"
		runtime "Release"
