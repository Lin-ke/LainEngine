--- xmakedeps.lua
prjprefix = "$(projectdir)/engine/source"
vdrprefix = "$(projectdir)/engine/thirdparty"
VULKAN_SDK = os.getenv("VULKAN_SDK")
prjbin = "$(projectdir)/bin"
IncludeDir = {}
IncludeDir["spdlog"]    =   (vdrprefix .. "/spdlog/include")
IncludeDir["glfw"]      =   (vdrprefix .. "/glfw/include")
IncludeDir["imgui"]     =   (vdrprefix .. "/imgui")
IncludeDir["llvm"]      =   (vdrprefix .. "/LLVM/include")
IncludeDir["mustache"]  =   (vdrprefix .. "/mustache")
IncludeDir["json11"]  =   (vdrprefix .. "/json11")
IncludeDir["mbedtls"] = (vdrprefix .. "/mbedtls/include")
IncludeDir["stb_image"]  =   (vdrprefix .. "/stb_image")
IncludeDir["spirv_reflect"]  =   (vdrprefix .. "/spirv-reflect")
IncludeDir["vma"]  =   (vdrprefix .. "/vma/include")
IncludeDir["tinyobj"]  =   (vdrprefix .. "/tinyobjloader")
IncludeDir["VulkanSDK"] = (VULKAN_SDK .. "/Include")

LibraryDir = {}

LibraryDir["VulkanSDK"] = VULKAN_SDK .. "/Lib"

Library = {}
-- Vulkan
Library["Vulkan"] = LibraryDir.VulkanSDK .. "/vulkan-1.lib"
Library["VulkanUtils"] = LibraryDir.VulkanSDK .. "/VkLayer_utils.lib"
Library["volk"] = LibraryDir.VulkanSDK .. "/volk.lib"

Library["ShaderC_Debug"] = LibraryDir.VulkanSDK .. "/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = LibraryDir.VulkanSDK .. "/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = LibraryDir.VulkanSDK .. "/spirv-cross-glsld.lib"

Library["SPIRV_Tools_Debug"] = LibraryDir.VulkanSDK .. "/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = LibraryDir.VulkanSDK .. "/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = LibraryDir.VulkanSDK .. "/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = LibraryDir.VulkanSDK .. "/spirv-cross-glsl.lib"
Library["SPIRV_Tools_Release"] = LibraryDir.VulkanSDK .. "/SPIRV-Tools.lib"

Library["llvm_x64_lib"] =        (vdrprefix .. "/LLVM/lib/x64/libclang.lib")
Library["llvm_x64_dll"] =        (vdrprefix .. "/LLVM/bin/x64/libclang.dll")
Library["llvm_linux"] =      (vdrprefix .. "/LLVM/bin/x64/libclang.so.12")

function GetLibrary()
    return Library
end
function GetIncludeDir()
    return IncludeDir
end