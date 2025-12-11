cmake_minimum_required(VERSION 3.21)

project(app)

# Put UI code in a separate library to be rebuilt
add_library(${PROJECT_NAME}_ui STATIC ./main.c)
# add_library(${PROJECT_NAME}_ui STATIC ./main.cpp)

# Link to ppdraw
add_subdirectory(../.. ${CMAKE_CURRENT_BINARY_DIR}/ppdraw)
target_link_libraries(app_ui PUBLIC ppdraw ppdraw_fonts)

# Register app_ui_ppdraw_preview target
ppdraw_preview(${PROJECT_NAME}_ui)
