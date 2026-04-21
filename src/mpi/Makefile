# mpi 子目录 Makefile
# 编译各平台 MPI 实现（rockit / hisi / rkmedia / stub）
#
# 用法:
#   make -C src/mpi                  # 编译所有平台 .so
#   make -C src/mpi rockit          # 仅编译 rockit
#   make -C src/mpi hisi            # 仅编译 hisi
#   make -C src/mpi rkmedia         # 仅编译 rkmedia
#   make -C src/mpi stub            # 仅编译 stub
#   make -C src/mpi demo            # 编译 demo 程序
#   make -C src/mpi test            # 编译测试程序（需 MPI_PLATFORM 指定平台）
#
# 交叉编译:
#   make -C src/mpi CROSS_COMPILE=aarch64-openwrt-linux- TOOLCHAIN_DIR=/home/lijun/workspace/sunlogin-orayos/staging_dir/toolchain-aarch64_cortex-a53_gcc-12.3.0_glibc MPI_PLATFORM=rockit

MPI_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
BUILD_DIR := $(MPI_DIR)/build
LOG_DIR := $(MPI_DIR)/../log

# 交叉编译工具链（强制赋值，不继承环境变量）
CC     := $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)gcc
AR     := $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)ar
CFLAGS ?= -Wall -O2 -fPIC -DDYNAMIC_MPI

INCLUDES := -I$(MPI_DIR) \
            -I$(MPI_DIR)/.. \
            -I$(MPI_DIR)/../mpi_ctx \
            -I$(LOG_DIR) \
            -I$(MPI_DIR)/rockit/inc

# 默认目标：编译所有平台
.PHONY: all rockit hisi rkmedia stub clean test demo

all: rockit hisi rkmedia stub 

# ---------- rockit ----------
ROCKIT_SRC := $(MPI_DIR)/rockit/mpi_rockit.c
ROCKIT_OBJ := $(BUILD_DIR)/rockit/mpi_rockit.o
ROCKIT_LIB := $(BUILD_DIR)/libmpi_rockit.so
ROCKIT_LIB_DIR := $(MPI_DIR)/rockit/lib

CAN_LINK_ROCKIT := $(shell ROCKIT_SO="$(ROCKIT_LIB_DIR)/librockit.so"; \
                    if [ ! -f "$$ROCKIT_SO" ]; then echo "no"; \
                    elif file "$$ROCKIT_SO" | grep -q "ELF 64-bit LSB.*x86-64"; then echo "yes"; \
                    else echo "no"; fi)

rockit: $(ROCKIT_LIB)

$(ROCKIT_LIB): $(ROCKIT_OBJ)
	@mkdir -p $(dir $@)
	@if [ "$(CAN_LINK_ROCKIT)" = "yes" ]; then \
		$(CC) $(CFLAGS) -shared -o $@ $(ROCKIT_OBJ) \
			-L$(ROCKIT_LIB_DIR) -lrockit -ldl; \
		echo "[INFO] rockit .so linked with librockit"; \
	else \
		$(CC) $(CFLAGS) -shared -o $@ $(ROCKIT_OBJ) -ldl; \
		echo "[WARN] rockit .so built without librockit (arch mismatch or not found)"; \
	fi

$(ROCKIT_OBJ): $(ROCKIT_SRC)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# ---------- hisi ----------
HISI_SRC := $(MPI_DIR)/hisi/mpi_hisi.c
HISI_OBJ := $(BUILD_DIR)/hisi/mpi_hisi.o
HISI_LIB := $(BUILD_DIR)/libmpi_hisi.so

hisi: $(HISI_LIB)

