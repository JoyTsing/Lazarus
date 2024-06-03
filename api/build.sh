#!/bin/bash
if [ -d ./build ]; then
  rm build -r
fi
cmake -B build -GNinja && cmake --build build -j 12
# sometimes not build proto first, so we just need to build again
# it just works
cmake --build build -j 12
