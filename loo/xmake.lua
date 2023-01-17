add_rules("mode.debug", "mode.release")

add_requires("glfw 3.3.8", "glm 0.9.9+8", "glog v0.6.0", "assimp v5.2.5")
add_requires("imgui v1.89", {configs = {glfw_opengl3 = true, use_glad = true}})

target("loo")
    set_kind("object")
    set_languages("c11", "cxx20")
    add_includedirs(".", "ext/headeronly", {public = true})

    add_files("src/*.cpp")
    add_packages("glfw", "glm", "glog", "imgui", {public = true})
    add_packages("assimp")

    -- glad
    if is_plat("macosx") then
        -- macosx offers the latest support until 4.3
        ogl_ver = "43"
    else
        ogl_ver = "46"
    end
    add_defines("OGL_" .. ogl_ver, "_USE_MATH_DEFINES", {public = true})
    add_includedirs(string.format("ext/glad%s/include", ogl_ver), {public = true})
    add_files(string.format("ext/glad%s/src/glad.c", ogl_ver), {public = true})
    on_config(function (target)
        local ogl_ver = "4.6"
        if is_plat("macosx") then
            ogl_ver = "4.6"
        end
        cprintf("${bright green}[INFO] ${clear}glad configure using opengl %s on %s\n", ogl_ver, os.host())
    end)

    add_rules("utils.install.cmake_importfiles")
    add_rules("utils.install.pkgconfig_importfiles")