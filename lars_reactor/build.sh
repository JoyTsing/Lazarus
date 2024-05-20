#!/bin/sh
echo "======Building!!!"
bear -- make clean && bear -- make -j12
echo "======liblreactor done!!!"
cd example/lars_reactor || echo "no such direction" || return
make clean && make -j12
echo "======example done!!!"
