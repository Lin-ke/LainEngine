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
parser_path = "%{wks.location}engine/bin"
if os.isdir("engine/bin") == false then
   os.mkdir("engine/bin")
end
group "Dependencies"
   include "setup/premake"
   include "engine/thirdparty/spdlog"
   include "engine/thirdparty/glfw"
   include "engine/thirdparty/imgui"
   include "engine/thirdparty/json11"
group ""

group "Tools"
   include "engine/source/meta_parser/parser"
group ""

group "Engine"
   include "engine/source/runtime"
   include "engine/source/editor"
   include "engine/source/"
group ""

group "PremakeTargets"
   include "engine"
group ""