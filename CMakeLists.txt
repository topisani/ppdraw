project(ppdraw)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_library(ppdraw
    src/registry.c
    src/draw.c
)

set_property(TARGET ppdraw  PROPERTY POSITION_INDEPENDENT_CODE on)

target_include_directories(ppdraw PUBLIC include PRIVATE src)

add_subdirectory(src/fonts)
add_subdirectory(tools/preview)

# TODO: Only enable if enabling C++
Include(FetchContent)
FetchContent_Declare(
  etl
  GIT_REPOSITORY https://github.com/ETLCPP/etl
  GIT_TAG        20.44.2
)
FetchContent_MakeAvailable(etl)

target_link_libraries(ppdraw PUBLIC etl::etl)
