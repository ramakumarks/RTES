#!/bin/sh
rm -rf build
mkdir build && cd build
cmake -DBOARD=galileo ..
make
