
import os
import subprocess, shutil
import platform
# from SetupPython import PythonConfiguration as PythonRequirements

# # Make sure everything we need for the setup is installed
# PythonRequirements.Validate()
if "setup" not in os.listdir("./"):
    os.chdir('./../') 
# from SetupPremake import PremakeConfiguration as PremakeRequirements

from SetupVulkan import VulkanConfiguration as VulkanRequirements
# Change from devtools/scripts directory to root
# root
# premakeInstalled = PremakeRequirements.Validate()
try:
    if "VULKAN_SDK" not in os.environ.keys():
        VulkanRequirements.Validate()
except Exception as e:
    print(e)
    print("Something wrong, you need to manually install vulkan")

# print("\nUpdating submodules...")
# subprocess.call(["git", "submodule", "update", "--init", "--recursive"])
# sending
premakefilePath = ".\setup\premake\premakefile"
thirdPartyPath = "./engine/thirdparty/"

try:
    if(not os.path.exists(os.path.join(thirdPartyPath,"volk"))):
        #makedir
        os.makedirs(os.path.join(thirdPartyPath,"volk"))
    vulkanSDK = os.environ.get("VULKAN_SDK")
    shutil.copyfile(f"{vulkanSDK}/include/Volk/volk.c",os.path.join(os.path.join(thirdPartyPath,"volk"), "volk.c"))
    shutil.copyfile(f"{vulkanSDK}/include/Volk/volk.h",os.path.join(os.path.join(thirdPartyPath, "volk"), "volk.h"))
except Exception as e:
    print("error when copying {}".format("volk"))
    print(e)
try:
    if(not os.path.exists(os.path.join(thirdPartyPath,"vma"))):
        #makedir
        os.makedirs(os.path.join(thirdPartyPath,"vma"))
    vulkanSDK = os.environ.get("VULKAN_SDK")
    shutil.copyfile(f"{vulkanSDK}/include/vma/vk_mem_alloc.h", os.path.join(os.path.join(thirdPartyPath,"vma"), "vk_mem_alloc.h"))
except Exception as e:
    print("error when copying {}".format("vma"))
    print(e)
thirdPartyFolders = os.listdir(thirdPartyPath)
for premakeName in os.listdir(premakefilePath):
    name = premakeName[:-len("-premake5.lua")]
    print(name)
    if name in thirdPartyFolders:
        # EXAMPLE: name:glad, path:glad/premake5.lua
        try:
            new_path = shutil.copyfile(os.path.join(premakefilePath, premakeName),os.path.join(os.path.join(thirdPartyPath,name), "premake5.lua"))

        except Exception as e:
            print("error when copying {}".format(premakeName))
            print(e)
            continue



premakeInstalled = True

if (premakeInstalled):
    if platform.system() == "Windows":
        print("\nRunning premake...")
        subprocess.call([os.path.abspath("./setup/Win-GenProjects.bat"), "nopause"])

    print("\nSetup completed!")
else:
    print("Lain requires Premake to generate project files.")

