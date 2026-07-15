cmake_minimum_required(VERSION 3.28...4.0)
project(robert_server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# iso c++20, i think it was ISO/IEC 14882:2020
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions(ZMQ_STATIC)

IF (POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
ENDIF()
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(FetchContent)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum required version of CMake" FORCE)

FetchContent_Declare(
    zmq
    GIT_REPOSITORY https://github.com/zeromq/libzmq.git
    GIT_TAG        v4.3.5
)

# c++ headers for libzmq
FetchContent_Declare(
    cppzmq
    GIT_REPOSITORY https://github.com/zeromq/cppzmq.git
    GIT_TAG        v4.11.0
)

FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG        v34.1
)

set(ZMQ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(WITH_PERF_TOOL OFF CACHE BOOL "" FORCE)
set(ENABLE_CPACK OFF CACHE BOOL "" FORCE)
set(CPPZMQ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)

set(BUILD_SHARED OFF CACHE BOOL "Disable shared libzmq" FORCE)
set(BUILD_STATIC ON CACHE BOOL "Enable static libzmq" FORCE)

# download and available
FetchContent_MakeAvailable(zmq cppzmq protobuf)
find_package(Threads REQUIRED)

set(PROTO_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/protocol/protocol.proto"
)
include(${protobuf_SOURCE_DIR}/cmake/protobuf-generate.cmake)

# protocol library
add_library(protocol_lib STATIC
    ${PROTO_FILE}
)
protobuf_generate(
    TARGET protocol_lib
    PROTOS ${PROTO_FILE}
    LANGUAGE cpp
)

target_include_directories(protocol_lib PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(protocol_lib PUBLIC
    libprotobuf
)

add_subdirectory(src/core)
add_subdirectory(src/commands)
add_subdirectory(src/robot)
add_subdirectory(src/server)

set(MAIN_SOURCES
    src/main.cpp
    src/utils.cpp
)

add_executable(${PROJECT_NAME} ${MAIN_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    server_lib
    protocol_lib
    robot_lib
)
