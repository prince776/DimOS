#!/bin/sh
set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

rm -f dimos.iso
rm -rf ${SCRIPT_DIR}/build
rm -rf ${SCRIPT_DIR}/isodir
rm -rf ${SCRIPT_DIR}/sysroot
