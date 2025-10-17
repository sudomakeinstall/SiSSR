# Multi-stage build for dv-sissr
FROM ubuntu:24.04 as builder

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    libboost-program-options-dev \
    liblapacke-dev \
    libsuitesparse-dev \
    libeigen3-dev \
    libgoogle-glog-dev \
    libgflags-dev \
    libceres-dev \
    rapidjson-dev \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /build

# Build ITK from source (required for specific modules)
RUN wget https://github.com/InsightSoftwareConsortium/ITK/releases/download/v5.4.4/InsightToolkit-5.4.4.tar.gz && \
    tar -xzf InsightToolkit-5.4.4.tar.gz && \
    mkdir ITK-build && \
    cd ITK-build && \
    cmake -GNinja \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING=OFF \
        -DBUILD_EXAMPLES=OFF \
        -DModule_SubdivisionQuadEdgeMeshFilter=ON \
        -DModule_IOMeshSTL=ON \
        -DITK_USE_SYSTEM_VXL=OFF \
        -DITK_USE_SYSTEM_EIGEN=ON \
        -DModule_ITKVNL=ON \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        ../InsightToolkit-5.4.4 && \
    ninja && \
    ninja install && \
    cd .. && \
    rm -rf InsightToolkit-5.4.4* ITK-build


# Copy source code
COPY . /build/dv-sissr

# Build dv-sissr
WORKDIR /build/dv-sissr
RUN mkdir build && \
    cd build && \
    cmake -GNinja \
        -DCMAKE_BUILD_TYPE=Release \
        -DITK_DIR=/usr/local/lib/cmake/ITK-5.4 \
        .. && \
    ninja dv-sissr

# Runtime stage
FROM ubuntu:24.04 as runtime

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libboost-program-options1.83.0 \
    liblapacke \
    libsuitesparseconfig7 \
    libeigen3-dev \
    libgoogle-glog0v6t64 \
    libgflags2.2 \
    libceres4t64 \
    && rm -rf /var/lib/apt/lists/*

# Copy the built executable and required libraries
COPY --from=builder /build/dv-sissr/build/dv-sissr /usr/local/bin/
COPY --from=builder /usr/local/lib/ /usr/local/lib/

# Set up library path
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Create a non-root user
RUN useradd -m -u 1001 sissr
USER sissr

# Set working directory for data
WORKDIR /data

# Default entrypoint
ENTRYPOINT ["/usr/local/bin/dv-sissr"]
CMD ["--help"]
