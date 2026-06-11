#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LLVM_VERSION="${LLVM_VERSION:-20}"
CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Debug}"
IMAGE_NAME="${KRIOL_CONTAINER_IMAGE:-kriol:ubuntu-26.04-llvm${LLVM_VERSION}}"
OUT_DIR="${KRIOL_CONTAINER_OUT:-${ROOT_DIR}/build/container-ubuntu-26.04}"

if [ -n "${CONTAINER_ENGINE:-}" ]; then
    ENGINE="${CONTAINER_ENGINE}"
elif command -v podman >/dev/null 2>&1; then
    ENGINE="podman"
elif command -v docker >/dev/null 2>&1; then
    ENGINE="docker"
else
    echo "error: install podman or docker, or set CONTAINER_ENGINE" >&2
    exit 1
fi

echo "==> Building ${IMAGE_NAME} with ${ENGINE}"
"${ENGINE}" build \
    --build-arg LLVM_VERSION="${LLVM_VERSION}" \
    --build-arg CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
    -f "${ROOT_DIR}/Dockerfile.ubuntu-26.04" \
    -t "${IMAGE_NAME}" \
    "${ROOT_DIR}"

mkdir -p "${OUT_DIR}"

container_id="$("${ENGINE}" create "${IMAGE_NAME}")"
cleanup() {
    "${ENGINE}" rm -f "${container_id}" >/dev/null 2>&1 || true
}
trap cleanup EXIT

echo "==> Copying build artifacts to ${OUT_DIR}"
"${ENGINE}" cp "${container_id}:/src/kriol/build/resolute/kriol" "${OUT_DIR}/kriol"
"${ENGINE}" cp "${container_id}:/tmp/kriol-hello.wasm" "${OUT_DIR}/kriol-hello.wasm"
"${ENGINE}" cp "${container_id}:/src/kriol/build/resolute/CMakeCache.txt" "${OUT_DIR}/CMakeCache.txt"
"${ENGINE}" cp "${container_id}:/usr/lib/llvm-${LLVM_VERSION}/include/lld/Common/Driver.h" "${OUT_DIR}/lld-Driver.h"

echo "==> Done"
echo "    ${OUT_DIR}/kriol"
echo "    ${OUT_DIR}/kriol-hello.wasm"
echo "    ${OUT_DIR}/CMakeCache.txt"
echo "    ${OUT_DIR}/lld-Driver.h"
