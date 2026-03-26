# Copilot instructions for media-server-new

This file summarizes how to build, test, and understand the media-server framework. Keep it focused on repository-specific commands, architecture, and conventions so Copilot sessions can quickly help contributors.

---

## Build, test, and lint commands

- Build (local):
  - From repository root: `make -C src`
  - Build binary + static lib: `make -C src bin` (produces `src/build/media-server` and `src/build/libmedia_server.a`).
  - Build only platform MPI shared libs: `make -C src so` (produces `src/build/libmpi_<platform>.so`).
  - Clean: `make -C src clean`.

- OpenWrt package build:
  - The top-level `Makefile` is an OpenWrt-style package Makefile. Use the OpenWrt buildroot (TOPDIR) to run full packaging; the package installs headers to `/usr/include/media-server` and lib to `/usr/lib`.

- Run the binary (after building):
  - `./src/build/media-server` (may require proper runtime environment / permissions).

- Single test (manual):
  - Test UDS client helper: `python3 test/test.py [video|audio|audio_mic]`.
  - Legacy protocol: `python3 test/test.py --legacy video`.
  - The test script connects to `/tmp/media_server.sock` by default.

- Linting: No repository-wide linter or lint targets detected. Follow project C/C++ standards (C for MPI/plugin code, C++11 for streams/media). Add CI/linter config if desired.

---

## High-level architecture (big picture)

- Purpose: a lightweight media-server framework providing an MPI abstraction layer, stream implementations, listeners, hardware checks, and a media manager.

- Major subsystems (directory -> responsibility):
  - `src/mpi/` — MPI interface and platform-specific implementations. Each platform implements a `libmpi_<platform>.so` (rockchip/rkmedia, stub, etc.).
  - `src/mpi_ctx/` — Context and JSON helpers (includes cJSON). Implements the runtime context and JSON serialization used by MPI and other components.
  - `src/stream/` — Stream implementations and helpers: `stream_base`, `stream_vi`, `stream_venc`, `stream_aenc`, `stream_ai`, `stream_hdmi_*`, `stream_pipe`, etc. These are C++ components using the framework's stream abstractions.
  - `src/listeners/` — Connection/listener implementations (UDS socket-based clients, file stream adapter) and the connection manager.
  - `src/media/` — `media_manager` (C++) orchestrates streams, listeners and event flow.
  - `src/hw_check/` — Hardware detection helpers (HDMI, USB) that gate functionality.
  - `src/log/` — Minimal tagging/log header(s).
  - `src/main.cpp` — Program entrypoint that links the static library and starts runtime.

- Build outputs: static library `libmedia_server.a`, executable `media-server`, and platform MPI shared libs under `src/build/`.

---

## Key repository conventions and patterns

- Mixed C and C++ code:
  - C (`.c`, `.h`) is used for the MPI interface and platform plugin implementations so they can be built as shared libs for different platforms.
  - C++ (`.cpp`, `.hpp`) is used for stream implementations, media manager, and the main program. C++ is compiled with `-std=c++11` by default.

- Build layout:
  - Source-root build directory: `src/build/`.
  - Public headers are in `src/mpi/` and `src/mpi_ctx/`; OpenWrt packaging installs them under `/usr/include/media-server`.
  - Keep include paths aligned with `src/Makefile` INCLUDES setting when adding code.

- MPI plugins naming and discovery:
  - Platform MPI shared libs follow `libmpi_<platform>.so`. The build system expects these names; use the same prefix when adding platforms.

- UDS protocol and tests:
  - The test helper uses `/tmp/media_server.sock` and a `libuds` framing format (see `test/test.py`). Keep compatibility with this framing if changing the UDS API.

- cJSON is vendored under `src/mpi_ctx/json/` and used by the context layer — avoid duplicating JSON libraries unless intentionally replacing it.

- Keep platform-specific code isolated under `src/mpi/` (e.g., `rockchip/`, `rkmedia/`, `stub`) so the core framework remains platform-agnostic.

---

## Files to consult when extending the project

- `src/Makefile` — build rules and targets (first place to check for build behavior).
- `Makefile` (repo root) — OpenWrt package integration.
- `test/test.py` — UDS client example and expected packet layout.
- `src/main.cpp`, `src/media/media_manager.*`, `src/stream/stream_base.*` — typical call flows and lifecycle.

---

If this file already exists, merge carefully: preserve any existing project-specific tips and add missing commands/architecture notes.

