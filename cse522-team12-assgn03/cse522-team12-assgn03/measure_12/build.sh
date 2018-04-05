#!/bin/sh
mkdir build && cd build
cmake -GNinja -DBOARD=galileo ..
ninja
