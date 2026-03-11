cmake_minimum_required(VERSION 3.16)
project(RobeRT_server VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
find_package(Threads REQUIRED)

set(SOURCES
    src/main.cpp
    src/robot.cpp
    src/server.cpp
    src/command.cpp
    src/parser.cpp
    src/target.cpp
    src/utils.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    zmq
    Threads::Threads
)
