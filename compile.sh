#!/bin/bash
# This script compiles the main.cpp file into an executable named 'filesystem'
# It uses the g++ compiler with the C++11 standard.

echo "Compiling main.cpp..."
g++ main.cpp -o filesystem -std=c++11 -Wall
echo "Compilation finished. Run with ./filesystem"
