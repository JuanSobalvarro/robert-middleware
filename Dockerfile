FROM debian:13-slim AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    ninja-build \
    clang \
    ccache \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN --mount=type=cache,target=/app/build \
    --mount=type=cache,target=/root/.cache/ccache \
    cmake -G Ninja -S . -B build \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache && \
    cmake --build build && \
    cp build/robert_server /tmp/robert_server

FROM debian:13-slim

WORKDIR /app

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /tmp/robert_server .

EXPOSE 42069

ENTRYPOINT [ "./robert_server", "config/config.yaml"]
