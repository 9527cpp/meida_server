# AGENTS.md — media-server-new

## Build (local, from repo root)

```bash
make -C src              # static lib only → src/build/libmedia_server.a
make -C src bin           # binary + lib + MPI .so → src/build/media-server, src/build/libmedia_server.a
make -C src so            # only MPI shared libs (currently stub only; rockit commented out)
make -C src clean
```

**OpenWrt packaging**: the root `Makefile` is an OpenWrt package Makefile — it requires `TOPDIR` set by the OpenWrt buildroot and copies `src/mpi/*.h` + `src/mpi_ctx/*.h` to `/usr/include/media-server` and `libmedia_server.a` to `/usr/lib`. Do not run `make` from repo root without TOPDIR.

## Runtime

```bash
./src/build/media-server --mpi <libmpi_xxx.so> --enable-uds   # UDS server mode
./src/build/media-server --mpi <libmpi_xxx.so> --enable-file  # file output mode
./src/build/media-server --mpi <libmpi_xxx.so>                 # no output (media manager only)
./src/build/media-server --help
```

`--enable-uds` and `--enable-file` are mutually exclusive (both claim hdmi video channel 0).

Default UDS socket: `/tmp/media_server.sock`.

## Testing

```bash
python3 test/test.py video      # libuds protocol, requests video stream
python3 test/test.py audio
python3 test/test.py audio_mic
python3 test/test.py --legacy video   # legacy 1-byte protocol
```

Server must be running with `--enable-uds` before the test client connects.

## Language convention

- **C** (`.c`, `.h`): MPI interface and platform plugins (`src/mpi/`)
- **C++11** (`.cpp`, `.hpp`): streams, media manager, listeners (`src/stream/`, `src/media/`, `src/listeners/`)
- cJSON is vendored under `src/mpi_ctx/json/` — do not add another JSON library

## MPI plugin naming

- Pattern: `libmpi_<platform>.so` (e.g., `libmpi_rockit.so`)
- Currently only `libmpi_stub.so` builds; rockit is commented out in `src/Makefile:41`

## Include paths

All subdirectory includes are set via `INCLUDES` in `src/Makefile:20`. When adding new source directories, add `-I<dir>` there, not inline.

## Directory ownership

| Directory | Language | Purpose |
|-----------|----------|---------|
| `src/mpi/` | C | MPI plugin interface + stub |
| `src/mpi_ctx/` | C | JSON config loading, cJSON |
| `src/stream/` | C++ | stream_base, VI/VPSS/VENC/AI/AENC/Hdmi pipeline |
| `src/listeners/` | C++ | UDS socket server, file output |
| `src/media/` | C++ | media_manager event loop |
| `src/hw_check/` | C++ | HDMI/USB hotplug detection |
| `src/log/` | C | logging macros |
| `src/main.cpp` | C++ | entry point, dlopen MPI .so |

## No linter / no codegen

No lint, typecheck, or code-generation tooling is configured in this repo. Style is implicit from the sources.
