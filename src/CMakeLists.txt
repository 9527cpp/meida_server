cmake_minimum_required(VERSION 3.10)
project(media-server C CXX)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -fPIC -std=c++11")

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

# C++ 源文件
set(CXX_SRCS
    ${STREAM_DIR}/stream_base.cpp
    ${STREAM_DIR}/stream_vi.cpp
    ${STREAM_DIR}/stream_vpss.cpp
    ${STREAM_DIR}/stream_venc.cpp
    ${STREAM_DIR}/stream_ai.cpp
    ${STREAM_DIR}/stream_aenc.cpp
    ${STREAM_DIR}/stream_hdmi_video.cpp
    ${STREAM_DIR}/stream_hdmi_audio.cpp
    ${LISTEN_DIR}/uds_stream.cpp
    ${LISTEN_DIR}/uds_connection_manager.cpp
    ${LISTEN_DIR}/file_stream.cpp
    ${HW_DIR}/hw_check.cpp
    ${HW_DIR}/hdmi_check.cpp
    ${HW_DIR}/usb_check.cpp
    ${MEDIA_DIR}/media_manager.cpp
)

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

# MPI rockit
add_library(mpi_rockit SHARED ${MPI_DIR}/rockit/mpi_rockit.c)
target_include_directories(mpi_rockit PUBLIC ${CMAKE_SOURCE_DIR} ${MPI_DIR} ${LOG_DIR})
set_target_properties(mpi_rockit PROPERTIES OUTPUT_NAME "libmpi_rockit" PREFIX "")

# 可执行文件
add_executable(media-server main.cpp)
target_link_libraries(media-server media_server pthread dl)
