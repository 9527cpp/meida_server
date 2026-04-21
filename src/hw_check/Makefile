# hw_check 子目录 Makefile
# 编译各平台 hw_check .so（hdmi / usb）
#
# 用法:
#   make -C src/hw_check                  # 编译所有平台 .so
#   make -C src/hw_check hdmi            # 仅编译 hdmi
#   make -C src/hw_check usb            # 仅编译 usb
#   make -C src/hw_check test           # 编译测试程序
#   make -C src/hw_check demo           # 编译演示程序
#   make -C src/hw_check clean          # 清理
#
# 交叉编译:
#   make -C src/hw_check CROSS_COMPILE=aarch64-openwrt-linux- \
#        TOOLCHAIN_DIR=/path/to/toolchain

HW_CHECK_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
BUILD_DIR    := $(HW_CHECK_DIR)/build
LOG_DIR      := $(HW_CHECK_DIR)/../log

# 交叉编译工具链（强制赋值，不继承环境变量）
CC      := $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)gcc
AR      := $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)ar
CXX     := $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)g++
CFLAGS  ?= -Wall -O2 -fPIC
CXXFLAGS ?= -Wall -O2 -fPIC -std=c++11

INCLUDES := -I$(HW_CHECK_DIR) \
            -I$(HW_CHECK_DIR)/.. \
            -I$(LOG_DIR)

LDFLAGS := -lpthread -ldl

# 默认目标：编译所有平台 .so
.PHONY: all hdmi usb clean test demo

all: hdmi usb

# ---------- hdmi ----------
HDMI_BUILD := $(BUILD_DIR)/hdmi
HDMI_OBJ := \
	$(HDMI_BUILD)/hdmi_check.o \
	$(HDMI_BUILD)/hw_check.o \
	$(HDMI_BUILD)/hw_check_factory.o
HDMI_LIB := $(BUILD_DIR)/libhw_check_hdmi.so

hdmi: $(HDMI_LIB)

$(HDMI_LIB): $(HDMI_OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(HDMI_OBJ) $(LDFLAGS)
	@echo "[INFO] built: $@"

$(HDMI_BUILD)/%.o: $(HW_CHECK_DIR)/%.cpp | $(HDMI_BUILD)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(HDMI_BUILD)/hw_check_factory.o: $(HW_CHECK_DIR)/hw_check_factory.cpp | $(HDMI_BUILD)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -fno-exceptions -c -o $@ $<

$(HDMI_BUILD)::
	@mkdir -p $@

# ---------- usb ----------
USB_BUILD := $(BUILD_DIR)/usb
USB_OBJ := \
	$(USB_BUILD)/usb_check.o \
	$(USB_BUILD)/hw_check.o \
	$(USB_BUILD)/hw_check_factory.o
USB_LIB := $(BUILD_DIR)/libhw_check_usb.so

usb: $(USB_LIB)

$(USB_LIB): $(USB_OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(USB_OBJ) $(LDFLAGS)
	@echo "[INFO] built: $@"

$(USB_BUILD)/%.o: $(HW_CHECK_DIR)/%.cpp | $(USB_BUILD)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(USB_BUILD)/hw_check_factory.o: $(HW_CHECK_DIR)/usb_factory.cpp | $(USB_BUILD)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -fno-exceptions -c -o $@ $<

$(USB_BUILD)::
	@mkdir -p $@

# ---------- 测试 ----------
# 编译:  make -C src/hw_check test
# 运行:  ./build/test_hw_check_hdmi ./build/libhw_check_hdmi.so
TEST_BUILD_DIR := $(BUILD_DIR)
TEST_SRC := $(HW_CHECK_DIR)/test_hw_check.c
TEST_HDMI_BIN := $(TEST_BUILD_DIR)/test_hw_check_hdmi
TEST_USB_BIN := $(TEST_BUILD_DIR)/test_hw_check_usb

test: $(TEST_HDMI_BIN) $(TEST_USB_BIN)

$(TEST_HDMI_BIN): $(TEST_SRC) $(HDMI_LIB)
	@mkdir -p $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) -DHW_CHECK_TYPE=0 $(INCLUDES) -ldl
	@echo "[INFO] built: $@"

$(TEST_USB_BIN): $(TEST_SRC) $(USB_LIB)
	@mkdir -p $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) -DHW_CHECK_TYPE=1 $(INCLUDES) -ldl
	@echo "[INFO] built: $@"

# ---------- demo ----------
# 编译:  make -C src/hw_check demo
# 运行:  ./build/demo_hw_check_hdmi -w ./build/libhw_check_hdmi.so
DEMO_SRC := $(HW_CHECK_DIR)/demo_hw_check.c
DEMO_HDMI_BIN := $(TEST_BUILD_DIR)/demo_hw_check_hdmi
DEMO_USB_BIN := $(TEST_BUILD_DIR)/demo_hw_check_usb

demo: $(DEMO_HDMI_BIN) $(DEMO_USB_BIN)

$(DEMO_HDMI_BIN): $(DEMO_SRC) $(HDMI_LIB)
	@mkdir -p $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(DEMO_SRC) -DHW_CHECK_TYPE=0 $(INCLUDES) -lpthread -ldl
	@echo "[INFO] built: $@"

$(DEMO_USB_BIN): $(DEMO_SRC) $(USB_LIB)
	@mkdir -p $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(DEMO_SRC) -DHW_CHECK_TYPE=1 $(INCLUDES) -lpthread -ldl
	@echo "[INFO] built: $@"

clean:
	rm -rf $(BUILD_DIR)
