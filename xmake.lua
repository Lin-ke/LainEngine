add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
add_rules("mode.debug", "mode.release")
add_requires( "assimp", "vulkan-memory-allocator","volk" ,"glfw","imgui",  "zlib", "spdlog","tinyobjloader", "vulkan-headers")
add_requires("mustache")
target("Spirv-Reflect")
    set_kind("static")
    set_languages("cxx17")
    add_files("engine/thirdparty/spirv-reflect/**.cpp")
    add_files("engine/thirdparty/spirv-reflect/**.c")
    add_headerfiles("engine/thirdparty/spirv-reflect/**.h")
    add_includedirs("engine/thirdparty/spirv-reflect/include")
    target_end()
target("mbedtls")
    set_kind("static")
    add_files("engine/thirdparty/mbedtls/library/*.c")
    add_headerfiles("engine/thirdparty/mbedtls/include/**.h")
    add_includedirs("engine/thirdparty/mbedtls/include", {public = true})
    add_includedirs("engine/thirdparty/zlib", {public = true})
    add_defines("MBEDTLS_ZLIB_SUPPORT")
    target_end()
target("Parser")
    set_kind("binary")
    set_languages("cxx17")
    add_files(("engine/source/meta_parser/parser/**.cpp"))
    add_headerfiles("engine/source/meta_parser/parser/**.h")
    add_includedirs("engine/thirdparty/LLVM/include")
    add_includedirs("engine/source/meta_parser/parser")
    add_packages("mustache")
    if is_plat("windows") then
        if is_arch("x64") then
            add_linkdirs("engine/thirdparty/LLVM/lib/x64")
        end
        if is_arch("arm64") then
            add_linkdirs("engine/thirdparty/LLVM/lib/arm64")
        end
        add_links("libclang.lib")
    end
    after_build(function(target)
        os.cp(target:targetfile(), "$(projectdir)/engine/bin")
        -- 判断dll是否存在
        local clangdll = target:targetdir().."/libclang.dll"
        if not os.exists(clangdll) then
            if is_arch("x64") then
                os.cp("engine/thirdparty/LLVM/bin/x64/libclang.dll", "$(projectdir)/engine/bin")
            end
            if is_arch("arm64") then
                os.cp("engine/thirdparty/LLVM/bin/arm64/libclang.dll", "$(projectdir)/engine/bin")
            end
        end
    end)
    
target_end()

target("MetaParser")
    set_kind("phony")
    add_headerfiles("engine/source/_generated/**.h")
    add_headerfiles("engine/source/_generated/**.ipp")
    add_deps(("Parser"))
    after_build(
       function(target)
        import("core.base.process")
        import ("setup.precompile_xmake",{alias = "precompile"})
        
        precompile()
        proc = process.open((os.projectdir()) .. "/engine/bin/Parser.exe" .. " " .. precompile.get_parser_order())
        if proc then
            proc:wait()
            proc:close()
        end
    end
    )
target("Renderer")
    set_kind("binary")
    set_languages("cxx17", "c99")
    add_deps("Spirv-Reflect")
    add_includedirs("engine/thirdparty/volk")
    add_includedirs("engine/thirdparty/vma")
    add_headerfiles("engine/source/**.h")
    add_files("engine/source/runtime/**.cpp")
    add_files("engine/source/editor/**.cpp")
    add_headerfiles("engine/source/_generated/**.h")
    add_deps("mbedtls")
    add_packages("assimp","glfw", "imgui",  "zlib", "spdlog","tinyobjloader","vulkan-headers" )
    add_includedirs("engine/source","engine/source/runtime", "engine/source/editor")
    -- stb_img
    -- json 11
    add_includedirs("engine/thirdparty/stb_image")
    add_headerfiles("engine/thirdparty/json11/json11.hpp")
    add_files("engine/thirdparty/json11/json11.cpp")
    add_includedirs("engine/thirdparty/json11")
    -- others
    add_includedirs("engine/thirdparty")
    -- spirv-reflect 用自己的
    add_includedirs("engine/thirdparty/spirv-reflect")
    -- vulkan
    add_includedirs("$(env VULKAN_SDK)/Include")
    add_linkdirs("$(env VULKAN_SDK)/Lib")
    add_defines{
    "_CRT_SECURE_NO_WARNINGS",
     "VULKAN_ENABLED",
      "USE_VOLK",
     "_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING",
     }
    
    if is_plat("windows") then
        add_links("Ws2_32.lib")
        add_links("Winmm.lib")
        add_links("Version.lib")
        add_links("Bcrypt.lib")
        add_defines("L_PLATFORM_WINDOWS")
        add_defines("_WIN32")
    end
    if is_mode("debug") then
        add_defines("L_DEBUG")
    end
     target_end()
    

-- target ("Editor")
--     add_includedirs("engine/source","engine/source/runtime", "engine/source/editor")
--     add_files("engine/source/editor/**.cpp")
--     add_deps("LainRuntime")
--     set_kind("binary")