$(HISI_LIB): $(HISI_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -shared -o $@ $< -ldl

$(HISI_OBJ): $(HISI_SRC)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# ---------- rkmedia ----------
RKMEDIA_SRC := $(MPI_DIR)/rkmedia/mpi_rkmedia.c
RKMEDIA_OBJ := $(BUILD_DIR)/rkmedia/mpi_rkmedia.o
RKMEDIA_LIB := $(BUILD_DIR)/libmpi_rkmedia.so

rkmedia: $(RKMEDIA_LIB)

$(RKMEDIA_LIB): $(RKMEDIA_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -shared -o $@ $< -ldl

$(RKMEDIA_OBJ): $(RKMEDIA_SRC)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# ---------- stub ----------
STUB_SRC := $(MPI_DIR)/mpi_stub.c
STUB_OBJ := $(BUILD_DIR)/stub/mpi_stub.o
STUB_LIB := $(BUILD_DIR)/libmpi_stub.so

stub: $(STUB_LIB)

$(STUB_LIB): $(STUB_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -shared -o $@ $< -ldl

$(STUB_OBJ): $(STUB_SRC)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# ---------- 测试 ----------
# 编译:  make -C src/mpi test MPI_PLATFORM=rockit
# 运行:  ./build/test_mpi_rockit
TEST_BUILD_DIR := $(BUILD_DIR)
TEST_SRC := $(MPI_DIR)/test_mpi.c

test: test_rockit test_hisi test_rkmedia test_stub

test_rockit:
	@mkdir -p $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -DMPI_PLATFORM_ROCKIT $(INCLUDES) -o $(TEST_BUILD_DIR)/test_mpi_rockit $(TEST_SRC) -ldl -lm
	@echo "[INFO] built: $(TEST_BUILD_DIR)/test_mpi_rockit"

test_hisi:
	@mkdir -p $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -DMPI_PLATFORM_HISI $(INCLUDES) -o $(TEST_BUILD_DIR)/test_mpi_hisi $(TEST_SRC) -ldl -lm
	@echo "[INFO] built: $(TEST_BUILD_DIR)/test_mpi_hisi"

test_rkmedia:
	@mkdir -p $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -DMPI_PLATFORM_RKMEDIA $(INCLUDES) -o $(TEST_BUILD_DIR)/test_mpi_rkmedia $(TEST_SRC) -ldl -lm
	@echo "[INFO] built: $(TEST_BUILD_DIR)/test_mpi_rkmedia"

test_stub:
	@mkdir -p $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -DMPI_PLATFORM_STUB $(INCLUDES) -o $(TEST_BUILD_DIR)/test_mpi_stub $(TEST_SRC) -ldl -lm
	@echo "[INFO] built: $(TEST_BUILD_DIR)/test_mpi_stub"

# ---------- demo ----------
# 演示：从创建 mpi_intf 到取帧到释放的全流程
# 编译:  make -C src/mpi demo MPI_PLATFORM=rockit
# 运行:  ./build/demo_mpi_rockit
DEMO_SRC := $(MPI_DIR)/demo_mpi.c

demo: demo_rockit demo_hisi demo_rkmedia demo_stub

demo_rockit:
	@mkdir -p $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -DMPI_PLATFORM_ROCKIT $(INCLUDES) -o $(TEST_BUILD_DIR)/demo_mpi_rockit $(DEMO_SRC) -ldl -lm -lpthread
	@echo "[INFO] built: $(TEST_BUILD_DIR)/demo_mpi_rockit"

demo_hisi:
	@mkdir -p $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -DMPI_PLATFORM_HISI $(INCLUDES) -o $(TEST_BUILD_DIR)/demo_mpi_hisi $(DEMO_SRC) -ldl -lm -lpthread
	@echo "[INFO] built: $(TEST_BUILD_DIR)/demo_mpi_hisi"

demo_rkmedia:
	@mkdir -p $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -DMPI_PLATFORM_RKMEDIA $(INCLUDES) -o $(TEST_BUILD_DIR)/demo_mpi_rkmedia $(DEMO_SRC) -ldl -lm -lpthread
	@echo "[INFO] built: $(TEST_BUILD_DIR)/demo_mpi_rkmedia"

demo_stub:
	@mkdir -p $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -DMPI_PLATFORM_STUB $(INCLUDES) -o $(TEST_BUILD_DIR)/demo_mpi_stub $(DEMO_SRC) -ldl -lm -lpthread
	@echo "[INFO] built: $(TEST_BUILD_DIR)/demo_mpi_stub"

clean:
	rm -rf $(BUILD_DIR)
