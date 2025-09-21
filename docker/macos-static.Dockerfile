FROM ubuntu:24.04

ARG ZIG_VERSION=0.12.0
ARG ZIG_ARCH=linux-x86_64

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       ca-certificates \
       curl \
       build-essential \
       cmake \
       ninja-build \
       git \
       pkg-config \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /opt/zig \
    && curl -L -o /tmp/zig.tar.xz https://ziglang.org/download/${ZIG_VERSION}/zig-${ZIG_ARCH}-${ZIG_VERSION}.tar.xz \
    && tar -xJf /tmp/zig.tar.xz -C /opt/zig --strip-components=1 \
    && rm /tmp/zig.tar.xz

ENV PATH="/opt/zig:${PATH}"

WORKDIR /src
COPY . /src

RUN cmake -S . -B build-macos -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/zig-macos-static.cmake \
        -DSTATICCOMPILE=ON \
        -DZIG_EXECUTABLE=/opt/zig/zig

RUN cmake --build build-macos
