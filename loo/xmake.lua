add_rules("mode.debug", "mode.release")

add_requires("glfw 3.3.8", "glm 0.9.9+8", "glog v0.6.0")
add_requires("imgui v1.89", {configs = {glfw_opengl3 = true}})

target("loo")
    set_kind("shared")
    set_languages("c11", "cxx17")
    add_includedirs(".", "ext/glad/include", "ext/headeronly", {public = true})
    add_files("src/*.cpp")
    add_packages("glfw", "glm", "imgui", "glog")

    -- glad
    add_files("ext/glad/src/glad.c")

    add_rules("utils.install.cmake_importfiles")
    add_rules("utils.install.pkgconfig_importfiles")