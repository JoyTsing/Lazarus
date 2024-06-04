#!/bin/sh

if [ $# -ne 1 ]; then
  echo "./example_run.sh return_state[0:suc,1:overload]"
  exit
fi

for _ in $(seq 1 1 150); do
  #0 success ;1 overload
  ./build/examples/example 1 1 "$1"
  sleep 0.5
done
