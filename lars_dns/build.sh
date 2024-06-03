#!/bin/bash
if [ -d ./build ]; then
  rm build -r
fi
cmake -B build -GNinja && cmake --build build -j 12
