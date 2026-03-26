set_project("EPPL")
set_version("1.0.0")

set_languages("c++23")

add_rules("mode.release")

set_policy("build.ccache", true)

add_repositories("xmake-repo https://github.com/xmake-io/xmake-repo.git")
add_repositories("nexcaise-repo https://github.com/nexcaise/xmake-repo.git")

add_requires("miniapi")

target("EPPL")
    set_kind("shared")
    add_linkdirs("api/libs/arm64-v8a")
    add_packages("miniapi")
    add_links("nise", "log")

    add_files("src/*.cpp", "src/**/*.cpp", "api/deps/gamepwnage/src/*.c", "api/deps/gamepwnage/src/**/*.c")

    add_includedirs("include", "api/include", "api/deps/gamepwnage/includes", {public = true})

    add_cxflags("-O2", "-fvisibility=hidden", "-ffunction-sections", "-fdata-sections", "-w")
    add_cflags("-O2", "-fvisibility=hidden", "-ffunction-sections", "-fdata-sections", "-w")
    add_ldflags("-Wl,--gc-sections,--strip-all", "-s")