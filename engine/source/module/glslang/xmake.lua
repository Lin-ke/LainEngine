target("thirdparty_glslang")
    set_kind("static")
    set_languages("cxx17")
    add_includedirs("$(projectdir)/engine/thirdparty/glslang", {public = true}) -- 如果不这样设置会导致后面的去访问vulkan-sdk的头文件
    -- @todo 完全避免 env:VulkanSDK 的使用
    add_files(
        "$(projectdir)/engine/thirdparty/glslang/glslang/GenericCodeGen/CodeGen.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/GenericCodeGen/Link.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/attribute.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/Constant.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/glslang_tab.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/InfoSink.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/Initialize.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/Intermediate.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/intermOut.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/IntermTraverse.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/iomapper.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/limits.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/linkValidate.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/parseConst.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/ParseContextBase.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/ParseHelper.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/PoolAlloc.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/preprocessor/PpAtom.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/preprocessor/PpContext.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/preprocessor/Pp.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/preprocessor/PpScanner.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/preprocessor/PpTokens.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/propagateNoContraction.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/reflection.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/RemoveTree.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/Scan.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/ShaderLang.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/SpirvIntrinsics.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/SymbolTable.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/MachineIndependent/Versions.cpp",
        "$(projectdir)/engine/thirdparty/glslang/glslang/ResourceLimits/ResourceLimits.cpp",
        "$(projectdir)/engine/thirdparty/glslang/SPIRV/disassemble.cpp",
        "$(projectdir)/engine/thirdparty/glslang/SPIRV/doc.cpp",
        "$(projectdir)/engine/thirdparty/glslang/SPIRV/GlslangToSpv.cpp",
        "$(projectdir)/engine/thirdparty/glslang/SPIRV/InReadableOrder.cpp",
        "$(projectdir)/engine/thirdparty/glslang/SPIRV/Logger.cpp",
        "$(projectdir)/engine/thirdparty/glslang/SPIRV/SpvBuilder.cpp",
        "$(projectdir)/engine/thirdparty/glslang/SPIRV/SpvPostProcess.cpp",
        "$(projectdir)/engine/thirdparty/glslang/SPIRV/SPVRemapper.cpp",
        "$(projectdir)/engine/thirdparty/glslang/SPIRV/SpvTools.cpp")
    if is_plat("windows") then
        add_files("$(projectdir)/engine/thirdparty/glslang/glslang/OSDependent/Windows/ossource.cpp")
    end
target_end()

static_component("lglslang", "Scene")
    add_rules("mode.debug", "mode.release")
    add_files("./**.cpp")

    set_languages("cxx17")
    add_deps("thirdparty_glslang")