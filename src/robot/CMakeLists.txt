add_library(robot_lib STATIC
    robot.cpp
)

target_include_directories(robot_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)
