


IncludeDir = {}
prjprefix = "%{wks.location}/engine/source"
vdrprefix = "%{wks.location}/engine/thirdparty"
IncludeDir["spdlog"]    =   (vdrprefix .. "/spdlog/include")
IncludeDir["glfw"]      =   (vdrprefix .. "/glfw/include")
IncludeDir["glad"]      =   (vdrprefix .. "/Glad/include")
IncludeDir["imgui"]     =   (vdrprefix .. "/imgui")
IncludeDir["llvm"]      =   (vdrprefix .. "/LLVM/include")
IncludeDir["mustache"]  =   (vdrprefix .. "/mustache")
IncludeDir["json11"]  =   (vdrprefix .. "/json11")



LibraryDir = {}
LibraryDir["llvm_x64_lib"] =        (vdrprefix .. "/LLVM/lib/x64/libclang.lib")
LibraryDir["llvm_x64_dll"] =        (vdrprefix .. "/LLVM/bin/x64/libclang.dll")
LibraryDir["llvm_linux"] =      (vdrprefix .. "/LLVM/bin/x64/libclang.so.12")

Library = {}
-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"