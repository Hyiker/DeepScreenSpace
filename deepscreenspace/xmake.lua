add_rules("mode.debug", "mode.release")

target("DeepScreenSpace")
    set_kind("binary")
    add_files("src/*.cpp")
    set_languages("c11", "cxx20")
    
    add_deps("loo")

    add_defines("_CRT_SECURE_NO_WARNINGS")
    set_policy("build.warning", true)
    set_warnings("all", "extra")

    -- solve msvc unfriendly to unicode and utf8
    add_defines( "UNICODE", "_UNICODE")
    add_cxflags("/execution-charset:utf-8", "/source-charset:utf-8", {tools = {"clang_cl", "cl"}})

