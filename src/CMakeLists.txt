cmake_minimum_required(VERSION 3.10)

# 交叉编译支持
# 用法:
#   cmake -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -B build
# 或直接指定:
#   cmake -DCMAKE_C_COMPILER=aarch64-openwrt-linux-gcc \
#         -DCMAKE_CXX_COMPILER=aarch64-openwrt-linux-g++ \
#         -B build
#
# 工具链文件示例 (toolchain.cmake):
#   set(CMAKE_SYSTEM_NAME Linux)
#   set(CMAKE_C_COMPILER /path/to/toolchain/bin/aarch64-openwrt-linux-gcc)
#   set(CMAKE_CXX_COMPILER /path/to/toolchain/bin/aarch64-openwrt-linux-g++)
#   set(CMAKE_FIND_ROOT_PATH /path/to/sysroot)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -fPIC -std=c++11")

# 交叉编译时指定 sysroot (通过 CMAKE_SYSROOT 或 -DCMAKE_SYSROOT)
if(CMAKE_SYSROOT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${CMAKE_SYSROOT}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${CMAKE_SYSROOT}")
endif()

# 源文件路径
set(MPI_DIR ${CMAKE_SOURCE_DIR}/mpi)
set(CTX_DIR ${CMAKE_SOURCE_DIR}/mpi_ctx)
set(STREAM_DIR ${CMAKE_SOURCE_DIR}/stream)
set(LISTEN_DIR ${CMAKE_SOURCE_DIR}/listeners)
set(HW_DIR ${CMAKE_SOURCE_DIR}/hw_check)
set(MEDIA_DIR ${CMAKE_SOURCE_DIR}/media)
set(LOG_DIR ${CMAKE_SOURCE_DIR}/log)

# C 源文件
set(C_SRCS
    ${MPI_DIR}/mpi_intf.c
    ${MPI_DIR}/mpi_stub.c
    ${CTX_DIR}/mpi_ctx_intf.c
    ${CTX_DIR}/json/cJSON.c
    ${CTX_DIR}/mpi_ctx_json.c
)

# C++ 源文件（stream 模块已独立为 libstream.a，由 add_subdirectory 引入）
set(CXX_SRCS
    ${LISTEN_DIR}/uds_stream.cpp
    ${LISTEN_DIR}/uds_connection_manager.cpp
    ${LISTEN_DIR}/file_stream.cpp
    ${HW_DIR}/hw_check.cpp
    ${HW_DIR}/hdmi_check.cpp
    ${HW_DIR}/usb_check.cpp
    ${MEDIA_DIR}/media_manager.cpp
)

# stream 模块子目录
add_subdirectory(${STREAM_DIR})

# 静态库
add_library(media_server STATIC ${C_SRCS} ${CXX_SRCS})
target_include_directories(media_server PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${MPI_DIR}
    ${CTX_DIR}
    ${STREAM_DIR}
    ${LISTEN_DIR}
    ${HW_DIR}
    ${MEDIA_DIR}
    ${LOG_DIR}
)

# 链接 stream 静态库
target_link_libraries(media_server stream)

# MPI 平台实现（由 mpi/CMakeLists.txt 管理）
add_subdirectory(${MPI_DIR} mpi_build)

# 可执行文件
add_executable(media-server main.cpp)
target_link_libraries(media-server media_server pthread dl)
