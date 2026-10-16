Graphics library for low resolution displays on MCUs.

## Features
- Basic shapes (lines, rectangles, circles)
- Text rendering
	- UTF-8/Unicode
	- Variable/mono pitch fonts.
	- Font C-file generation from BDF
	- Text space calculation.
- Backend independent rendering
- Arbitrary bits per pixel support for monochrome, gray scale, colors or fixed palette.
- Desktop Preview with live reload support.
- Optional C++ wrapper

## Limitations
- Assumes horizontal pixel layout. Some smaller monochrome displays use one byte to represent 8 vertical pixels. This could be implemented if needed.
- Performance: Currently always draws one pixel at a time, though this could be optimized using platform specific backends.

## Planned features
- [x] Zephyr integration module
- [ ] Missing rendering features
	- [ ] Circles
	- [ ] Circular arcs
	- [ ] Bézier curves.
- [ ] Live-preview on device by sending image over serial transport or similar
- [ ] Parameters/inputs in live preview
- [ ] Image embedding cmake support
- [x] Improved font selection cmake support

## Live Preview
Allows live-editing code with real time preview. Works by continuously rebuilding the graphics code for the host computer through cmake, and rendering the result in a desktop window.

### Demo
![[docs/demo.gif]]

## Zephyr

ppdraw is a Zephyr module. Add it to your `west.yml`:

```yaml
manifest:
  projects:
    - name: ppdraw
      url: https://github.com/topisani/ppdraw
      revision: main
      path: modules/lib/ppdraw
```

or point `ZEPHYR_EXTRA_MODULES` at a checkout. Then in `prj.conf`:

```
CONFIG_PPDRAW=y
CONFIG_PPDRAW_FONTS="spleen-5x8 tiny5"   # bundled fonts, default spleen-5x8
CONFIG_PPDRAW_WIDGET_REGISTRY=y          # optional
```

and `#include <ppdraw/draw.h>` / `<ppdraw/fonts.h>`. Font generation needs the Python packages in `scripts/requirements.txt`; `west packages pip --install` installs them. The C++ wrapper additionally needs ETL on the include path.

## Fonts

All bundled BDF fonts are compiled by default. To select a subset:

```cmake
set(PPDRAW_FONTS spleen-5x8 berry tiny5)
add_subdirectory(ppdraw)
target_link_libraries(myapp ppdraw ppdraw_fonts)
```

To compile your own BDF fonts:

```cmake
include(PpdrawFonts)
ppdraw_add_fonts(my_fonts path/to/custom.bdf path/to/another.bdf)
target_link_libraries(myapp my_fonts)
```

This creates a static library and generates a `my_fonts/my_fonts.h` header with extern declarations for each font. A custom header path can be specified with the `HEADER` option:

```cmake
ppdraw_add_fonts(my_fonts path/to/custom.bdf HEADER "${CMAKE_CURRENT_BINARY_DIR}/my_fonts.h")
```

## Usage

### 1. Split graphics code into its own cmake library target

The graphics/widget code must be in a separate static library target so it can be linked into the preview executable.

```cmake
add_library(app_ui STATIC widgets.c)
target_link_libraries(app_ui PUBLIC ppdraw)
```

### 2. Register preview widgets

In your C source files, use the `PPDRAW_REGISTER_PREVIEW` macro to register drawable widgets:

```c
#include <ppdraw/preview.h>

void my_widget_draw(draw_t ctx) {
    // Drawing code here
}

PPDRAW_REGISTER_PREVIEW(my_widget_draw, DRAW_VEC(128, 64));
```

### 3. Add the preview target

Call `ppdraw_preview()` in your CMakeLists.txt:

```cmake
ppdraw_preview(app_ui)
```

This creates two targets:
- `app_ui-ppdraw-preview` — the preview executable
- `app_ui-ppdraw-preview-run` — runs the interactive preview GUI

### 4. Run the preview

```bash
cmake --build build --target app_ui-ppdraw-preview-run
```

This opens a Tkinter window showing the rendered widget. Right-click to switch between registered widgets. The preview watches for changes and automatically rebuilds and reloads.
