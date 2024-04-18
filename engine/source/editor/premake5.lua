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
		"%{IncludeDir.json11}",
		"%{IncludeDir.spdlog}",
		prjprefix,
		(prjprefix .. "/editor"),
      	(prjprefix .. "/runtime"),
		vdrprefix,
	}

	links
	{
		"spdlog",
		"imgui",
		"LainRuntime"
	}

	filter "system:windows"
		systemversion "latest"
		defines "L_PLATFORM_WINDOWS"

	filter "configurations:Debug"
		defines "L_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "L_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "L_DIST"
		runtime "Release"
		optimize "on"
