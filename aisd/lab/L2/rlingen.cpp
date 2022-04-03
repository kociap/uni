#include <iostream>
#include <random>
#include <stdlib.h>

using i64 = long long;

int main(int argc, char** argv) {
    i64 const count = atoll(argv[1]);
    std::random_device g;
    for(i64 i = 0; i < count; ++i) {
        if(i != 0) {
            std::cout << ' ';
        }
        std::cout << (count - i - 1);
    }
    std::cout << '\n';
    return 0;
}
