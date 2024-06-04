#!/bin/sh
for _ in $(seq 1 1 150); do
  #0 success ;1 overload
  ./build/examples/example 1 1 0
  sleep 0.5
done
