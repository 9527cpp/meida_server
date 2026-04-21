# mpi 子目录 CMakeLists.txt
# 编译各平台 MPI 实现（rockit / hisi / rkmedia / stub）
#
# ========== 本地构建 ==========
#   mkdir build && cd build
#   cmake .. -DCMAKE_BUILD_TYPE=Release
#   make                          # 编译所有 .so
#   make mpi_rockit              # 仅 rockit
#   make mpi_hisi                # 仅 hisi
#   make mpi_rkmedia             # 仅 rkmedia
#   make mpi_stub                # 仅 stub
#
# ========== 交叉编译 ==========
#   mkdir build && cd build
#   cmake .. \
#       -DCMAKE_TOOLCHAIN_FILE=mpi.toolchain.cmake \
#       -DTOOLCHAIN_PREFIX=aarch64-openwrt-linux- \
#       -DTOOLCHAIN_DIR=/path/to/toolchain \
#       -DCMAKE_BUILD_TYPE=Release
#   make
#
#   或直接指定编译器：
#   cmake .. \
#       -DCMAKE_C_COMPILER=aarch64-openwrt-linux-gcc \
#       -DCMAKE_CXX_COMPILER=aarch64-openwrt-linux-g++ \
#       -DCMAKE_SYSROOT=/path/to/sysroot \
#       -DCMAKE_BUILD_TYPE=Release
#   make
#
# ========== 测试（需 MPI_TEST 指定平台）==========
#   cmake .. -DMPI_TEST=rockit -DCMAKE_BUILD_TYPE=Release
#   make mpi_test
#   ./mpi_test
#
#   平台选项：rockit / hisi / rkmedia / stub
#
# ========== Demo（需 MPI_DEMO 指定平台）==========
#   cmake .. -DMPI_DEMO=stub -DCMAKE_BUILD_TYPE=Release
#   make mpi_demo
#   ./mpi_demo

cmake_minimum_required(VERSION 3.10)

project(MediaServerMPI C)

# 默认 Release 优化
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 交叉编译 sysroot 支持
if(CMAKE_SYSROOT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${CMAKE_SYSROOT}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${CMAKE_SYSROOT}")
endif()

# rockit 平台库是否可用（交叉编译时由外部提供）
option(ENABLE_ROCKIT "Build rockit mpi" ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -fPIC -DDYNAMIC_MPI")

set(MPI_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../log)

# 所有平台 .so 统一输出到 mpi/build/
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ---------- rockit ----------
if(ENABLE_ROCKIT)
add_library(mpi_rockit SHARED
    ${MPI_DIR}/rockit/mpi_rockit.c
)
target_include_directories(mpi_rockit PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${MPI_DIR}
    ${MPI_DIR}/..
    ${LOG_DIR}
    ${MPI_DIR}/rockit/inc
)
if(EXISTS ${MPI_DIR}/rockit/lib/librockit.so)
    target_link_directories(mpi_rockit PUBLIC ${MPI_DIR}/rockit/lib)
    target_link_libraries(mpi_rockit PUBLIC rockit dl)
else()
    # librockit.so 不可用或架构不匹配时，不链接
    message(STATUS "rockit: librockit.so not found or arch mismatch, building without it")
endif()
set_target_properties(mpi_rockit PROPERTIES
    OUTPUT_NAME "libmpi_rockit"
    PREFIX ""
)
endif()

# ---------- hisi ----------
add_library(mpi_hisi SHARED
    ${MPI_DIR}/hisi/mpi_hisi.c
)
target_include_directories(mpi_hisi PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${MPI_DIR}
    ${MPI_DIR}/..
    ${LOG_DIR}
)
target_link_libraries(mpi_hisi PUBLIC dl)
set_target_properties(mpi_hisi PROPERTIES
    OUTPUT_NAME "libmpi_hisi"
    PREFIX ""
)

# ---------- rkmedia ----------
add_library(mpi_rkmedia SHARED
    ${MPI_DIR}/rkmedia/mpi_rkmedia.c
)
target_include_directories(mpi_rkmedia PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${MPI_DIR}
    ${MPI_DIR}/..
    ${LOG_DIR}
)
target_link_libraries(mpi_rkmedia PUBLIC dl)
set_target_properties(mpi_rkmedia PROPERTIES
    OUTPUT_NAME "libmpi_rkmedia"
    PREFIX ""
)

# ---------- stub ----------
add_library(mpi_stub SHARED
    ${MPI_DIR}/mpi_stub.c
)
target_include_directories(mpi_stub PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${MPI_DIR}
    ${MPI_DIR}/..
    ${LOG_DIR}
)
target_link_libraries(mpi_stub PUBLIC dl)
set_target_properties(mpi_stub PROPERTIES
    OUTPUT_NAME "libmpi_stub"
    PREFIX ""
)

# ---------- 测试 ----------
if(MPI_TEST)
    set(TEST_SRC ${MPI_DIR}/test_mpi.c)
    if(MPI_TEST STREQUAL "rockit")
        add_executable(mpi_test ${TEST_SRC})
        target_compile_definitions(mpi_test PRIVATE MPI_PLATFORM_ROCKIT)
    elseif(MPI_TEST STREQUAL "hisi")
        add_executable(mpi_test ${TEST_SRC})
        target_compile_definitions(mpi_test PRIVATE MPI_PLATFORM_HISI)
    elseif(MPI_TEST STREQUAL "rkmedia")
        add_executable(mpi_test ${TEST_SRC})
        target_compile_definitions(mpi_test PRIVATE MPI_PLATFORM_RKMEDIA)
    elseif(MPI_TEST STREQUAL "stub")
        add_executable(mpi_test ${TEST_SRC})
        target_compile_definitions(mpi_test PRIVATE MPI_PLATFORM_STUB)
    else()
        message(FATAL_ERROR "MPI_TEST must be rockit/hisi/rkmedia/stub, got: ${MPI_TEST}")
    endif()
    target_include_directories(mpi_test PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${MPI_DIR}
        ${MPI_DIR}/..
        ${LOG_DIR}
    )
    target_link_libraries(mpi_test PRIVATE dl m pthread)
endif()

# ---------- Demo ----------
if(MPI_DEMO)
    set(DEMO_SRC ${MPI_DIR}/demo_mpi.c)
    if(MPI_DEMO STREQUAL "rockit")
        add_executable(mpi_demo ${DEMO_SRC})
        target_compile_definitions(mpi_demo PRIVATE MPI_PLATFORM_ROCKIT)
    elseif(MPI_DEMO STREQUAL "hisi")
        add_executable(mpi_demo ${DEMO_SRC})
        target_compile_definitions(mpi_demo PRIVATE MPI_PLATFORM_HISI)
    elseif(MPI_DEMO STREQUAL "rkmedia")
        add_executable(mpi_demo ${DEMO_SRC})
        target_compile_definitions(mpi_demo PRIVATE MPI_PLATFORM_RKMEDIA)
    elseif(MPI_DEMO STREQUAL "stub")
        add_executable(mpi_demo ${DEMO_SRC})
        target_compile_definitions(mpi_demo PRIVATE MPI_PLATFORM_STUB)
    else()
        message(FATAL_ERROR "MPI_DEMO must be rockit/hisi/rkmedia/stub, got: ${MPI_DEMO}")
    endif()
    target_include_directories(mpi_demo PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${MPI_DIR}
        ${MPI_DIR}/..
        ${LOG_DIR}
    )
    target_link_libraries(mpi_demo PRIVATE dl m pthread)
endif()
