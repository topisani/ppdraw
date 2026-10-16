# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ppdraw is a C graphics library for embedded/small displays. It provides framebuffer-based drawing primitives (pixels, lines, rectangles, images, text), a font compilation pipeline (BDF→C), and a widget registry system using linker sections.

## Build Commands

```bash
cmake -B build
cmake --build build
```

The preview example can be run with:
```bash
./build/examples/live_preview/app_ui-ppdraw-preview              # List widgets
./build/examples/live_preview/app_ui-ppdraw-preview "widget_name" # Render widget
```

## Architecture

- **Core C library** (`src/draw.c`, `include/ppdraw/draw.h`): Framebuffer drawing with configurable bpp, clipping, and translation contexts
- **C++ wrapper** (`include/ppdraw/draw.hpp`): Class-based API using ETL's `string_view`; requires C++17+
- **Widget registry** (`src/registry.c`, `include/ppdraw/registry.h`): Widgets registered via `PPDRAW_REGISTER_WIDGET()` macro into `ppdraw_widget_registry` linker section for runtime discovery
- **Preview system** (`include/ppdraw/preview.h`, `tools/preview/`): `PPDRAW_REGISTER_PREVIEW()` stores preview widgets in `ppdraw_preview_registry` section. CMake `ppdraw_preview()` function creates preview executables that link widget libraries with `--whole-archive`
- **Font pipeline** (`src/fonts/`): `bdf2c.py` converts BDF font files to C arrays at build time via CMake custom commands

## Key Conventions

- C99+ for core library, C++17+ for wrapper
- CMake 3.21+ required
- ETL (Embedded Template Library) fetched via FetchContent
- Preview output format: `width=X\nheight=Y\nbpp=Z\ndata=<hex>`
- Python build tools (`bdf2c.py`) use only the standard library
