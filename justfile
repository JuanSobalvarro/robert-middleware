

default:
    just --list

[group('building')]
build:
    @echo "Building RobeRT..."
    cmake -G Ninja -S . -B build -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
    cmake --build build

[group('building')]
docker_build:
    @echo "Building Docker image..."
    docker build -t robert-middleware:1.1 .

[group('building')]
clean:
    @echo "Cleaning build dir..."
    rm -rf build

[group('run')]
run:
    @echo "Running RobeRT..."
    @./build/robert_server.exe ./config/config.yaml
    @echo "RobeRT stopped."

[group('run')]
docker_create:
    @echo "Cleaning, creating and running Docker container..."
    -just docker_clean
    docker run --name robert-middleware -p 42069:42069 -v "{{justfile_directory()}}/config/config.yaml:/app/config/config.yaml" robert-middleware:latest

[group('run')]
docker_run:
    @echo "Starting Docker container..."
    docker start robert-middleware

[group('clean')]
docker_clean:
    @echo "Stopping and removing Docker container..."
    -docker stop robert-middleware
    -docker rm robert-middleware

build_run:
    just build
    just run
