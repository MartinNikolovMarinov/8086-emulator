#!/bin/bash

function cleanup() {
    echo "Cleaning up..."
    rm build/test_01_from_01_to_05.asm
    rm build/test_01_from_01_to_05.asm.o
}

function check_exit_code() {
    if [ $? -ne 0 ]; then
        echo "[FAILED]"
        exit 1
    fi
}

exec_quiet() {
    eval $@ > /dev/null 2> /dev/null
    check_exit_code
}

exec_quiet_no_check() {
    eval $@ > /dev/null 2> /dev/null
}

trap cleanup EXIT

echo "Building..."

exec_quiet mkdir -p build

exec_quiet pushd build
exec_quiet cmake .. -DCMAKE_BUILD_TYPE=Debug
exec_quiet make -j 8

# Compile nasm test files:
exec_quiet nasm ../data/test_01_from_01_to_05.asm -o test_01_from_01_to_05.asm.o

echo "Running..."

./main -f ./test_01_from_01_to_05.asm.o > test_01_from_01_to_05.asm
check_exit_code

exec_quiet popd

echo "Diffing..."

command diff -Z build/test_01_from_01_to_05.asm data/test_01_from_01_to_05.asm
check_exit_code

echo "[OK] Found no differences!"
