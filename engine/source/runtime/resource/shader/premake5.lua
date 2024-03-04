
project "CompileShader"
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")    
    kind "Utility"
    postbuildcommands {
        (PREMAKE_PATH .. " --file=\"./compileshader.lua\""),
    }
