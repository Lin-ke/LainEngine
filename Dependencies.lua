


IncludeDir = {}
prjprefix = "%{wks.location}/engine/source"
vdrprefix = "%{wks.location}/engine/thirdparty"
IncludeDir["spdlog"]    = (vdrprefix .. "/spdlog/include")
IncludeDir["glfw"]      = (vdrprefix .. "/glfw/include")
IncludeDir["glad"]      = (vdrprefix .. "/Glad/include")
IncludeDir["imgui"]      = (vdrprefix .. "/imgui")

LibraryDir = {}


Library = {}
-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"