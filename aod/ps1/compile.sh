#!/bin/bash
build=$1
if [[ -z "$build" ]]; then
    build="release"
fi

if [[ "$1" -eq "debug" ]]; then
    g++ -Werror -Wall -Wextra --pedantic -std=c++20 -DENABLE_HOTSPOT_ERRORS=1 -g3 -I./ -o main_e1 e1.cpp
    g++ -Werror -Wall -Wextra --pedantic -std=c++20 -DENABLE_HOTSPOT_ERRORS=1 -g3 -I./ -o main_e2 e2.cpp
    g++ -Werror -Wall -Wextra --pedantic -std=c++20 -DENABLE_HOTSPOT_ERRORS=1 -g3 -I./ -o main_e3 e3.cpp
    g++ -Werror -Wall -Wextra --pedantic -std=c++20 -DENABLE_HOTSPOT_ERRORS=1 -g3 -I./ -o main_e4 e4.cpp
else
    g++ -Werror -Wall -Wextra --pedantic -std=c++20 -O3 -g3 -I./ -o main_e1 e1.cpp
    g++ -Werror -Wall -Wextra --pedantic -std=c++20 -O3 -g3 -I./ -o main_e2 e2.cpp
    g++ -Werror -Wall -Wextra --pedantic -std=c++20 -O3 -g3 -I./ -o main_e3 e3.cpp
    g++ -Werror -Wall -Wextra --pedantic -std=c++20 -O3 -g3 -I./ -o main_e4 e4.cpp
fi
