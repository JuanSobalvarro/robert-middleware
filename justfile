

default:
    just --list

[group('building')]
build:
    echo "Building RobeRT..."
    cmake -S . -B build
    cmake --build build --config Release

[group('run')]
run:
    echo "Running RobeRT..."
    ./build/Release/RobeRT_server
    echo "RobeRT stopped."
