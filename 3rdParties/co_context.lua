package("co_context")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "../co_context"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=OFF")

        if is_arch("riscv64") then
            table.insert(configs, "-DCMAKE_TOOLCHAIN_FILE=" .. path.join(os.scriptdir(), "co_context_riscv.cmake"))
        end

        import("package.tools.cmake").install(package, configs)
    end)
package_end()
