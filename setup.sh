#!/usr/bin/env bash
emsdk/emsdk update
emsdk/emsdk install latest
emsdk/emsdk activate latest

source emsdk/emsdk_env.sh

rm -rf build

mkdir build

cd build/

emcmake cmake ..

intercept-build make
