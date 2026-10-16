set(PPDRAW_BDF2C_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/../src/fonts/bdf2c.py"
    CACHE FILEPATH "Path to bdf2c.py font conversion script")
set(_PPDRAW_FONTS_DIR "${CMAKE_CURRENT_LIST_DIR}/../src/fonts"
    CACHE INTERNAL "Path to ppdraw font.h directory")

# PPDRAW_PYTHON: Python interpreter with scripts/requirements.txt installed.
# Defaults to the Python3 found by CMake.
if(NOT PPDRAW_PYTHON)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  set(PPDRAW_PYTHON "${Python3_EXECUTABLE}")
endif()

# ppdraw_add_fonts(<target> <bdf_files...> [HEADER <path>] [INCLUDE_DIR <dir>])
#
# Creates a static library <target> from the given BDF font files.
# Generates a header with extern declarations for each font.
# HEADER defaults to <target>/<target>.h in CMAKE_CURRENT_BINARY_DIR.
# INCLUDE_DIR defaults to the directory containing the header.
function(ppdraw_add_fonts target)
  cmake_parse_arguments(ARG "" "HEADER;INCLUDE_DIR" "" ${ARGN})
  set(bdf_files ${ARG_UNPARSED_ARGUMENTS})

  if(ARG_HEADER)
    set(header_file "${ARG_HEADER}")
  else()
    set(header_file "${CMAKE_CURRENT_BINARY_DIR}/${target}/${target}.h")
  endif()

  if(ARG_INCLUDE_DIR)
    set(header_dir "${ARG_INCLUDE_DIR}")
  else()
    get_filename_component(header_dir "${header_file}" DIRECTORY)
  endif()

  # Clear the header file so bdf2c.py regenerates it
  file(WRITE "${header_file}" "")

  set(c_files)
  foreach(bdf ${bdf_files})
    get_filename_component(bdf_abs "${bdf}" ABSOLUTE)
    get_filename_component(bdf_name "${bdf}" NAME_WE)
    set(c_file "${CMAKE_CURRENT_BINARY_DIR}/${target}/src/${bdf_name}.bdf.c")

    add_custom_command(OUTPUT "${c_file}"
      COMMAND "${PPDRAW_PYTHON}" "${PPDRAW_BDF2C_SCRIPT}" "${bdf_abs}" "${c_file}" "${header_file}"
      DEPENDS "${bdf_abs}" "${PPDRAW_BDF2C_SCRIPT}"
      COMMENT "Converting BDF font ${bdf_name}"
      VERBATIM
    )
    list(APPEND c_files "${c_file}")
  endforeach()

  add_library(${target} STATIC ${c_files})
  target_include_directories(${target}
    PRIVATE "${_PPDRAW_FONTS_DIR}"
    PUBLIC "${header_dir}"
  )
endfunction()
