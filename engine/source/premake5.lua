require "precompile"
precompile()

p = path.getabsolute("../..")

if (os.ishost("windows")) then
    parser_obj = (parser_path .. "/Parser.exe")
    precompile_params_path = json_path
    parser_input = (p .. "/engine/source/parser_header.h")
    sysinclude = "*"
 end
if (os.ishost("linux")) then
    parser_obj = (parser_path .. "Parser")
    precompile_params_path = json_path
    parser_input = (p .. "/engine/source/parser_header.h")
    sysinclude = nil
end
 -- generate precompile.json
 
project "PreCompile"
        targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
        objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
      include "precompile.lua"
      kind "Utility"
        files 
        {("**.h"), ("**.cpp")}
      links {
          "Parser",
       }

       -- may call some python to generate .json
       postbuildcommands {
          (parser_obj .. " ".. precompile_params_path .. " " .. parser_input .." " ..  (p .. "/engine/source ")..  sysinclude .." " .. "Lain " ..  "0"),
       }