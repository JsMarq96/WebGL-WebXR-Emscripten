#!/usr/bin/env bash

source ./emsdk/emsdk_env.sh

./build.sh

cd build

openssl req -new -x509 -keyout key.pem -out server.pem -days 365 -nodes

python3 ../server.py
