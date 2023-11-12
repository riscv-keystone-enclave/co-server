includes("./toolchains/*.lua")
includes("./3rdParties/*.lua")

if is_arch("riscv64") then
    add_toolchains("riscv")
end

add_requires("co_context")


includes("httpd")
