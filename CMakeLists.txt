
set(eapp_bin co-server-uintr)
file(GLOB eapp_src eapp/*.c)
set(host_bin ${eapp_bin}-runner)
file(GLOB host_src "httpd/src/*.cpp")
set(package_name ${eapp_bin}.ke)
set(package_script "./${eapp_bin}-runner ${eapp_bin} eyrie-rt")

if(RISCV32)
  set(eyrie_plugins "rv32 freemem")
else()
  set(eyrie_plugins "freemem")
endif()

# eapp
string(REGEX REPLACE "-Werror" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

# deny gp register usage
# https://www.cnblogs.com/wahahahehehe/p/15140813.html
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--no-relax")

add_executable(${eapp_bin} ${eapp_src})
target_link_libraries(${eapp_bin} "-nostdlib -static" ${KEYSTONE_LIB_UTILS} ${KEYSTONE_LIB_UMMANAGE_RING_BUFFER}  ${KEYSTONE_LIB_UINTR} ${KEYSTONE_LIB_EAPP} ${KEYSTONE_LIB_EDGE})
# target_link_libraries(${eapp_bin} "-static" ${KEYSTONE_LIB_EAPP} ${KEYSTONE_LIB_EDGE})


target_include_directories(${eapp_bin}
  PUBLIC ${KEYSTONE_SDK_DIR}/include/app
  PUBLIC ${KEYSTONE_SDK_DIR}/include/edge
  PUBLIC ${KEYSTONE_SDK_DIR}/include/uintr
  PUBLIC ${KEYSTONE_SDK_DIR}/include/utils
  PUBLIC ${KEYSTONE_SDK_DIR}/include/ring-buffer-ummanage)


# host

set(CMAKE_SYSTEM_PROCESSOR "riscv64")
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_VERSION 5.19)

add_definitions(-DLIBURINGCXX_KERNEL_VERSION_MAJOR=5 -DLIBURINGCXX_KERNEL_VERSION_MINOR=19)


# add include
include_directories("httpd/include")


add_executable(${host_bin} ${host_src})


target_link_libraries(${host_bin} "-static"  ${KEYSTONE_LIB_UMMANAGE_RING_BUFFER} 
  ${KEYSTONE_LIB_UTILS} ${KEYSTONE_LIB_UINTR} 
  ${KEYSTONE_LIB_HOST} ${KEYSTONE_LIB_EDGE} 
  /opt/co_context-riscv64/lib/libco_context.a
  "-s" atomic)

set_target_properties(${host_bin}
  PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
)
target_include_directories(${host_bin}
  PUBLIC ${KEYSTONE_SDK_DIR}/include/host
  PUBLIC ${KEYSTONE_SDK_DIR}/include/edge
  PUBLIC ${KEYSTONE_SDK_DIR}/include/uintr
  PUBLIC ${KEYSTONE_SDK_DIR}/include/utils
  PUBLIC ${KEYSTONE_SDK_DIR}/include/ring-buffer-ummanage
  PUBLIC /opt/co_context-riscv64/include
  )


# add target for Eyrie runtime (see keystone.cmake)

set(eyrie_files_to_copy .options_log eyrie-rt)
add_eyrie_runtime(${eapp_bin}-eyrie
  ${eyrie_plugins}
  ${eyrie_files_to_copy})

# add target for packaging (see keystone.cmake)

add_keystone_package(${eapp_bin}-package
  ${package_name}
  ${package_script}
  ${eyrie_files_to_copy} ${eapp_bin} ${host_bin})

add_dependencies(${eapp_bin}-package ${eapp_bin}-eyrie)

# add package to the top-level target
add_dependencies(examples ${eapp_bin}-package)

