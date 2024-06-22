
project "Parser"
      kind "ConsoleApp"
      language "C++"
      cppdialect "C++17"
      
      targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
      objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
      --  * will match against a single directory, ** will recurse into subdirectories as well
      files 
      {  ("**.h") ,
         ("**.cpp") 
      }
      -- use lower
      includedirs
      {

         "%{prj.location}",
         "%{IncludeDir.llvm}",
         "%{IncludeDir.mustache}",
      }
      defines
      {
         DTIXML_USE_STL
      }

      
      --- @TODO add arm64 support
      filter "system:windows"
         links
         {
            "%{LibraryDir.llvm_x64_lib}",
            "%{LibraryDir.llvm_x64_dll}",
         }
         
         postbuildcommands {
            ("{COPY} %{cfg.targetdir}/%{prj.name}.exe ".. parser_path),
            ("{COPY} %{cfg.targetdir}/%{prj.name}.pdb ".. parser_path),
            ("{COPY} %{LibraryDir.llvm_x64_dll} ".. parser_path), --clang
            
          }

      
      
      filter "configurations:Debug"
         defines { "L_DEBUG" }
         symbols "On"


      
      filter "configurations:Release"
         defines { "L_RELEASE" }
         optimize "On"
   