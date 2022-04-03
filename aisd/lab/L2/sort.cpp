#include <iomanip>
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>

#include <chrono>

using i64 = long long;

template<typename T>
T min(T a, T b) {
    return a < b ? a : b;
}

// template<typename Input_Iterator, typename Output_Iterator>
// Output_Iterator move(Input_Iterator first, Input_Iterator last, Output_Iterator dest) {
//     for(; first != last; ++first, ++dest) {
//         *dest = std::move(*first);
//     }
//     return dest;
// }

template<typename Forward_Iterator, typename Predicate, typename Swap>
void bubble_sort(Forward_Iterator first, Forward_Iterator last, Predicate predicate, Swap swap) {
    for(; first != last;) {
        Forward_Iterator i = first;
        Forward_Iterator next = first;
        ++next;
        bool swapped = false;
        for(; next != last; ++next, ++i) {
            if(predicate(*next, *i)) {
                swap(*i, *next);
                swapped = true;
            }
        }
        last = i;
        if(!swapped) {
            break;
        }
    }
}

template<typename Random_Access_Iterator, typename Predicate, typename Swap, typename Print>
void insertion_sort(Random_Access_Iterator first, Random_Access_Iterator last, Predicate predicate, Swap swap, Print print) {
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

        print();
    }
}

template<typename Random_Access_Iterator, typename Predicate, typename Swap, typename Print>
void quick_sort(Random_Access_Iterator first, Random_Access_Iterator last, Predicate predicate, Swap swap, Print print) {
    auto const size = last - first;
    if(size < 2) {
        return;
    }

    if(size < 6) {
        bubble_sort(first, last, predicate, swap);
        return;
    }

    Random_Access_Iterator pivot = first + size / 2;
    Random_Access_Iterator i = first;
    Random_Access_Iterator j = last - 1;
    while(true) {
        while(predicate(*i, *pivot)) {
            ++i;
        }

        while(predicate(*pivot, *j)) {
            --j;
        }

        // while(!predicate(*pivot, *i)) {
        //     ++i;
        // }

        // while(!predicate(*j, *pivot)) {
        //     --j;
        // }

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
void merge_sort(Random_Access_Iterator first, Random_Access_Iterator last, Predicate p, Swap swap, Print print) {
    i64 const length = last - first;
    // Sort 4-long runs using bubble sort
    for(i64 i = 0; i < length; i += 4) {
        Random_Access_Iterator b = first + i;
        Random_Access_Iterator e = first + min(i + 4, length);
        bubble_sort(b, e, p, swap);
    }

    if(length <= 4) {
        return;
    }

    using value_type = typename std::iterator_traits<Random_Access_Iterator>::value_type;
    std::vector<value_type> storage;
    storage.reserve(length);
    for(Random_Access_Iterator i = first; i < last; ++i) {
        storage.push_back(*i);
    }
    auto const storage_begin = storage.begin();
    auto const storage_end = storage.end();
    // Direction of elements copying. If true, copy from input range to storage.
    bool copy_a_to_b = true;
    // Bottom-top merge sort
    for(i64 width = 4; width < length; width *= 2) {
        if(copy_a_to_b) {
            i64 i = 0;
            for(; i + width < length; i += 2 * width) {
                Random_Access_Iterator left = first + i;
                Random_Access_Iterator right = first + (i + width);
                Random_Access_Iterator end_l = right;
                Random_Access_Iterator end_r = first + min(i + 2 * width, length);
                auto out = storage_begin + i;
                for(; left != end_l && right != end_r; ++out) {
                    bool const greater = p(*right, *left);
                    if(!greater) {
                        *out = std::move(*left);
                        ++left;
                    } else {
                        *out = std::move(*right);
                        ++right;
                    }
                }

                move(left, end_l, out);
                move(right, end_r, out);
            }

            // There are leftover sorted elements. Copy them to the storage buffer
            if(i < length) {
                Random_Access_Iterator b = first + i;
                move(b, last, storage_begin + i);
            }
        } else {
            i64 i = 0;
            for(; i + width < length; i += 2 * width) {
                auto left = storage_begin + i;
                auto right = storage_begin + (i + width);
                auto end_l = right;
                auto end_r = storage_begin + min(i + 2 * width, length);
                Random_Access_Iterator out = first + i;
                for(; left != end_l && right != end_r; ++out) {
                    bool const greater = p(*right, *left);
                    if(!greater) {
                        *out = std::move(*left);
                        ++left;
                    } else {
                        *out = std::move(*right);
                        ++right;
                    }
                }

                move(left, end_l, out);
                move(right, end_r, out);
            }

            // There are leftover sorted elements. Copy them to the storage buffer
            if(i < length) {
                Random_Access_Iterator out = first + i;
                move(storage_begin + i, storage_end, out);
            }
        }

        print();
        copy_a_to_b = !copy_a_to_b;
    }

    // Copy elements back into the input range
    if(!copy_a_to_b) {
        move(storage_begin, storage_end, first);
    }
}

template<typename Random_Access_Iterator, typename Predicate, typename Swap, typename Print>
void hybrid_sort(Random_Access_Iterator first, Random_Access_Iterator last, Predicate predicate, Swap swap, Print print) {
    auto const size = last - first;
    if(size < 2) {
        return;
    }

    if(size < 6) {
        insertion_sort(first, last, predicate, swap, []() {});
        return;
    }

    Random_Access_Iterator pivot = first + size / 2;
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
        insertion_sort(first, last, predicate, swap, []() {});
        return;
    }

    Random_Access_Iterator median_left = first + size / 3;
    Random_Access_Iterator median_right = first + (2 * size) / 3;
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

#if defined(QUICK_SORT)
    #define SORT quick_sort
#elif defined(MERGE_SORT)
    #define SORT merge_sort
#elif defined(INSERTION_SORT)
    #define SORT insertion_sort
#elif defined(HYBRID_SORT)
    #define SORT hybrid_sort
#elif defined(DUAL_PIVOT_QUICK_SORT)
    #define SORT dual_pivot_quick_sort
#else
    #error "sort not defined"
#endif

int main(int argc, char** argv) {
    std::vector<i64> numbers;
    numbers.reserve(64);
    if(argc == 1) {
        i64 a;
        while(std::cin >> a) {
            numbers.push_back(a);
        }
    } else {
        for(i64 i = 1; i < argc; ++i) {
            i64 v = atoll(argv[i]);
            numbers.push_back(v);
        }
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

    if(count < 50) {
        for(i64 v: numbers) {
            std::cout << std::setw(2) << v << ' ';
        }
        std::cout << '\n';
    }
    std::cout << compares << ',' << swaps << '\n';

    return 0;
}
