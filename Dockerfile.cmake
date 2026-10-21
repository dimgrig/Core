ARG BASE_DEPS_IMAGE=base-deps-linux:latest
ARG BUILD_TYPE=Release
ARG CMAKE_CMD=cmake

# ============================================================
# Build stage
# ============================================================
FROM ${BASE_DEPS_IMAGE} AS build
ARG BUILD_TYPE
ARG CMAKE_CMD

ENV CORE_LINUX_LIBS_PATH=/usr/lib/qt
RUN mkdir -p /usr/lib/qt && chmod 777 /usr/lib/qt

WORKDIR /app
COPY Core/ /app/Core/
WORKDIR /app/Core

RUN ${CMAKE_CMD} -S . -B build \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
        -DBUILD_TESTING=OFF \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_VERBOSE_MAKEFILE=OFF \
    && cmake --build build -j"$(nproc)" \
    && cmake --install build

# ============================================================
# Test stage (опционально)
# ============================================================
#FROM build AS test
#WORKDIR /app/Core
#RUN cmake -S . -B build-debug \
#        -DCMAKE_BUILD_TYPE=Debug \
#        -DBUILD_TESTING=ON \
#    && cmake --build build-debug -j"$(nproc)" --target core-tests \
#    && cd build-debug && ctest --output-on-failure -V
