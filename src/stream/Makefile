# stream 模块独立构建
# 支持: make build, make clean, make test

CXX ?= g++
CXXFLAGS ?= -Wall -O2 -fPIC -std=c++11

# 路径（使用父级 build 目录，便于链接）
PARENT_DIR := ..
BUILD_DIR := $(PARENT_DIR)/build
LOG_DIR := $(PARENT_DIR)/log
MPI_CTX_DIR := $(PARENT_DIR)/mpi_ctx

INCLUDES := -I$(PARENT_DIR) -I. -I$(LOG_DIR) -I$(MPI_CTX_DIR)

# 源文件
CXX_SRCS := \
	stream_base.cpp \
	stream_vi.cpp \
	stream_vpss.cpp \
	stream_venc.cpp \
	stream_ai.cpp \
	stream_aenc.cpp \
	stream_hdmi_video.cpp \
	stream_hdmi_audio.cpp

OBJS := $(CXX_SRCS:.cpp=.o)
OBJS := $(addprefix $(BUILD_DIR)/stream/,$(OBJS))
LIB := $(BUILD_DIR)/libstream.a

# 测试配置
TEST_SRC := test_stream.cpp
TEST_STUB := test_mpi_stub.cpp
TEST_OBJ := $(BUILD_DIR)/stream/test_stream.o
TEST_STUB_OBJ := $(BUILD_DIR)/stream/test_mpi_stub.o
TEST_BIN := $(BUILD_DIR)/test_stream
TEST_LDFLAGS := -lpthread

.PHONY: all build clean test

all: build

build: $(LIB)

test: $(LIB)
	@mkdir -p $(dir $(TEST_OBJ))
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(TEST_OBJ) $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(TEST_STUB_OBJ) $(TEST_STUB)
	$(CXX) $(CXXFLAGS) -o $(TEST_BIN) $(TEST_OBJ) $(TEST_STUB_OBJ) $(LIB) $(TEST_LDFLAGS)
	@echo "Running stream tests..."
	@$(TEST_BIN)

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

$(BUILD_DIR)/stream/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(LIB) $(OBJS) $(TEST_OBJ) $(TEST_STUB_OBJ) $(TEST_BIN)
