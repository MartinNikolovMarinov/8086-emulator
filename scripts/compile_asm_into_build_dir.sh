#!/bin/bash

mkdir -p build
mkdir -p build/my_examples

pushd data > /dev/null

# compile all asm files into object files:
for f in ./*.asm; do
    nasm "$f" -o ../build/"${f%}.o"
done

pushd my_examples > /dev/null

# compile all asm files into object files:
for f in ./*.asm; do
    nasm "$f" -o ../../build/my_examples/"${f%}.o"
done

popd > /dev/null

popd > /dev/null
