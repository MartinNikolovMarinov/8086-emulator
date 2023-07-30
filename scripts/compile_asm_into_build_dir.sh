#!/bin/bash

# compile all asm files into object files:
for f in data/*.asm; do
    nasm "$f" -o "${f%}.o"
done

# move all object files to build directory:
for f in data/*.o; do
    mv "$f" build/
done
