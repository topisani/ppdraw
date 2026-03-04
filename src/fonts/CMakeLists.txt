include(PpdrawFonts)

file(GLOB_RECURSE BDF_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" ./*.bdf)

# Build default font list from BDF filenames (without extension)
set(all_font_names)
foreach(bdf ${BDF_FILES})
  get_filename_component(name "${bdf}" NAME_WE)
  list(APPEND all_font_names "${name}")
endforeach()
list(REMOVE_DUPLICATES all_font_names)

set(PPDRAW_FONTS "${all_font_names}" CACHE STRING "List of bundled fonts to compile")

# Resolve font names back to BDF file paths
set(selected_bdf_files)
foreach(font_name ${PPDRAW_FONTS})
  set(found FALSE)
  foreach(bdf ${BDF_FILES})
    get_filename_component(name "${bdf}" NAME_WE)
    if(name STREQUAL font_name)
      list(APPEND selected_bdf_files "${CMAKE_CURRENT_SOURCE_DIR}/${bdf}")
      set(found TRUE)
      break()
    endif()
  endforeach()
  if(NOT found)
    message(FATAL_ERROR "ppdraw: unknown font '${font_name}'. Available: ${all_font_names}")
  endif()
endforeach()

ppdraw_add_fonts(ppdraw_fonts ${selected_bdf_files}
  HEADER "${CMAKE_CURRENT_BINARY_DIR}/include/ppdraw/fonts.h"
  INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include"
)
