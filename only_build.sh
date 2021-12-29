#!/bin/bash
g++ -I /$(pwd)/libs -L /$(pwd)/libs -Wall -g -o audioParse_bin ./audioParse.cpp -lstdc++fs  -std=c++17  -pthread -lmatplot -laubio
