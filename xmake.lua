add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
includes("setup/xmake_rules.lua")
set_encodings("utf-8")
engine_version = "0.1.0"
default_unity_batch = 16
set_warnings("all")
set_policy("build.ccache", false)
set_policy("check.auto_ignore_flags", false)
set_policy("build.warning", true)

add_rules("mode.debug", "mode.release")

add_requires( "assimp",  "zstd","glfw","imgui",  "zlib", "spdlog","tinyobjloader", "vulkan-headers")
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
target("smol-v")
    set_kind("static")
    add_files("engine/thirdparty/smol-v/**.cpp")
    add_headerfiles("engine/thirdparty/smol-v/**.h")
    add_includedirs("engine/thirdparty/smol-v", {public = true})
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
    add_deps(("Parser"))
    after_build(
       function(target)
        import("core.base.process")
        import ("setup.precompile_xmake",{alias = "precompile"})
        
        precompile()
        proc = process.open((os.projectdir()) .. "\\engine\\bin\\Parser.exe" .. " " .. precompile.get_parser_order())
        if proc then
            proc:wait()
            proc:close()
        end
    end
    )

target("GenShader")
    set_kind("phony")
    before_build(
        function (target) 
            import ("glsl_builders", {alias = "gbd"})
            import("core.base.process")
            print("--Gen ShaderFiles--")
            local err_name = "./setup/shader_generate.log" 
            os.rm(err_name)
            glsl_files = os.files("$(projectdir)/**.glsl")
            -- 去掉后缀为inc
            for i=#glsl_files, 1, -1 do
                    if string.find(glsl_files[i], ".inc") then
                        table.remove(glsl_files, i)
                    end
            end
            err_file = io.open(err_name, "w")
            try{
                function ()
                    print(glsl_files)
                    gbd.build_rd_headers({}, glsl_files, {})   
                end,
                catch {
                    function (v) 
                        print("error:", v)
                        
                        err_file:write(v)
                    end
                },
                finally{
                    function (ok, errors)
                        err_file:write(v)
                    end
                }
            }
            
            err_file:close()
            -- lock:unlock()
        end
    )
target("CompileShader")
    set_kind("phony")
    before_build(
        function (target) 
            import("core.base.process")
            print("--Compile ShaderFiles--")
            local err_name = "./setup/shader_compile_error.log" 
            os.rm(err_name)
            vert = os.files("$(projectdir)/**.vert")
            frag = os.files("$(projectdir)/**.frag")
            file = io.open(err_name, "w")
            files = {}
            files["vert"] = vert
            files["frag"] = frag
            for k, v1 in pairs(files) do
                file:write("Compile " .. k .. "\n")
                print("Compile " .. k)
                for _, v in ipairs(v1) do
                try {
                    function ()
                        -- print("glslangValidator.exe -o " .. v ..".spv -V ".. v)
                        -- os.run("glslangValidator.exe -o " .. v ..".spv -V ".. v)
                    end,
                    catch {
                        function (v) 
                            file:write(v)
                        end
                    },
                    finally{
                        function (ok, errors)
                            
                        end
                    }

                }
                end
            end
            
            file:close()
            -- lock:unlock()
        end
    )
target("Core")
    add_defines("DEBUG_METHODS_ENABLED")
    set_kind("static")
    set_languages("cxx17", "c99")
    add_headerfiles("engine/source/runtime/**.h") --- 
    -- sourcve 
    add_files("engine/source/runtime/core/**.cpp")
    add_packages(
        "assimp","glfw", "imgui", "zstd",  "zlib", "spdlog","tinyobjloader"
    )
    -- include path
    add_includedirs("engine/source/runtime")
    add_includedirs("engine/source")
    
    -- mbedtls
    add_deps("mbedtls")
    -- stb_img
    -- json 11
    add_includedirs("engine/thirdparty/stb_image")
    -- implementation in "image_loader_stb.cpp"
    add_headerfiles("engine/thirdparty/json11/json11.hpp")
    add_files("engine/thirdparty/json11/json11.cpp")
    add_includedirs("engine/thirdparty/json11")
    -- fastlz
    add_includedirs("engine/thirdparty", {public = true})
    add_includedirs("engine/thirdparty/misc")
    add_files("engine/thirdparty/misc/*.c")
    add_headerfiles(("engine/thirdparty/misc/*.h"))
    add_includedirs("engine/thirdparty/volk", {public = true})
    add_includedirs("engine/thirdparty/vma", {public = true})
     -- spirv-reflect 
    add_includedirs("engine/thirdparty/spirv-reflect")
    
    add_includedirs("$(env VULKAN_SDK)/Include", {public = true})
    if is_plat("windows") then
        add_defines("L_PLATFORM_WINDOWS", {public = true})
    end

    if is_config("mode", "debug") then
        add_defines("L_DEBUG", "DEBUG_ENABLED", {public = true})
    end
    if is_config("mode", "release") then
        add_defines("L_RELEASE", {public = true })
    end
--- functions (servers)
static_component("Renderer", "Core") 
    add_rules("mode.debug", "mode.release")
    add_deps("Spirv-Reflect", "mbedtls", "smol-v")
    set_languages("cxx17")
    add_files("engine/source/runtime/function/render/**.cpp")

    add_includedirs("engine/thirdparty/volk", {public = true})
    add_includedirs("engine/thirdparty/vma", {public = true})
     -- spirv-reflect 
    add_includedirs("engine/thirdparty/spirv-reflect")
    
    add_includedirs("$(env VULKAN_SDK)/Include", {public = true})
    add_linkdirs("$(env VULKAN_SDK)/Lib", {public = true})
    add_defines(
    "_CRT_SECURE_NO_WARNINGS",
     "PRINT_NATIVE_COMMANDS"
    )
    add_defines("VULKAN_ENABLED", {public = true})
    -- 
static_component("Shader", "Renderer")
    add_rules("mode.debug", "mode.release")
    set_languages("cxx17")
    add_files("engine/source/runtime/function/shader/**.cpp")
    
static_component("Display", "Renderer")
    set_languages("cxx17")
    add_files("engine/source/runtime/function/display/**.cpp")

-- resources (scenes)
static_component("Scene", "Renderer")
    set_languages("cxx17")
    add_files("engine/source/runtime/scene/**.cpp")

-- editors
static_component("Editor", "Scene")
    set_languages("cxx17")
    add_files("engine/source/editor/**.cpp")

-- modules
includes("engine/source/module/**/xmake.lua")



target("main")
    set_languages("cxx17")
    set_kind("static")
    add_files("engine/source/runtime/main/**.cpp")
    add_files("engine/source/module/register_module_types.cpp")

    add_includedirs("engine/source/runtime", {public = true})
    add_deps("Core")
    -- functions
    add_deps("Renderer", "Display", "Scene", "Shader", "lglslang") -- @todo 给lglslang 这一步以后做成自动的 add_deps �? Module中的 xmake 项目
    -- editor
    add_deps("Editor")
    -- modules?

target("lain-windows")
    set_default(true)
    set_languages("cxx17")
    set_kind("binary")
    add_files("engine/source/runtime/platform/**_windows.cpp")
    add_files("engine/source/runtime/test/**.cpp")
    add_deps("main")
    add_links(
        "BCrypt",
        "Winmm"
    )
