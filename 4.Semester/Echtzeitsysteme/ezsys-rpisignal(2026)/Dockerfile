# Use an ARM64 Ubuntu base image
FROM arm64v8/ubuntu:24.04 AS build

# Set environment variables to non-interactive (useful for automated builds)
ENV DEBIAN_FRONTEND=noninteractive

# Update and install build dependencies
# Note: Ubuntu 24.04's libgpiod-dev package only ships v1.6.3, so libgpiod
# is built from source below to get the current v2 API.
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    meson \
    ninja-build \
    pkg-config \
    git \
    && rm -rf /var/lib/apt/lists/*

# Build and install libgpiod (current release) from source
RUN git clone --branch v2.3 --depth 1 https://github.com/brgl/libgpiod.git /tmp/libgpiod && \
    meson setup /tmp/libgpiod/build /tmp/libgpiod \
        -Dtools=disabled -Dtests=disabled -Dexamples=disabled \
        -Dbindings-cxx=disabled -Dbindings-glib=disabled -Ddbus=disabled && \
    ninja -C /tmp/libgpiod/build install && \
    ldconfig && \
    rm -rf /tmp/libgpiod

# Set working directory
WORKDIR /app

# Copy your source code into the container
COPY . .

# Configure and build your project
#RUN cmake -DCMAKE_BUILD_TYPE=Debug . && make
RUN cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} . && make

# Stage 2: Export the final binary
FROM scratch AS export-stage
# Copy the final binary from the build stage.
# Adjust the path and binary name accordingly.
COPY --from=build /app/RPISignal /RPISignal