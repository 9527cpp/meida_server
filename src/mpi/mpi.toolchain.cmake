# mpi 子目录交叉编译工具链模板
#
# 使用方法：
#   mkdir build && cd build
#   cmake ../mpi \
#       -DCMAKE_TOOLCHAIN_FILE=mpi.toolchain.cmake \
#       -DTOOLCHAIN_PREFIX=aarch64-openwrt-linux- \
#       -DTOOLCHAIN_DIR=/path/to/toolchain
#
# 或者直接指定编译器：
#   cmake ../mpi \
#       -DCMAKE_C_COMPILER=aarch64-openwrt-linux-gcc \
#       -DCMAKE_CXX_COMPILER=aarch64-openwrt-linux-g++ \
#       -DCMAKE_SYSROOT=/path/to/sysroot
#
# 工具链前缀（aarch64/hisi 等平台）
set(TOOLCHAIN_PREFIX aarch64-openwrt-linux- CACHE STRING "Cross-compiler prefix")
# 工具链根目录（包含 bin/lib）
set(TOOLCHAIN_DIR "" CACHE PATH "Toolchain root directory")

# 设置目标操作系统
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# 查找交叉编译工具
if(TOOLCHAIN_DIR)
    set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR})
    set(CMAKE_PREFIX_PATH ${TOOLCHAIN_DIR})
endif()

# C 编译器
if(TOOLCHAIN_PREFIX AND TOOLCHAIN_DIR)
    set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/${TOOLCHAIN_PREFIX}gcc)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/${TOOLCHAIN_PREFIX}g++)
    set(CMAKE_AR ${TOOLCHAIN_DIR}/bin/${TOOLCHAIN_PREFIX}ar)
    set(CMAKE_RANLIB ${TOOLCHAIN_DIR}/bin/${TOOLCHAIN_PREFIX}ranlib)
    set(CMAKE_LINKER ${TOOLCHAIN_DIR}/bin/${TOOLCHAIN_PREFIX}ld)
elseif(TOOLCHAIN_PREFIX)
    find_program(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
    find_program(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
    find_program(CMAKE_AR ${TOOLCHAIN_PREFIX}ar)
    find_program(CMAKE_RANLIB ${TOOLCHAIN_PREFIX}ranlib)
endif()

# Sysroot（用于查找库和头文件）
if(TOOLCHAIN_DIR)
    set(CMAKE_SYSROOT ${TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}sysroot CACHE PATH "Sysroot path")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${CMAKE_SYSROOT}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${CMAKE_SYSROOT}")
endif()

# 搜索模式：在工具链目录而非主机目录查找
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
