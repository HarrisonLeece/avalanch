#!/bin/bash
g++ -I /$(pwd)/../libs -L /$(pwd)/../libs -g -o audioParse.bin ./audioParse.cpp -lstdc++fs  -std=c++17  -pthread -lmatplot -laubio
./audioParse.bin
