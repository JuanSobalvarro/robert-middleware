add_library(server_lib STATIC
    server.cpp
    sock_comm.cpp
    tasker.cpp
    session.cpp
    request_handler.cpp
)

target_include_directories(server_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(server_lib PUBLIC
    libzmq-static
    cppzmq-static
    core_lib
    commands_lib
    protocol_lib
    Threads::Threads
)

IF (WIN32)
    target_link_libraries(server_lib PRIVATE ws2_32 iphlpapi)
ENDIF()
