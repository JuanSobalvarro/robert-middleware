cmake_minimum_required(VERSION 3.28)
project(RobeRT_server VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

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

# download and available
FetchContent_MakeAvailable(zmq cppzmq protobuf)

find_package(Threads REQUIRED)

set(PROTO_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/protocol/protocol.proto"
)

include(${protobuf_SOURCE_DIR}/cmake/protobuf-generate.cmake)

set(SOURCES
    src/main.cpp
    src/robot.cpp
    src/server.cpp
    src/command.cpp
    src/decoder.cpp
    src/data.cpp
    src/utils.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# automatically handle generating the .pb.cc and .pb.h
protobuf_generate(
    TARGET ${PROJECT_NAME}
    PROTOS ${PROTO_FILE}
    LANGUAGE cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR} # where .pb are
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    libzmq-static
    cppzmq
    libprotobuf
    Threads::Threads
)
