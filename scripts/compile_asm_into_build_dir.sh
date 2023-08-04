#!/bin/bash

mkdir -p build

pushd data > /dev/null

# compile all asm files into object files:
for f in ./*.asm; do
    nasm "$f" -o ../build/"${f%}.o"
done

popd > /dev/null
