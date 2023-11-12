set(host_bin co-server-baseline)
file(GLOB host_src "httpd/src/*.cpp")

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


# host

# add include
include_directories("httpd/include", "co_context/include")
add_executable(${host_bin} ${host_src})


# 添加co_context库
add_subdirectory(./co_context)


target_link_libraries(${host_bin} "-static"  ${KEYSTONE_LIB_UMMANAGE_RING_BUFFER} 
  ${KEYSTONE_LIB_UTILS} ${KEYSTONE_LIB_UINTR} 
  ${KEYSTONE_LIB_HOST} ${KEYSTONE_LIB_EDGE} 
  co_context)

set_target_properties(${host_bin}
  PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
)
target_include_directories(${host_bin}
  PUBLIC ${KEYSTONE_SDK_DIR}/include/host
  PUBLIC ${KEYSTONE_SDK_DIR}/include/edge
  PUBLIC ${KEYSTONE_SDK_DIR}/include/uintr
  PUBLIC ${KEYSTONE_SDK_DIR}/include/utils
  PUBLIC ${KEYSTONE_SDK_DIR}/include/ring-buffer-ummanage)


# add package to the top-level target
add_dependencies(examples ${host_bin})
