#!/usr/bin/env bash

sudo apt-get install check ninja-build doxygen
cp contrib/examples/example_app/lwipcfg.h.ci contrib/examples/example_app/lwipcfg.h
make -C contrib/ports/unix/check
mkdir build && cd build && cmake .. -G Ninja && cmake --build .
