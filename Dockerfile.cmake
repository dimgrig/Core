# syntax=docker/dockerfile:1
ARG BASE_DEPS_IMAGE=base-deps-linux:latest

FROM ${BASE_DEPS_IMAGE} AS build

ARG BUILD_TYPE=Release
ARG CMAKE_CMD=cmake
# Для Linux-образов: /opt/core, итог → /opt/core/<version>/...
# Для Windows-образа (кросс из Linux): /app/Core/applocal-staging,
#   итог → /app/Core/applocal-staging/<version>/...
ARG CORE_INSTALL_BASE=/opt/core

WORKDIR /app
COPY Core/ /app/Core/
WORKDIR /app/Core

# CORE_VERSION читается из VERSION,
# CMakeLists.txt повторно парсит тот же файл — версия гарантированно одна.
# install(TARGETS …) в cmake/CoreLib.cmake раскладывает
#   include/<lib>/…, lib/lib<lib>.so.<version>, lib/cmake/Core/*.cmake
RUN CORE_VERSION="$(tr -d '[:space:]' < VERSION)" && \
    echo "=== Core version: ${CORE_VERSION} ===" && \
    ${CMAKE_CMD} -S . -B build \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
        -DBUILD_TESTING=OFF \
        -DCORE_INSTALL_PREFIX="${CORE_INSTALL_BASE}/${CORE_VERSION}" \
        -DCMAKE_VERBOSE_MAKEFILE=OFF \
    && cmake --build build -j"$(nproc)" \
    && cmake --install build \
    && echo "=== Installed to ${CORE_INSTALL_BASE}/${CORE_VERSION} ===" \
    && find "${CORE_INSTALL_BASE}/${CORE_VERSION}" -maxdepth 2 -type d | sort