# Store the directory where the function is defined
set(PPDRAW_PREVIEW_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

function(ppdraw_preview library_target)
    # find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(preview_target "${library_target}-ppdraw-preview")
    add_executable(${preview_target}
        EXCLUDE_FROM_ALL
        ${PPDRAW_PREVIEW_DIR}/preview.c
    )
    target_link_libraries(${preview_target} 
        PRIVATE -Wl,--whole-archive ${library_target} -Wl,--no-whole-archive ppdraw)

    # set_property(TARGET ${library_target} 
    #     PROPERTY POSITION_INDEPENDENT_CODE on)

    # set_property(TARGET ${shared_lib_name} 
    #     PROPERTY POSITION_INDEPENDENT_CODE on)

    # add_custom_target(${library_target}_ppdraw_preview
    #     COMMAND ${Python3_EXECUTABLE}
    #         ${PPDRAW_PREVIEW_DIR}/preview.py
    #         $<TARGET_FILE:${shared_lib_name}>
    #         --watch
    #         "${CMAKE_COMMAND}" "--build" "${CMAKE_BINARY_DIR}" "--target" "${shared_lib_name}"
    #     DEPENDS ${shared_lib_name}
    #     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    #     COMMENT "Running ppdraw preview for ${library_target}"
    #     VERBATIM
    #     USES_TERMINAL
    # )
endfunction()
