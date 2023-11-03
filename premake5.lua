-- premake5.lua
include "./setup/premake/premakefile/solution_items.lua"
include "Dependencies.lua"

workspace "Lain"
   configurations { "Debug", "Release" }
   architecture "x86_64"
   startproject "Editor"
   flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
   include "setup/premake"
	include "engine/thirdparty/spdlog"
   include "engine/thirdparty/glfw"
   include "engine/thirdparty/imgui"
group ""

group "Engine"
   include "engine/source/runtime"
   include "engine/source/editor"
group ""