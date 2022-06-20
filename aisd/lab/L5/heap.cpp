#include <chrono>
#include <iostream>
#include <type_traits>
#include <vector>

using i64 = long long;

template<typename T, typename Compare, typename Swap>
struct Heap {
public:
    Heap(Compare compare, Swap swap): compare(compare), swap(swap) {}

    void insert(T value) {
        storage.push_back(std::move(value));
        T* const element = &storage.back();
        heapify_up(element);
    }

    T extract() {
        T value = std::move(storage.front());
        using std::swap;
        swap(storage.front(), storage.back());
        storage.resize(storage.size() - 1);
        T* const element = storage.data();
        heapify_down(element);
        return value;
    }

    i64 size() const {
        return storage.size();
    }

    void print() {
        if(storage.size() > 0) {
            Print_Heap_Ctx ctx;
            ctx.left_trace = (char*)malloc(10000 * sizeof(char));
            ctx.right_trace = (char*)malloc(10000 * sizeof(char));
            for(int i = 0; i < 10000; i++) {
                ctx.left_trace[i] = ' ';
                ctx.left_trace[i] = ' ';
            }
            internal_print(ctx, &storage.front(), 0, '-');
            free(ctx.left_trace);
            free(ctx.right_trace);
        }
    }

    std::vector<T> const& get_storage() const {
        return storage;
    }

private:
    std::vector<T> storage;
    Compare compare;
    Swap swap;

    T* parent(T const* const element) {
        T* const begin = storage.data();
        i64 const index = element - begin;
        return begin + (index - 1) / 2;
    }

    T* left(T const* const element) {
        T* const begin = storage.data();
        i64 const index = element - begin;
        i64 const child_index = 2 * index + 1;
        return child_index < storage.size() ? (begin + child_index) : nullptr;
    }

    T* right(T const* const element) {
        T* const begin = storage.data();
        i64 const index = element - begin;
        i64 const child_index = 2 * index + 2;
        return child_index < storage.size() ? (begin + child_index) : nullptr;
    }

    void heapify_up(T* const element) {
        T* const p = parent(element);
        if(compare(*p, *element)) {
            // using std::swap;
            swap(*p, *element);
            heapify_up(p);
        }
    }

    void heapify_down(T* const element) {
        T* largest = element;
        T* const l = left(element);
        if(l != nullptr && compare(*largest, *l)) {
            largest = l;
        }
        T* const r = right(element);
        if(r != nullptr && compare(*largest, *r)) {
            largest = r;
        }

        if(largest != element) {
            // using std::swap;
            swap(*largest, *element);
            heapify_down(largest);
        }
    }

    struct Print_Heap_Ctx {
        char* left_trace;
        char* right_trace;
    };

    void internal_print(Print_Heap_Ctx const& ctx, T const* const root, int const depth, char const prefix) {
        if(root == nullptr) {
            return;
        }

        T const* l = left(root);
        if(l != nullptr) {
            internal_print(ctx, l, depth + 1, '/');
        }

        if(prefix == '/') {
            ctx.left_trace[depth - 1] = '|';
        }
        if(prefix == '\\') {
            ctx.right_trace[depth - 1] = ' ';
        }

        if(depth == 0) {
            std::cout << '-';
        }
        if(depth > 0) {
            std::cout << ' ';
        }

        for(int i = 0; i < depth - 1; i++) {
            if(ctx.left_trace[i] == '|' || ctx.right_trace[i] == '|') {
                std::cout << "| ";
            } else {
                std::cout << "  ";
            }
        }

        if(depth > 0) {
            std::cout << prefix << '-';
        }

        std::cout << '[' << *root << "]\n";
        ctx.left_trace[depth] = ' ';
        T* const r = right(root);
        if(r != nullptr) {
            ctx.right_trace[depth] = '|';
            internal_print(ctx, r, depth + 1, '\\');
        }
    }
};

int main() {
    i64 swap_counter = 0;
    i64 compare_counter = 0;

    auto compare = [&compare_counter](i64 lhs, i64 rhs) {
        compare_counter += 1;
        return lhs > rhs;
    };

    auto swap = [&swap_counter](i64& lhs, i64& rhs) {
        i64 tmp = lhs;
        lhs = rhs;
        rhs = tmp;
        swap_counter += 1;
    };

    Heap<i64, decltype(compare), decltype(swap)> heap(compare, swap);
    i64 n;
    std::cin >> n;
    if(n < 50) {
        for(i64 i = 0; i < n; ++i) {
            i64 a;
            std::cin >> a;
            heap.insert(a);
#if defined(HEAP)
            std::cout << "insert " << a << '\n';
            heap.print();
            std::cout << '\n';
#endif
        }
    } else {
        for(i64 i = 0; i < n; ++i) {
            i64 a;
            std::cin >> a;
            heap.insert(a);
        }
    }

    std::vector<i64> array;
    if(n < 50) {
        while(heap.size() > 0) {
            i64 const value = heap.extract();
#if defined(HEAPSORT)
            std::vector<i64> const& storage = heap.get_storage();
            array.push_back(value);
            for(i64 v: storage) {
                std::cout << v << ' ';
            }
            for(i64 v: array) {
                std::cout << v << ' ';
            }
            std::cout << '\n';
#endif
#if defined(HEAP)
            std::cout << "extract " << value << '\n';
            heap.print();
            std::cout << '\n';
#endif
        }
#if defined(HEAPSORT)
        for(i64 v: array) {
            std::cout << v << ' ';
        }
        std::cout << '\n';
#endif
        std::cout << compare_counter << ',' << swap_counter << '\n';
    } else {
        while(heap.size() > 0) {
            heap.extract();
        }
        std::cout << compare_counter << ',' << swap_counter << '\n';
    }

    return 0;
}
