# media-server 框架静态库
# 可在源码树内直接 make，或由 OpenWrt Build/Compile 调用

CC     ?= gcc
CXX    ?= g++
AR     ?= ar
CFLAGS ?= -Wall -O2 -fPIC
CXXFLAGS ?= -Wall -O2 -fPIC -std=c++11

SRC_DIR := .
MPI_DIR    := $(SRC_DIR)/mpi
CTX_DIR    := $(SRC_DIR)/mpi_ctx
STREAM_DIR := $(SRC_DIR)/stream
LISTEN_DIR := $(SRC_DIR)/listeners
HW_DIR     := $(SRC_DIR)/hw_check
MEDIA_DIR  := $(SRC_DIR)/media

INCLUDES := -I$(SRC_DIR) -I$(MPI_DIR) -I$(CTX_DIR) -I$(STREAM_DIR) -I$(LISTEN_DIR) -I$(HW_DIR) -I$(MEDIA_DIR) 

C_SRCS   := $(MPI_DIR)/mpi_intf.c $(CTX_DIR)/mpi_ctx_intf.c $(MPI_DIR)/mpi_rockit.c $(MPI_DIR)/mpi_stub.c $(CTX_DIR)/json/cJSON.c $(CTX_DIR)/mpi_ctx_json.c
CXX_SRCS := $(STREAM_DIR)/stream_base.cpp $(STREAM_DIR)/stream_pipe.cpp \
            $(STREAM_DIR)/stream_vi.cpp $(STREAM_DIR)/stream_venc.cpp \
            $(STREAM_DIR)/stream_ai.cpp $(STREAM_DIR)/stream_aenc.cpp \
            $(STREAM_DIR)/stream_hdmi_video.cpp $(STREAM_DIR)/stream_hdmi_audio.cpp \
            $(LISTEN_DIR)/uds_stream.cpp $(LISTEN_DIR)/uds_connection_manager.cpp $(LISTEN_DIR)/file_stream.cpp \
            $(HW_DIR)/hw_check.cpp $(HW_DIR)/hdmi_check.cpp $(HW_DIR)/usb_check.cpp \
            $(MEDIA_DIR)/media_manager.cpp 

OBJS := $(C_SRCS:.c=.o) $(CXX_SRCS:.cpp=.o)
LIB  := libmedia_server.a
MAIN_OBJ := main.o
BIN  := media-server

.PHONY: all clean bin


bin: $(LIB) $(BIN)

all: $(LIB) $(bin)

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

$(BIN): $(LIB) $(MAIN_OBJ) 
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN_OBJ) $(LIB) -lpthread
	rm -f $(OBJS) $(MAIN_OBJ)

$(MAIN_OBJ): main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(OBJS) $(MAIN_OBJ) $(LIB) $(BIN)
