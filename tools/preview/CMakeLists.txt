# Store the directory where the function is defined
set(PPDRAW_PREVIEW_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

find_package(Python3 REQUIRED COMPONENTS Interpreter)

function(ppdraw_preview library_target)
    set(preview_target "${library_target}-ppdraw-preview")
    add_executable(${preview_target}
        EXCLUDE_FROM_ALL
        ${PPDRAW_PREVIEW_DIR}/preview.c
    )
    target_link_libraries(${preview_target}
        PRIVATE -Wl,--whole-archive ${library_target} -Wl,--no-whole-archive ppdraw)

    add_custom_target(${preview_target}-run
        COMMAND ${Python3_EXECUTABLE}
            ${PPDRAW_PREVIEW_DIR}/preview.py
            --preview-exec $<TARGET_FILE:${preview_target}>
            --build ${CMAKE_BINARY_DIR}
            --target ${preview_target}
            --watch
        COMMENT "Running ppdraw preview for ${library_target}"
        VERBATIM
        USES_TERMINAL
    )
endfunction()
