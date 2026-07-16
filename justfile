

default:
    just --list

[group('building')]
build:
    @echo "Building RobeRT..."
    cmake -G Ninja -S . -B build -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
    cmake --build build

[group('building')]
clean:
    @echo "Cleaning build dir..."
    rm -rf build

[group('run')]
run:
    @echo "Running RobeRT..."
    @./build/robert_server.exe ./robots.conf ./users.conf
    @echo "RobeRT stopped."

build_run:
    just build
    just run
