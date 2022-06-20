#include <chrono>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <utility>
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

#define SELECT_PARITION_SIZE 5

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

template<typename Random_Access_Iterator, typename Predicate, typename Swap, typename Print>
void quick_sort(Random_Access_Iterator first, Random_Access_Iterator last, Predicate predicate, Swap swap, Print print) {
    auto const size = last - first;
    if(size < 2) {
        return;
    }

    if(size < 6) {
        insertion_sort(first, last, predicate, swap);
        return;
    }

#if defined(USE_SELECT)
    Random_Access_Iterator pivot = select(first, last, size / 2, predicate, swap, print);
#else
    Random_Access_Iterator pivot = first + size / 2;
#endif

    Random_Access_Iterator i = first;
    Random_Access_Iterator j = last - 1;
    while(true) {
        while(predicate(*i, *pivot)) {
            ++i;
        }

        while(predicate(*pivot, *j)) {
            --j;
        }

        if(i < j) {
            swap(*i, *j);
            if(i == pivot) {
                pivot = j;
            } else if(j == pivot) {
                pivot = i;
            }
            ++i;
            --j;
        } else {
            break;
        }
    }

    print();

    quick_sort(first, j + 1, predicate, swap, print);
    quick_sort(j + 1, last, predicate, swap, print);
}

template<typename Random_Access_Iterator, typename Predicate, typename Swap, typename Print>
void dual_pivot_quick_sort(Random_Access_Iterator first, Random_Access_Iterator last, Predicate predicate, Swap swap, Print print) {
    auto const size = last - first;
    if(size < 2) {
        return;
    }

    if(size < 6) {
        insertion_sort(first, last, predicate, swap);
        return;
    }

#if defined(USE_SELECT)
    Random_Access_Iterator median_left = select(first, last, size / 3, predicate, swap, print);
    Random_Access_Iterator median_right = select(first, last, (2 * size) / 3, predicate, swap, print);
#else
    Random_Access_Iterator median_left = first + size / 3;
    Random_Access_Iterator median_right = first + (2 * size) / 3;
#endif
    Random_Access_Iterator pivot_low = first;
    Random_Access_Iterator pivot_high = last - 1;
    swap(*median_left, *pivot_low);
    swap(*median_right, *pivot_high);
    // Ensure *pivot_low < *pivot_high.
    if(predicate(*pivot_high, *pivot_low)) {
        swap(*pivot_high, *pivot_low);
    }
    Random_Access_Iterator i = first + 1;
    Random_Access_Iterator low_range = first + 1;
    Random_Access_Iterator high_range = last - 2;
    while(high_range - i >= 0) {
        while(high_range - i > 0 && !predicate(*high_range, *pivot_high)) {
            --high_range;
        }

        if(predicate(*i, *pivot_low)) {
            // *i < *pivot_low
            swap(*i, *low_range);
            ++i;
            ++low_range;
        } else if(predicate(*pivot_high, *i)) {
            // *i > *pivot_high
            swap(*i, *high_range);
            --high_range;
        } else {
            // *pivot_low <= *i <= *pivot_high
            ++i;
        }
    }

    // Move pivots into the correct positions
    swap(*pivot_low, *(low_range - 1));
    swap(*pivot_high, *(high_range + 1));

    print();

    dual_pivot_quick_sort(first, low_range - 1, predicate, swap, print);
    dual_pivot_quick_sort(low_range, high_range + 1, predicate, swap, print);
    dual_pivot_quick_sort(high_range + 2, last, predicate, swap, print);
}

#if defined(QSORT)
    #define SORT quick_sort
#elif defined(DPQSORT)
    #define SORT dual_pivot_quick_sort
#else
    #error "sort not defined"
#endif

int main(int argc, char** argv) {
    std::vector<i64> numbers;
    numbers.reserve(64);
    i64 a;
    while(std::cin >> a) {
        numbers.push_back(a);
    }

    i64 const count = numbers.size();
    if(count < 50) {
        for(i64 v: numbers) {
            std::cout << std::setw(2) << v << ' ';
        }
        std::cout << '\n';
    }

    i64 compares = 0;
    i64 swaps = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    SORT(
        numbers.begin(), numbers.end(),
        [&compares](i64 lhs, i64 rhs) {
            compares += 1;
            return lhs < rhs;
        },
        [&swaps](i64& lhs, i64& rhs) {
            swaps += 1;
            i64 tmp = lhs;
            lhs = rhs;
            rhs = tmp;
        },
        [first = numbers.begin(), last = numbers.end()]() {
            if(last - first < 50) {
                for(auto i = first; i != last; ++i) {
                    if(i != first) {
                        std::cout << ' ';
                    }
                    std::cout << std::setw(2) << *i;
                }
                std::cout << '\n';
            }
        });
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
    if(count < 50) {
        for(i64 v: numbers) {
            std::cout << std::setw(2) << v << ' ';
        }
        std::cout << '\n';
    }
    std::cout << duration << ',' << compares << ',' << swaps << '\n';

    return 0;
}