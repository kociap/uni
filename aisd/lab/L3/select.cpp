#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

using i64 = long long;

template<typename Random_Access_Iterator, typename Predicate, typename Swap>
void insertion_sort(Random_Access_Iterator first, Random_Access_Iterator last, Predicate predicate, Swap swap) {
    if(first == last) {
        return;
    }

    for(Random_Access_Iterator i = first + 1; i != last; ++i) {
        for(Random_Access_Iterator j = i; j != first;) {
            Random_Access_Iterator prev = j;
            --prev;
            if(!predicate(*prev, *j)) {
                swap(*j, *prev);
                j = prev;
            } else {
                break;
            }
        }
    }
}

#define SELECT_PARITION_SIZE 9

template<typename Random_Access_Iterator, typename Predicate, typename Swap, typename Print>
Random_Access_Iterator select(Random_Access_Iterator first, Random_Access_Iterator last, i64 k, Predicate p, Swap swap, Print print) {
    i64 const initial_size = last - first;
    if(k < 1 || k > initial_size) {
        return last;
    }

    while(true) {
        i64 const size = last - first;
        if(size <= SELECT_PARITION_SIZE) {
            insertion_sort(first, last, p, swap);
            return first + k - 1;
        }
        // Find median.
        Random_Access_Iterator medians = first;
        for(Random_Access_Iterator left = first; true;) {
            i64 const remaining = last - left;
            Random_Access_Iterator right = left + (remaining < SELECT_PARITION_SIZE ? remaining : SELECT_PARITION_SIZE);
            insertion_sort(left, right, p, swap);
            swap(*medians, *(left + (right - left) / 2));
            ++medians;
            left = right;
            if(remaining <= SELECT_PARITION_SIZE) {
                break;
            }
        }
        Random_Access_Iterator median = select(first, medians, (medians - first) / 2, p, swap, print);
        // Partition.
        {
            // Move median to the end of the range.
            Random_Access_Iterator end = last - 1;
            swap(*median, *end);
            Random_Access_Iterator less = first;
            for(Random_Access_Iterator f = first; f != end; ++f) {
                if(p(*f, *end)) {
                    swap(*f, *less);
                    ++less;
                }
            }
            swap(*less, *end);
            median = less;
        }

        print();

        i64 const distance = median - first + 1;
        if(k == distance) {
            return median;
        } else if(k < distance) {
            last = median;
        } else {
            k -= distance;
            first = median + 1;
        }
    }
}

template<typename Random_Access_Iterator, typename Predicate, typename Swap, typename Print, typename Random_Generator>
Random_Access_Iterator randomized_select(Random_Access_Iterator first, Random_Access_Iterator last, i64 const k, Predicate p, Swap swap, Print print,
                                         Random_Generator& random) {
    i64 const size = last - first;
    if(k < 1 || k > size) {
        return last;
    }

    if(size <= 5) {
        insertion_sort(first, last, p, swap);
        return first + k - 1;
    }

    Random_Access_Iterator mid = first + random() % size;
    // Partition.
    {
        // Move pivot to the end of the range.
        Random_Access_Iterator end = last - 1;
        swap(*mid, *end);
        Random_Access_Iterator less = first;
        for(Random_Access_Iterator f = first; f != end; ++f) {
            if(p(*f, *end)) {
                swap(*f, *less);
                ++less;
            }
        }
        swap(*less, *end);
        mid = less;
    }

    print();

    i64 const distance = mid - first + 1;
    if(k == distance) {
        return mid;
    } else if(k < distance) {
        return randomized_select(first, mid, k, p, swap, print, random);
    } else {
        return randomized_select(mid + 1, last, k - distance, p, swap, print, random);
    }
}

void print(std::vector<i64> const& numbers) {
    for(i64 i = 0; i < numbers.size(); ++i) {
        if(i != 0) {
            std::cout << ' ';
        }
        std::cout << std::setw(2) << numbers[i];
    }
    std::cout << '\n';
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

    if(numbers.size() < 50) {
        // Print input array.
        print(numbers);
    }

    i64 compares = 0;
    i64 swaps = 0;
    std::random_device rd;
    std::mt19937 random_generator(rd());
    auto t1 = std::chrono::high_resolution_clock::now();
#if defined(RAND_SELECT)
    auto const result = randomized_select(
        numbers.begin(), numbers.end(), k,
        [&compares](i64 lhs, i64 rhs) {
            compares += 1;
            return lhs < rhs;
        },
        [&swaps](i64& lhs, i64& rhs) {
            swaps += 1;
            i64 tmp = rhs;
            rhs = lhs;
            lhs = tmp;
        },
        [&numbers]() {
            if(numbers.size() < 50) {
                print(numbers);
            }
        },
        random_generator);
#elif defined(STD_SELECT)
    auto const result = select(
        numbers.begin(), numbers.end(), k,
        [&compares](i64 lhs, i64 rhs) {
            compares += 1;
            return lhs < rhs;
        },
        [&swaps](i64& lhs, i64& rhs) {
            swaps += 1;
            i64 tmp = rhs;
            rhs = lhs;
            lhs = tmp;
        },
        [&numbers]() {
            if(numbers.size() < 50) {
                print(numbers);
            }
        });
#else
    #error "select not defined"
#endif

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

    if(numbers.size() < 50) {
        // Print sorted array.
        std::sort(numbers.begin(), numbers.end());
        print(numbers);
        std::cout << *result << ", ";
    }
    std::cout << duration << ", " << compares << ", " << swaps << "\n";

    return 0;
}
