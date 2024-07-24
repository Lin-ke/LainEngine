
static_component("glslang", "Renderer")
    add_rules("mode.debug", "mode.release")
    add_files("./**.cpp")
    set_languages("cxx17")
    if is_mode("debug") then
        add_links("glslangd")
    end
    if is_mode("release") then
        add_links("glslang")
    end