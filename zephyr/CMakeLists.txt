if(CONFIG_PPDRAW)
  get_filename_component(PPDRAW_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

  zephyr_include_directories(${PPDRAW_DIR}/include)

  zephyr_library_named(ppdraw)
  zephyr_library_sources(${PPDRAW_DIR}/src/draw.c)

  if(CONFIG_PPDRAW_WIDGET_REGISTRY)
    zephyr_library_sources(${PPDRAW_DIR}/src/registry.c)
    zephyr_linker_sources(RODATA ppdraw-registry.ld)
  endif()

  string(STRIP "${CONFIG_PPDRAW_FONTS}" _fonts)
  if(_fonts)
    separate_arguments(PPDRAW_FONTS UNIX_COMMAND "${_fonts}")
    set(PPDRAW_PYTHON "${PYTHON_EXECUTABLE}")
    list(APPEND CMAKE_MODULE_PATH ${PPDRAW_DIR}/cmake)
    add_subdirectory(${PPDRAW_DIR}/src/fonts ${CMAKE_CURRENT_BINARY_DIR}/fonts)
    zephyr_include_directories(${CMAKE_CURRENT_BINARY_DIR}/fonts/include)
    zephyr_library_link_libraries(ppdraw_fonts)
  endif()
endif()
