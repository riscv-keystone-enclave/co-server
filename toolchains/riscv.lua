toolchain("riscv")
    set_kind("standalone")
    set_sdkdir("/opt/riscv-gnu-toolchain")
    set_cross("riscv64-unknown-linux-gnu-")
    add_cflags("--sysroot=/opt/riscv-gnu-toolchain/sysroot")
    add_cxxflags("--sysroot=/opt/riscv-gnu-toolchain/sysroot")
    add_ldflags("--sysroot=/opt/riscv-gnu-toolchain/sysroot")
toolchain_end()