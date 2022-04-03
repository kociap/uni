#include <iostream>
#include <random>
#include <stdlib.h>
#include <vector>

using i64 = long long;

template<typename Random_Access_Iterator>
void fill_qsort_worst_case(Random_Access_Iterator first, Random_Access_Iterator last, i64 fill) {
    i64 const size = last - first;
    if(size < 6) {
        for(; first != last; ++first) {
            *first = fill;
        }
    } else {
        Random_Access_Iterator pivot = first + size / 2;
        *pivot = fill;
        fill_qsort_worst_case(first, pivot, fill + 1);
        fill_qsort_worst_case(pivot + 1, last, fill + 1);
    }
}

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    i64 const count = atoll(argv[1]);
    std::vector<i64> numbers(count, 0);
    fill_qsort_worst_case(numbers.begin(), numbers.end(), 0);
    for(i64 i = 0; i < count; i += 1) {
        if(i != 0) {
            std::cout << ' ';
        }
        std::cout << numbers[i];
    }
    std::cout << '\n';
    return 0;
}