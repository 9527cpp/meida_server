# MPI 交叉编译指南

本文档说明如何交叉编译 `src/mpi/` 目录下的各平台 MPI 共享库。

## 目录结构

```
src/mpi/
├── CMakeLists.txt          # CMake 构建配置
├── Makefile                # Make 构建配置
├── mpi.toolchain.cmake     # CMake 交叉编译工具链模板
├── rockit/                 # rockit 平台实现
│   ├── mpi_rockit.c
│   ├── inc/
│   └── lib/                # 预编译的 librockit.so
├── hisi/                   # hisi 平台实现
│   └── mpi_hisi.c
├── rkmedia/                # rkmedia 平台实现
│   └── mpi_rkmedia.c
├── mpi_stub.c              # stub 空实现（默认）
├── test_mpi.c              # 单元测试
└── demo_mpi.c              # 全流程演示
```

## 平台与输出

| 平台 | 共享库 | 说明 |
|------|--------|------|
| rockit | `libmpi_rockit.so` | 需要 librockit.so |
| hisi | `libmpi_hisi.so` | 海思平台 |
| rkmedia | `libmpi_rkmedia.so` | RK 平台 |
| stub | `libmpi_stub.so` | 空实现，仅用于主机测试 |

输出目录：`src/mpi/build/`

---

## Make 构建

### 本地构建

```bash
cd src/mpi
make              # 编译所有平台 .so
make rockit        # 仅 rockit
make hisi          # 仅 hisi
make rkmedia       # 仅 rkmedia
make stub          # 仅 stub
make clean         # 清理
```

### 交叉编译

```bash
cd src/mpi
make CROSS_COMPILE=aarch64-openwrt-linux- \
     TOOLCHAIN_DIR=/path/to/toolchain
```

或直接指定编译器：

```bash
make CROSS_COMPILE=aarch64-openwrt-linux- \
     CC=aarch64-openwrt-linux-gcc
```

### 编译测试程序

```bash
# 先编译对应平台的 .so
make stub

# 编译 stub 平台测试
make test_stub

# 编译 stub 平台演示
make demo_stub
```

输出位置：`src/mpi/build/test_mpi_<platform>`、`src/mpi/build/demo_mpi_<platform>`

---

## CMake 构建

### 本地构建

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make                        # 编译所有 .so
make mpi_rockit             # 仅 rockit
make mpi_hisi              # 仅 hisi
make mpi_rkmedia            # 仅 rkmedia
make mpi_stub               # 仅 stub
```

### 交叉编译（工具链文件）

```bash
mkdir build && cd build
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=mpi.toolchain.cmake \
    -DTOOLCHAIN_PREFIX=aarch64-openwrt-linux- \
    -DTOOLCHAIN_DIR=/path/to/toolchain \
    -DCMAKE_BUILD_TYPE=Release
make
```

### 交叉编译（直接指定编译器）

```bash
mkdir build && cd build
cmake .. \
    -DCMAKE_C_COMPILER=aarch64-openwrt-linux-gcc \
    -DCMAKE_CXX_COMPILER=aarch64-openwrt-linux-g++ \
    -DCMAKE_SYSROOT=/path/to/sysroot \
    -DCMAKE_BUILD_TYPE=Release
make
```

### 工具链模板参数

`mpi.toolchain.cmake` 支持以下变量：

| 参数 | 说明 | 示例 |
|------|------|------|
| `TOOLCHAIN_PREFIX` | 交叉编译器前缀 | `aarch64-openwrt-linux-` |
| `TOOLCHAIN_DIR` | 工具链根目录（含 bin/lib） | `/home/user/toolchain` |

### 编译测试程序

```bash
cmake .. -DMPI_TEST=stub -DCMAKE_BUILD_TYPE=Release
make mpi_test
./mpi_test
```

平台选项：`rockit` / `hisi` / `rkmedia` / `stub`

### 编译演示程序

```bash
cmake .. -DMPI_DEMO=stub -DCMAKE_BUILD_TYPE=Release
make mpi_demo
./mpi_demo
```

---

## rockit 平台特殊说明

`libmpi_rockit.so` 依赖预编译的 `librockit.so`。该库为目标平台（aarch64）编译，无法在 x86-64 主机上链接。

Make 构建时会自动检测架构兼容性：

- 若 `librockit.so` 为 x86-64：跳过链接，仅生成 `libmpi_rockit.so`（无可用硬件抽象）
- 若 `librockit.so` 为 aarch64 且在目标工具链下：正常链接
- 若 `librockit.so` 不存在：同上，跳过链接

CMake 构建默认 `ENABLE_ROCKIT=ON`，但若找不到兼容的 `librockit.so` 会自动跳过链接。交叉编译时可保留 `ENABLE_ROCKIT=ON`，由工具链提供正确的 librockit.so。

---

## 完整交叉编译示例（Make）

```bash
cd src/mpi
make clean
make CROSS_COMPILE=aarch64-openwrt-linux- \
     TOOLCHAIN_DIR=/home/user/openwrt/staging_dir/toolchain-aarch64_cortex-a53_gcc-12.3.0_glibc
ls build/
# libmpi_hisi.so  libmpi_rkmedia.so  libmpi_rockit.so  libmpi_stub.so
```

## 完整交叉编译示例（CMake）

```bash
cd src/mpi
rm -rf build
mkdir build && cd build
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=mpi.toolchain.cmake \
    -DTOOLCHAIN_PREFIX=aarch64-openwrt-linux- \
    -DTOOLCHAIN_DIR=/home/user/openwrt/staging_dir/toolchain-aarch64_cortex-a53_gcc-12.3.0_glibc \
    -DCMAKE_BUILD_TYPE=Release \
    -DMPI_TEST=stub
make mpi_test mpi_stub
./mpi_test
```
