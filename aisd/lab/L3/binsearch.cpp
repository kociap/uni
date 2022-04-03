#include <chrono>
#include <iostream>
#include <vector>

using i64 = long long;

template<typename Random_Access_Iterator, typename T, typename Predicate>
bool binary_search(Random_Access_Iterator first, Random_Access_Iterator last, T const& value, Predicate p) {
    auto const size = last - first;
    // Linear search for at most 5 elements.
    if(size <= 5) {
        for(; first != last; ++first) {
            if(!p(*first, value) && !p(value, *first)) {
                return true;
            }
        }
        return false;
    }

    Random_Access_Iterator mid = first + size / 2;
    if(p(value, *mid)) {
        return binary_search(first, mid, value, p);
    } else if(p(*mid, value)) {
        return binary_search(mid + 1, last, value, p);
    } else {
        return true;
    }
}

int main() {
    std::ios::sync_with_stdio(false);

    i64 k;
    std::cin >> k;

    std::vector<i64> numbers;
    numbers.reserve(64);
    i64 a;
    while(std::cin >> a) {
        numbers.push_back(a);
    }

    i64 compares = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    bool const result = binary_search(numbers.begin(), numbers.end(), k, [&compares](i64 lhs, i64 rhs) {
        compares += 1;
        return lhs < rhs;
    });
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
    std::cout << duration << ',' << compares << "\n";
    // std::cout << result << '\n';
    return 0;
}
