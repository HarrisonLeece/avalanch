#!/bin/bash
g++ -I /$(pwd)/libs -L /$(pwd)/libs -Wall -g -o audioFlagger.bin ./audioFlagger.cpp -lstdc++fs  -std=c++17  -pthread -lmatplot -laubio
