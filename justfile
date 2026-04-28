

default:
    just --list

[group('building')]
build:
    @echo "Building RobeRT..."
    cmake -G Ninja -S . -B build
    cmake --build build 

[group('building')]
clean:
    @echo "Cleaning build dir..."
    cmake --build build --target clean

[group('run')]
run:
    @echo "Running RobeRT..."
    @./build/RobeRT_server ./robots.conf
    @echo "RobeRT stopped."