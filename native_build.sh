#!/usr/bin/env sh

mkdir build/

cd build

cmake ..

intercept-build make
