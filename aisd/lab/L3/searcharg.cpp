#include <iostream>
#include <random>
#include <stdlib.h>

using i64 = long long;

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    i64 const count = atoll(argv[1]);
    std::cout << count;
    i64 a;
    while(std::cin >> a) {
        std::cout << ' ' << a;
    }
    std::cout << '\n';
    return 0;
}