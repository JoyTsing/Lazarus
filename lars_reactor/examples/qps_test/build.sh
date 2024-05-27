#!/bin/sh
if [ -d ./build ]; then
  rm build -r
fi
cmake -B build && cmake --build build -j 12
