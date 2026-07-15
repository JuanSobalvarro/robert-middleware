add_library(commands_lib STATIC
    commands.cpp
    decoder.cpp
)

target_include_directories(commands_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(commands_lib PUBLIC
    protocol_lib
)
