# stream 模块 CMake 构建配置

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -fPIC -std=c++11")

# 路径
set(LOG_DIR ${CMAKE_SOURCE_DIR}/log)
set(MPI_CTX_DIR ${CMAKE_SOURCE_DIR}/mpi_ctx)

# 源文件
set(STREAM_SRCS
    stream_base.cpp
    stream_vi.cpp
    stream_vpss.cpp
    stream_venc.cpp
    stream_ai.cpp
    stream_aenc.cpp
    stream_hdmi_video.cpp
    stream_hdmi_audio.cpp
)

# 静态库
add_library(stream STATIC ${STREAM_SRCS})

target_include_directories(stream PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${LOG_DIR}
    ${MPI_CTX_DIR}
)

# ========== 测试配置 ==========
enable_testing()

# 自包含测试框架，无需外部依赖
set(TEST_SRCS test_stream.cpp test_mpi_stub.cpp)
add_executable(test_stream ${TEST_SRCS})
target_link_libraries(test_stream stream pthread)
target_include_directories(test_stream PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${LOG_DIR}
    ${MPI_CTX_DIR}
)
add_test(NAME stream_test COMMAND test_stream)
