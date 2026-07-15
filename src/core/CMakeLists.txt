add_library(core_lib STATIC
    data.cpp
)

target_include_directories(core_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)
