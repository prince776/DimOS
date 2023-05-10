#!/bin/sh
set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

DEFAULT_SYSROOT="${SCRIPT_DIR}/sysroot"
export SYSROOT=${SYSROOT:-${DEFAULT_SYSROOT}}

TOOLCHAIN="${SCRIPT_DIR}/x86_64-elf-toolchain.cmake"

BUILD_DIR="${SCRIPT_DIR}/build"

cmake . -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN} -DCMAKE_BUILD_TYPE=Debug -B ${BUILD_DIR}

cd ${BUILD_DIR}
make VERBOSE=1
cd ${SCRIPT_DIR}

mkdir ${SYSROOT}/boot
cp ${BUILD_DIR}/kernel/kernel ${SYSROOT}/boot/dimos.kernel

cp ${BUILD_DIR}/compile_commands.json ${SCRIPT_DIR}/compile_commands.json
