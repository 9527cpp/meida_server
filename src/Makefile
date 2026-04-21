# media-server 框架静态库
# 可在源码树内直接 make，或由 OpenWrt Build/Compile 调用
#
# 交叉编译用法:
#   make CROSS_COMPILE=aarch64-openwrt-linux- \
#        TOOLCHAIN_DIR=/path/to/toolchain TARGET_ARCH=aarch64 -f Makefile

# 交叉编译工具链 (通过 CROSS_COMPILE 和 TOOLCHAIN_DIR 指定)
CROSS_COMPILE ?=
TOOLCHAIN_DIR ?=

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

SRC_DIR  := .
BUILD_DIR := $(SRC_DIR)/build
MPI_DIR   := $(SRC_DIR)/mpi
CTX_DIR   := $(SRC_DIR)/mpi_ctx
STREAM_DIR := $(SRC_DIR)/stream

# stream 模块静态库（独立构建）
STREAM_LIB := $(BUILD_DIR)/libstream.a
LISTEN_DIR := $(SRC_DIR)/listeners
HW_DIR    := $(SRC_DIR)/hw_check
MEDIA_DIR  := $(SRC_DIR)/media
LOG_DIR    := $(SRC_DIR)/log

INCLUDES := -I$(SRC_DIR) -I$(MPI_DIR) -I$(CTX_DIR) -I$(STREAM_DIR) \
            -I$(LISTEN_DIR) -I$(HW_DIR) -I$(MEDIA_DIR) -I$(LOG_DIR) \
            -I$(MPI_DIR)/rockit/inc

C_SRCS   := $(MPI_DIR)/mpi_intf.c $(MPI_DIR)/mpi_stub.c $(CTX_DIR)/mpi_ctx_intf.c $(CTX_DIR)/json/cJSON.c $(CTX_DIR)/mpi_ctx_json.c
CXX_SRCS := $(LISTEN_DIR)/uds_stream.cpp \
            $(LISTEN_DIR)/uds_connection_manager.cpp \
            $(LISTEN_DIR)/file_stream.cpp \
            $(MEDIA_DIR)/media_manager.cpp

OBJS_C   := $(C_SRCS:.c=.o)
OBJS_CXX := $(CXX_SRCS:.cpp=.o)
OBJS_STREAM := $(STREAM_LIB)
OBJS := $(addprefix $(BUILD_DIR)/,$(OBJS_C) $(OBJS_CXX))
LIB  := $(BUILD_DIR)/libmedia_server.a
MAIN_OBJ := $(BUILD_DIR)/main.o
BIN  := $(BUILD_DIR)/media-server

.PHONY: all clean bin so mpi hw_check

all: bin so hw_check

bin: $(STREAM_LIB) $(LIB) $(BIN) $(MPI_SOLIBS)

so: mpi

mpi:
	$(MAKE) -C $(MPI_DIR) CROSS_COMPILE=$(CROSS_COMPILE) \
	        TOOLCHAIN_DIR=$(TOOLCHAIN_DIR)

hw_check:
	$(MAKE) -C $(HW_DIR) CROSS_COMPILE=$(CROSS_COMPILE) \
	        TOOLCHAIN_DIR=$(TOOLCHAIN_DIR)

$(STREAM_LIB):
	$(MAKE) -C $(STREAM_DIR) build \
		CROSS_COMPILE="$(CROSS_COMPILE)" \
		TOOLCHAIN_DIR="$(TOOLCHAIN_DIR)" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		AR="$(AR)"

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

$(BIN): $(STREAM_LIB) $(LIB) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN_OBJ) $(LIB) $(STREAM_LIB) -lpthread -ldl

$(MAIN_OBJ): main.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C $(MPI_DIR) clean
	$(MAKE) -C $(STREAM_DIR) clean
	$(MAKE) -C $(HW_DIR) clean
