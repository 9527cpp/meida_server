# media-server 框架静态库
# 可在源码树内直接 make，或由 OpenWrt Build/Compile 调用

CC     ?= gcc
CXX    ?= g++
AR     ?= ar
CFLAGS ?= -Wall -O2 -fPIC
CXXFLAGS ?= -Wall -O2 -fPIC -std=c++11

SRC_DIR := .
BUILD_DIR := $(SRC_DIR)/build
MPI_DIR    := $(SRC_DIR)/mpi
CTX_DIR    := $(SRC_DIR)/mpi_ctx
STREAM_DIR := $(SRC_DIR)/stream

# stream 模块静态库（独立构建）
STREAM_LIB := $(BUILD_DIR)/libstream.a
LISTEN_DIR := $(SRC_DIR)/listeners
HW_DIR     := $(SRC_DIR)/hw_check
MEDIA_DIR  := $(SRC_DIR)/media
LOG_DIR    := $(SRC_DIR)/log

INCLUDES := -I$(SRC_DIR) -I$(MPI_DIR) -I$(CTX_DIR) -I$(STREAM_DIR) -I$(LISTEN_DIR) -I$(HW_DIR) -I$(MEDIA_DIR) -I$(LOG_DIR)

C_SRCS   := $(MPI_DIR)/mpi_intf.c $(MPI_DIR)/mpi_stub.c $(CTX_DIR)/mpi_ctx_intf.c $(CTX_DIR)/json/cJSON.c $(CTX_DIR)/mpi_ctx_json.c
CXX_SRCS := $(LISTEN_DIR)/uds_stream.cpp $(LISTEN_DIR)/uds_connection_manager.cpp $(LISTEN_DIR)/file_stream.cpp \
            $(HW_DIR)/hw_check.cpp $(HW_DIR)/hdmi_check.cpp $(HW_DIR)/usb_check.cpp \
            $(MEDIA_DIR)/media_manager.cpp 

OBJS_C   := $(C_SRCS:.c=.o)
OBJS_CXX := $(CXX_SRCS:.cpp=.o)
OBJS_STREAM := $(STREAM_LIB)
OBJS := $(addprefix $(BUILD_DIR)/,$(OBJS_C) $(OBJS_CXX))
LIB  := $(BUILD_DIR)/libmedia_server.a
MAIN_OBJ := $(BUILD_DIR)/main.o
BIN  := $(BUILD_DIR)/media-server

# 平台 MPI 动态库（每个平台一个 .so）
SO_ROCKIT := $(BUILD_DIR)/libmpi_rockit.so
SO_STUB   := $(BUILD_DIR)/libmpi_stub.so
MPI_SOLIBS := #$(SO_ROCKIT) $(SO_STUB)

.PHONY: all clean bin so

bin: $(STREAM_LIB) $(LIB) $(BIN) $(MPI_SOLIBS)

all: $(BIN) $(MPI_SOLIBS)

so: $(MPI_SOLIBS)

$(STREAM_LIB):
	$(MAKE) -C $(STREAM_DIR) build

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

# $(SO_ROCKIT):
# 	$(MAKE) -C $(MPI_DIR)/rockchip all

$(SO_STUB): $(BUILD_DIR)/$(MPI_DIR)/mpi_stub.o
	$(CC) -shared -o $@ $<

clean:
	rm -rf $(BUILD_DIR)
