#!/usr/bin/env bash
source ./emsdk/emsdk_env.sh

mkdir build/

cd build/

git init

git remote add origin https://github.com/JsMarq96/JsMarq96.github.io.git

emcmake cmake ..

make

git add index.* resources/*
git commit -m "Deploy"
git push --set-upstream origin master
