
import os
import subprocess, shutil
import platform

# from SetupPython import PythonConfiguration as PythonRequirements

# # Make sure everything we need for the setup is installed
# PythonRequirements.Validate()

# from SetupPremake import PremakeConfiguration as PremakeRequirements
# from SetupVulkan import VulkanConfiguration as VulkanRequirements
if "setup" not in os.listdir("./"):
    os.chdir('./../') # Change from devtools/scripts directory to root
# root
# premakeInstalled = PremakeRequirements.Validate()
# VulkanRequirements.Validate()

# print("\nUpdating submodules...")
# subprocess.call(["git", "submodule", "update", "--init", "--recursive"])
# sending
premakefilePath = ".\setup\premake\premakefile"
thirdPartyPath = "./engine/thirdparty/"
thirdPartyFolders = os.listdir(thirdPartyPath)

for premakeName in os.listdir(premakefilePath):
    name = premakeName.split("-")[0]
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

