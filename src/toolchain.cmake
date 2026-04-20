# Cross-compilation toolchain file for aarch64 (OpenWrt)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Toolchain path
set(TOOLCHAIN_DIR /home/lijun/workspace/sunlogin-orayos/staging_dir/toolchain-aarch64_cortex-a53_gcc-12.3.0_glibc)

# Set compilers
set(CMAKE_C_COMPILER   ${TOOLCHAIN_DIR}/bin/aarch64-openwrt-linux-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-openwrt-linux-g++)
set(CMAKE_AR           ${TOOLCHAIN_DIR}/bin/aarch64-openwrt-linux-ar CACHE FILEPATH "Archiver")
set(CMAKE_RANLIB       ${TOOLCHAIN_DIR}/bin/aarch64-openwrt-linux-ranlib CACHE FILEPATH "Ranlib")
set(CMAKE_LINKER       ${TOOLCHAIN_DIR}/bin/aarch64-openwrt-linux-ld CACHE FILEPATH "Linker")

# Search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Build flags
set(CMAKE_C_FLAGS   "-Wall -O2 -fPIC")
set(CMAKE_CXX_FLAGS "-Wall -O2 -fPIC -std=c++11")
