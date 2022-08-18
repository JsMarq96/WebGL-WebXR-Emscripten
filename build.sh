#!/usr/bin/env bash
source ./emsdk/emsdk_env.sh

mkdir build/

cd build/

emcmake cmake ..

make
