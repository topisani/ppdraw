function(convert_fonts_bdf out_var)
  set(result)
  foreach(in_f ${ARGN})
    file(RELATIVE_PATH out_f ${CMAKE_CURRENT_SOURCE_DIR} ${in_f})
    set(out_f "${CMAKE_CURRENT_BINARY_DIR}/fonts/${out_f}.c")
    set(header_f "${CMAKE_CURRENT_BINARY_DIR}/fonts/include/ppdraw/fonts.h")
    add_custom_command(OUTPUT ${out_f}
      COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/bdf2c.py" "${in_f}" "${out_f}" "${header_f}"
      DEPENDS ${in_f} "${CMAKE_CURRENT_SOURCE_DIR}/bdf2c.py"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Converting bdf font to C ${in_f} -> ${out_f}"
      VERBATIM
      )
    list(APPEND result ${out_f})
  endforeach()
  set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

file(GLOB_RECURSE BDF_FILES ./*.bdf)
convert_fonts_bdf(font_c_files ${BDF_FILES})

add_library(ppdraw_fonts STATIC ${font_c_files})
target_include_directories(ppdraw_fonts PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/fonts/include)
