#!/usr/bin/env bash
source ./emsdk/emsdk_env.sh

mkdir build/

cd build/

git init

git remote add origin https://github.com/JsMarq96/JsMarq96.github.io.git

emcmake cmake ..

make

rm ../../JsMarq96.github.io/*
cp index* ../../JsMarq96.github.io/
cd ../../JsMarq96.github.io/
git add *
git commit -m "Deploy"
git push --set-upstream origin master
