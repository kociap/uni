#include <chrono>
#include <iostream>
#include <string>
#include <vector>

using i64 = long long;

i64 max(i64 a, i64 b) {
    return a > b ? a : b;
}

struct Pair {
    i64 first;
    i64 second;
};

template<typename T>
struct Array_2D {
public:
    Array_2D(i64 x, i64 y): _width(x), _array(x, std::vector<T>(y)) {}

    T& operator()(i64 x, i64 y) {
        // return _array[x * _width + y];
        return _array[x][y];
    }

private:
    // std::vector<T> _array;
    std::vector<std::vector<T>> _array;
    i64 _width;
};

std::vector<Pair> compute_lcs(std::string const& a, std::string const& b) {
    enum Direction { DIR_UP, DIR_LEFT, DIR_DIAG };
    Array_2D<i64> length(a.size() + 1, b.size() + 1);
    Array_2D<Direction> direction(a.size() + 1, b.size() + 1);
    for(i64 i = 1; i <= a.size(); ++i) {
        for(i64 j = 1; j <= b.size(); ++j) {
            if(a[i - 1] == b[j - 1]) {
                length(i, j) = length(i - 1, j - 1) + 1;
                direction(i, j) = DIR_DIAG;
            } else if(length(i - 1, j) >= length(i, j - 1)) {
                length(i, j) = length(i - 1, j);
                direction(i, j) = DIR_UP;
            } else {
                length(i, j) = length(i, j - 1);
                direction(i, j) = DIR_LEFT;
            }
        }
    }

    i64 i = a.size() - 1;
    i64 j = b.size() - 1;
    std::vector<Pair> result;
    while(i != 0 && j != 0) {
        if(direction(i, j) == DIR_DIAG) {
            result.push_back(Pair{i - 1, j - 1});
        }

        switch(direction(i, j)) {
            case DIR_UP:
                i -= 1;
                break;
            case DIR_LEFT:
                j -= 1;
                break;
            case DIR_DIAG:
                i -= 1;
                j -= 1;
                break;
        }
    }
    std::reverse(result.begin(), result.end());
    return result;
}

void print_lcs(std::string const& a, std::string const& b, std::vector<Pair> const& indices) {
    std::cout << a << '\n';
    i64 offset = -1;
    for(Pair const& index: indices) {
        for(i64 d = index.first - offset - 1; d > 0; --d) {
            std::cout << ' ';
        }
        std::cout << a[index.first];
        offset = index.first;
    }
    std::cout << '\n' << b << '\n';
    offset = -1;
    for(Pair const& index: indices) {
        for(i64 d = index.second - offset - 1; d > 0; --d) {
            std::cout << ' ';
        }
        std::cout << b[index.second];
        offset = index.second;
    }
}

int main() {
    std::string a, b;
    std::cin >> a >> b;

    auto t1 = std::chrono::high_resolution_clock::now();
    std::vector<Pair> lcs = compute_lcs(a, b);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
    if(max(a.size(), b.size()) < 50) {
        print_lcs(a, b, lcs);
        std::cout << '\n';
    }
    std::cout << duration << '\n';

    return 0;
}
