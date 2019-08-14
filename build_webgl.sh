#!/bin/sh

emcc --std=c++17 -O3 -s WASM=1 -s USE_GLFW=3 -s USE_WEBGL2=1 ./FillRate_attachment0_webgl.cpp -o attachment0.html
emcc --std=c++17 -O3 -s WASM=1 -s USE_GLFW=3 -s USE_WEBGL2=1 ./FillRate_attachment1_webgl.cpp -o attachment1.html
