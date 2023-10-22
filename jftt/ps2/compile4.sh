#!/bin/bash

FLAGS="-ggdb -O0"

clang++ -g -O0 -c -o ex4.o ex4.cpp
# ar r libex4 ex4.o

flex ex4.flex
clang -g -O0 -c -o lex.o lex.yy.c

clang++ -g -O0 -o calculator lex.o ex4.o
