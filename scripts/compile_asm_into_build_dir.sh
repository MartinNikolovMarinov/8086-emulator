#!/bin/bash

mkdir -p build
mkdir -p build/my_examples

pushd data > /dev/null

# compile all asm files into object files:
for f in ./*.asm; do
    nasm "$f" -o ../build/"${f%}.o"
    nasm "$f" -o "${f%}.o" # Create in the same directory as well
done

pushd my_examples > /dev/null

# compile all asm files into object files:
for f in ./*.asm; do
    nasm "$f" -o ../../build/my_examples/"${f%}.o"
    nasm "$f" -o "${f%}.o" # Create in the same directory as well
done

popd > /dev/null

popd > /dev/null
