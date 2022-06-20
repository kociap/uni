#include <iostream>
#include <stdio.h>

using i64 = long long;

#define STACK_MIN_ALLOCATION_SIZE 64
#define QUEUE_MIN_ALLOCATION_SIZE 64

template<typename T>
void uninitialized_copy(T* dst, T* src, T* src_end) {
    for(; src != src_end; ++src, ++dst) {
        ::new(dst) T(*src);
    }
}

template<typename T>
void destruct(T* first, T* last) {
    for(; first != last; ++first) {
        first->~T();
    }
}

template<typename T>
struct Stack {
public:
    Stack() = default;
    Stack(Stack const& other) = delete;
    Stack(Stack&& other) = delete;
    Stack& operator=(Stack const& other) = delete;
    Stack& operator=(Stack&& other) = delete;

    ~Stack() {
        destruct(data, data + size);
        free(data);
    }

    [[nodiscard]] bool empty() const {
        return size == 0;
    }

    void push(T const& v) {
        ensure_capacity(size + 1);
        ::new(data + size) T(v);
        ++size;
    }

    void pop() {
        (data + size - 1)->~T();
        --size;
    }

    [[nodiscard]] T& top() {
        return data[size - 1];
    }

    [[nodiscard]] T const& top() const {
        return data[size - 1];
    }

private:
    T* data = nullptr;
    i64 capacity = 0;
    i64 size = 0;

    void ensure_capacity(i64 const requested_capacity) {
        if(requested_capacity > capacity) {
            i64 new_capacity = (capacity > 0 ? capacity : STACK_MIN_ALLOCATION_SIZE);
            while(new_capacity < requested_capacity) {
                new_capacity *= 2;
            }

            T* new_data = (T*)malloc(new_capacity * sizeof(T));
            uninitialized_copy(new_data, data, data + size);
            destruct(data, data + size);
            free(data);
            data = new_data;
            capacity = new_capacity;
        }
    }
};

template<typename T>
struct Queue {
    Queue() = default;
    Queue(Queue const& other) = delete;
    Queue(Queue&& other) = delete;
    Queue& operator=(Queue const& other) = delete;
    Queue& operator=(Queue&& other) = delete;

    ~Queue() {}

    [[nodiscard]] bool empty() const {
        return (size_end - size_begin) == 0;
    }

    void push(T const& v) {
        ensure_capacity(size_end - size_begin + 1);
        ::new(size_end) T(v);
        ++size_end;
    }

    void pop() {
        (size_begin)->~T();
        ++size_begin;
    }

    [[nodiscard]] T& top() {
        return *size_begin;
    }

    [[nodiscard]] T const& top() const {
        return *size_begin;
    }

private:
    T* data = nullptr;
    T* capacity = nullptr;
    T* size_begin = nullptr;
    T* size_end = nullptr;

    void ensure_capacity(i64 const requested_capacity) {
        i64 const old_capacity = capacity - data;
        i64 const old_size = size_end - size_begin;
        bool const relocate_only = old_capacity > 0 && old_size * 2 <= old_capacity && size_end == capacity;
        if(relocate_only) {
            uninitialized_copy(data, size_begin, size_end);
            destruct(size_begin, size_end);
            size_begin = data;
            size_end = data + old_size;
        } else if(requested_capacity > old_capacity || size_end == capacity) {
            i64 new_capacity = (old_capacity > 0 ? old_capacity : QUEUE_MIN_ALLOCATION_SIZE);
            do {
                new_capacity *= 2;
            } while(new_capacity < requested_capacity);

            T* new_data = (T*)malloc(new_capacity * sizeof(T));
            uninitialized_copy(new_data, size_begin, size_end);
            destruct(size_begin, size_end);
            free(data);
            data = new_data;
            capacity = data + new_capacity;
            size_begin = data;
            size_end = data + old_size;
        }
    }
};

int main() {
    srand(0);

    std::cout << "populating\n";
    Stack<i64> stack;
    Queue<i64> queue;
    for(i64 i = 0; i < 100; ++i) {
        i64 const v = rand();
        std::cout << v << ", ";
        stack.push(v);
        queue.push(v);
    }
    std::cout << "\nstack\n";
    while(!stack.empty()) {
        i64 const v = stack.top();
        std::cout << v << ", ";
        stack.pop();
    }
    std::cout << "\nqueue\n";
    while(!queue.empty()) {
        i64 const v = queue.top();
        std::cout << v << ", ";
        queue.pop();
    }

    return 0;
}