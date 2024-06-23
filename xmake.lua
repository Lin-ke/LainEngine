add_rules("mode.debug", "mode.release")
add_requires( "vulkansdk","assimp", "vulkan-memory-allocator","volk" ,"glfw","imgui", "mbedtls", "zlib", "spdlog","tinyobjloader", "vulkan-headers")

-- target("glfw")
    -- set_kind("static")
    -- set_languages("c99")
    -- add_deps("volk")
    -- add_includedirs("engine/thirdparty/volk")
    -- add_headerfiles("engine/thirdparty/glfw/include/GLFW/glfw3.h",
	-- 	"engine/thirdparty/glfw/include/GLFW/glfw3native.h",
	-- 	"engine/thirdparty/glfw/src/glfw_config.h")
    -- add_files
	-- (
	-- 	"engine/thirdparty/glfw/src/context.c",
	-- 	"engine/thirdparty/glfw/src/init.c",
	-- 	"engine/thirdparty/glfw/src/input.c",
	-- 	"engine/thirdparty/glfw/src/monitor.c",
	-- 	"engine/thirdparty/glfw/src/null_init.c",
	-- 	"engine/thirdparty/glfw/src/null_joystick.c",
	-- 	"engine/thirdparty/glfw/src/null_monitor.c",
	-- 	"engine/thirdparty/glfw/src/null_window.c",
	-- 	"engine/thirdparty/glfw/src/platform.c",
	-- 	"engine/thirdparty/glfw/src/vulkan.c",
	-- 	"engine/thirdparty/glfw/src/window.c"    )
    -- if is_plat("linux") then
    --     add_files(
    --         "engine/thirdparty/glfw/src/x11_init.c",
    --         "engine/thirdparty/glfw/src/x11_monitor.c",
    --         "engine/thirdparty/glfw/src/x11_window.c",
    --         "engine/thirdparty/glfw/src/xkb_unicode.c",
    --         "engine/thirdparty/glfw/src/posix_module.c",
    --         "engine/thirdparty/glfw/src/posix_time.c",
    --         "engine/thirdparty/glfw/src/posix_thread.c",
    --         "engine/thirdparty/glfw/src/posix_module.c",
    --         "engine/thirdparty/glfw/src/glx_context.c",
    --         "engine/thirdparty/glfw/src/egl_context.c",
    --         "engine/thirdparty/glfw/src/osmesa_context.c",
    --         "engine/thirdparty/glfw/src/linux_joystick.c"
        
    --     )
    --     add_defines("_GLFW_X11")
    -- end
    -- if is_plat("macosx") then
    --     add_files(
    --         "engine/thirdparty/glfw/src/cocoa_init.m",
    --         "engine/thirdparty/glfw/src/cocoa_monitor.m",
    --         "engine/thirdparty/glfw/src/cocoa_window.m",
    --         "engine/thirdparty/glfw/src/cocoa_joystick.m",
    --         "engine/thirdparty/glfw/src/cocoa_time.c",
    --         "engine/thirdparty/glfw/src/nsgl_context.m",
    --         "engine/thirdparty/glfw/src/posix_thread.c"
    --     )
    -- end
    -- if is_plat("windows") then
    --     add_files(
    --         "engine/thirdparty/glfw/src/win32_window.c",
	-- 		"engine/thirdparty/glfw/src/win32_init.c",
	-- 		"engine/thirdparty/glfw/src/win32_joystick.c",
	-- 		"engine/thirdparty/glfw/src/win32_module.c",
	-- 		"engine/thirdparty/glfw/src/win32_monitor.c",
	-- 		"engine/thirdparty/glfw/src/win32_time.c",
	-- 		"engine/thirdparty/glfw/src/win32_thread.c",
	-- 		"engine/thirdparty/glfw/src/wgl_context.c",
	-- 		"engine/thirdparty/glfw/src/egl_context.c",
	-- 		"engine/thirdparty/glfw/src/osmesa_context.c"
    --     )
    --     add_defines("_GLFW_WIN32", "WIN32","GLFW_EXPOSE_NATIVE_WIN32", "_WIN32", "NoRuntimeChecks", {public=true} )

    -- end

target("Spirv-Reflect")
    set_kind("static")
    set_languages("cxx17")
    add_files("engine/thirdparty/spirv-reflect/**.cpp")
    add_files("engine/thirdparty/spirv-reflect/**.c")
    add_headerfiles("engine/thirdparty/spirv-reflect/**.h")
    add_includedirs("engine/thirdparty/spirv-reflect/include")
    target_end()

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
    add_packages("vulkansdk", "assimp","glfw", "imgui", "mbedtls", "zlib", "spdlog","tinyobjloader","vulkan-headers" )
    add_includedirs("engine/source","engine/source/runtime", "engine/source/editor")
    -- stb_img
    -- json 11
    add_includedirs("engine/thirdparty/stb_image")
    add_headerfiles("engine/thirdparty/json11/json11.hpp")
    add_files("engine/thirdparty/json11/json11.cpp")
    add_includedirs("engine/thirdparty/json11")
    -- others
    add_includedirs("engine/thirdparty")
    -- llvm headers
    -- add_includedirs("engine/thirdparty/LLVM/include")
    -- llvm libs
    -- add_linkdirs("engine/thirdparty/LLVM/lib/x64")
    -- add_links("libclang.lib")
    -- spirv-reflect 用自己的
    add_includedirs("engine/thirdparty/spirv-reflect")
    -- 疑似bug：
    add_includedirs("engine/thirdparty/spdlog/include")
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

