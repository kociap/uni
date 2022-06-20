#include <iostream>
#include <random>
#include <stdlib.h>

using i64 = long long;

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    i64 const count = atoll(argv[1]);
    i64 increment = 1;
    if(argc == 3) {
        increment = atoll(argv[2]);
    }
    for(i64 i = 0, c = 0; c < count; c += 1, i += increment) {
        if(i != 0) {
            std::cout << ' ';
        }
        std::cout << i;
    }
    std::cout << '\n';
    return 0;
}