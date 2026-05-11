#!/bin/bash

g++ -O3 -o solver solver.cpp
./solver ../challenge/possible_seeds.txt hits.txt $(nproc) 28 2>&1 | tee out
grep "BtSCTF" out
