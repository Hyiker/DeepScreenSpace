add_rules("mode.debug", "mode.release")

add_requires("glfw 3.3.8", "glm 0.9.9+8", "glog v0.6.0", "assimp v5.2.5", "meshoptimizer v0.18")
add_requires("imgui v1.89", {configs = {glfw_opengl3 = true, use_glad = true}})

package("meshoptimizer")
    set_homepage("https://github.com/zeux/meshoptimizer")
    set_description("Mesh optimization library that makes meshes smaller and faster to render")
    set_license("MIT")

    add_urls("https://github.com/zeux/meshoptimizer/archive/refs/tags/$(version).tar.gz",
             "https://github.com/zeux/meshoptimizer.git")
    add_versions("v0.18", "e62c41e47cfdaf014c1e2a98dfeb571dd60c271f")

    add_configs("shared", {description = "Build shared library.", default = true, type = "boolean"})
    add_deps("cmake")
 
    on_install("windows|x86", "windows|x64", "macosx", "linux", function (package)
        local configs = {"-DMESHOPT_BUILD_DEMO=OFF", "-DMESHOPT_BUILD_GLTFPACK=OFF"}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DMESHOPT_BUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        
        if package:is_plat("windows") then
            table.insert(configs, "-DUSE_STATIC_RUNTIME=" .. (package:config("vs_runtime"):startswith("MT") and "ON" or "OFF"))
        end
        import("package.tools.cmake").install(package, configs)
        print(package:installdir())
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            void test(int argc, char** argv) {
                meshopt_encodeVertexVersion(0);
            }]]}, {configs = {languages = "c++11"}, includes = "meshoptimizer.h"}))
    end)


target("loo")
    set_kind("object")
    set_languages("c11", "cxx20")
    add_includedirs(".", "ext/headeronly", {public = true})

    add_files("src/*.cpp")
    add_packages("glfw", "glm", "glog", "imgui", "assimp", "meshoptimizer", {public = true})

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


    on_config(function (target) 
        local function find_constants(filename)
            file = io.open(filename, "r")
            const_table = {}
            for line in file:lines() do
                const_name, const_value = string.match(line, "^constexpr int ([0-9a-zA-z_]*) *= *([0-9]*);$")
                if const_name ~= nil then 
                    const_table[const_name] = const_value
                end
            end
            file:close()
            return const_table
        end
        local function replace_constants(f_in, f_out, c_table)
            file_in = io.open(f_in, "r")
            file_out = io.open(f_out, "w+")
            file_out:write("// ".. path.filename(f_out) .. " generated by loo/xmake.lua, from ".. path.filename(f_in) .."\n")
            for line in file_in:lines() do 
                for const_name in string.gmatch(line, "%%([0-9a-zA-z_]*)%%") do
                    if const_name ~= nil then 
                        line = line:gsub("%%"..const_name.."%%", const_table[const_name])
                    end
                end
                file_out:write(line, '\n')
            end
            file_in:close()
            file_out:close()
        end
        -- create glsl constants files
        c_table = find_constants("$(projectdir)/loo/loo/loo.hpp")
        replace_constants("$(projectdir)/loo/templates/simple_material.glsl", "$(projectdir)/loo/shader/loo/simple_material.glsl", c_table)
        replace_constants("$(projectdir)/loo/templates/lights.glsl", "$(projectdir)/loo/shader/loo/lights.glsl", c_table)
    end)

    add_rules("utils.install.cmake_importfiles")
    add_rules("utils.install.pkgconfig_importfiles")