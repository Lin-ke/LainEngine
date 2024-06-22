-- premake5.lua
include "./setup/premake/premakefile/solution_items.lua"
include "Dependencies.lua"

--- NOTES: TO GAIN ARM64 SUPPORT @TODO
-- 1. Add ARM64 to the architecture
-- 2. Add ARM64 to the outputdir
-- 3. ADD VULKAN SDK ARM64 AND LLVM ARM64
-- 4. visual studio need to have ARM64 开发工具


workspace "Lain"
   configurations { "Debug", "Release","DebugArm64" }
   startproject "Editor"
   flags
	{
		"MultiProcessorCompile"
	}
   filter "configurations:Debug"
      architecture "x86_64"
   filter "configurations:Release"
      architecture "x86_64"
   filter "configurations:DebugArm64"
      architecture "ARM64"
   

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
   include "engine/thirdparty/mbedtls"
   include "engine/thirdparty/spirv-reflect"

group ""

group "Tools"
   include "engine/source/meta_parser/parser"
   include "engine/source/runtime/resource/shader"
group ""

group "Engine"
   include "engine/source/runtime"
   include "engine/source/editor"
   include "engine/source/"
group ""

group "PremakeTargets"
   include "engine"
group ""