# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Local build
make -C src                    # Build static lib
make -C src bin                # Build binary + static lib (outputs: src/build/media-server, src/build/libmedia_server.a)
make -C src so                 # Build platform MPI shared libs (src/build/libmpi_*.so)
make -C src clean              # Clean build artifacts

# OpenWrt package build
# Use OpenWrt buildroot (TOPDIR) to run full packaging via top-level Makefile
```

## High-Level Architecture

media-server is a lightweight MPI/stream abstraction framework for embedded media pipelines. It provides:

- **MPI abstraction**: Platform-specific media hardware access via dynamically-loaded shared libs (`libmpi_<platform>.so`)
- **Stream pipeline**: Modular data flow from video/audio input (VI/AI) through optional processing to encoding output (VENC/AENC)
- **Listener system**: Pluggable output handlers (UDS socket server, file output)
- **Hardware detection**: HDMI/USB hotplug detection that gates stream creation
- **Media manager**: Central event loop orchestrating streams, listeners, and hardware events

### Directory Structure

| Directory | Purpose |
|-----------|---------|
| `src/mpi/` | MPI interface (C) + platform-specific implementations as `.so` plugins |
| `src/mpi_ctx/` | Context loading/parsing (JSON), cJSON library |
| `src/stream/` | Stream implementations (C++): `stream_base`, `stream_vi`, `stream_venc`, `stream_ai`, `stream_aenc`, `stream_pipe` |
| `src/listeners/` | Output handlers: UDS socket server, file stream |
| `src/media/` | `media_manager` (C++) - event loop, stream orchestration |
| `src/hw_check/` | Hardware detection: HDMI, USB camera |
| `src/log/` | Logging utilities |

### Key Abstractions

- **stream_base**: Base class for all stream nodes; manages data thread, listener list, and upstream/downstream connections
- **stream_pipe**: Composes `stream_base` nodes into input → process → output chains (e.g., `stream_hdmi_video` = VI + VENC)
- **mpi_intf**: Plugin interface for platform-specific MPI implementations; loaded via `dlsym` from `libmpi_<platform>.so`
- **mpi_ctx**: Runtime configuration loaded from JSON config file

## Running Tests

```bash
python3 test/test.py [video|audio|audio_mic]   # UDS client test
python3 test/test.py --legacy video             # Legacy protocol test
```

Default socket: `/tmp/media_server.sock`

## Important Conventions

- **C for MPI/plugin code**, **C++11 for stream and media code**
- Platform MPI libs follow naming: `libmpi_<platform>.so`
- cJSON is vendored under `src/mpi_ctx/json/` - do not duplicate
- Include paths are set in `src/Makefile` INCLUDES variable
