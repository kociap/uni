#include <iostream>
#include <random>
#include <stdlib.h>

using i64 = long long;

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    i64 const count = atoll(argv[1]);
    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<i64> d(0, 2 * count - 1);
    for(i64 i = 0; i < count; ++i) {
        if(i != 0) {
            std::cout << ' ';
        }
        std::cout << d(g);
    }
    std::cout << '\n';
    return 0;
}
