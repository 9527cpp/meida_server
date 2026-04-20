# media-server 框架静态库
# 可在源码树内直接 make，或由 OpenWrt Build/Compile 调用
#
# 交叉编译用法:
#   make CROSS_COMPILE=aarch64-openwrt-linux- \
#        TOOLCHAIN_DIR=/path/to/toolchain \
#        TARGET_ARCH=aarch64 -f Makefile

# 交叉编译工具链 (通过 CROSS_COMPILE 和 TOOLCHAIN_DIR 指定)
CROSS_COMPILE ?=
TOOLCHAIN_DIR ?=

# 如果指定了 TOOLCHAIN_DIR，设置 PATH
ifeq ($(TOOLCHAIN_DIR),)
    CC     ?= $(CROSS_COMPILE)gcc
    CXX    ?= $(CROSS_COMPILE)g++
    AR     ?= $(CROSS_COMPILE)ar
else
    CC     := $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)gcc
    CXX    := $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)g++
    AR     := $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)ar
    PATH   := $(TOOLCHAIN_DIR)/bin:$(PATH)
endif

CFLAGS   ?= -Wall -O2 -fPIC
CXXFLAGS ?= -Wall -O2 -fPIC -std=c++11

SRC_DIR := .
BUILD_DIR := $(SRC_DIR)/build
MPI_DIR    := $(SRC_DIR)/mpi
CTX_DIR    := $(SRC_DIR)/mpi_ctx
STREAM_DIR := $(SRC_DIR)/stream
LISTEN_DIR := $(SRC_DIR)/listeners
HW_DIR     := $(SRC_DIR)/hw_check
MEDIA_DIR  := $(SRC_DIR)/media
LOG_DIR    := $(SRC_DIR)/log

INCLUDES := -I$(SRC_DIR) -I$(MPI_DIR) -I$(CTX_DIR) -I$(STREAM_DIR) -I$(LISTEN_DIR) -I$(HW_DIR) -I$(MEDIA_DIR) -I$(LOG_DIR) -I$(MPI_DIR)/rockit/inc

C_SRCS   := $(MPI_DIR)/mpi_intf.c $(MPI_DIR)/mpi_stub.c $(CTX_DIR)/mpi_ctx_intf.c $(CTX_DIR)/json/cJSON.c $(CTX_DIR)/mpi_ctx_json.c
CXX_SRCS := $(STREAM_DIR)/stream_base.cpp \
            $(STREAM_DIR)/stream_vi.cpp $(STREAM_DIR)/stream_vpss.cpp $(STREAM_DIR)/stream_venc.cpp \
            $(STREAM_DIR)/stream_ai.cpp $(STREAM_DIR)/stream_aenc.cpp \
            $(STREAM_DIR)/stream_hdmi_video.cpp $(STREAM_DIR)/stream_hdmi_audio.cpp \
            $(LISTEN_DIR)/uds_stream.cpp $(LISTEN_DIR)/uds_connection_manager.cpp $(LISTEN_DIR)/file_stream.cpp \
            $(HW_DIR)/hw_check.cpp $(HW_DIR)/hdmi_check.cpp $(HW_DIR)/usb_check.cpp \
            $(MEDIA_DIR)/media_manager.cpp 

OBJS_C   := $(C_SRCS:.c=.o)
OBJS_CXX := $(CXX_SRCS:.cpp=.o)
OBJS := $(addprefix $(BUILD_DIR)/,$(OBJS_C) $(OBJS_CXX))
LIB  := $(BUILD_DIR)/libmedia_server.a
MAIN_OBJ := $(BUILD_DIR)/main.o
BIN  := $(BUILD_DIR)/media-server

# 平台 MPI 动态库（每个平台一个 .so）
SO_ROCKIT := $(BUILD_DIR)/libmpi_rockit.so
MPI_SOLIBS := $(SO_ROCKIT)

# Rockchip 链接库 (在 rockit 目录下)
ROCKIT_LIB_DIR := $(MPI_DIR)/rockit/lib
ROCKIT_LIBS := -L$(ROCKIT_LIB_DIR) -lrockit -ldl

.PHONY: all clean bin so

bin: $(LIB) $(BIN) $(MPI_SOLIBS)

all: $(BIN) $(MPI_SOLIBS)

so: $(MPI_SOLIBS)

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

$(BIN): $(LIB) $(MAIN_OBJ) 
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN_OBJ) $(LIB) -lpthread -ldl
 
$(MAIN_OBJ): main.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

# $(SO_ROCKIT):
# 	$(MAKE) -C $(MPI_DIR)/rockchip all

$(SO_ROCKIT): $(BUILD_DIR)/$(MPI_DIR)/rockit/mpi_rockit.o
	$(CC) -shared -o $@ $< -ldl

clean:
	rm -rf $(BUILD_DIR)
