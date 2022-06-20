#include <chrono>
#include <iostream>
#include <stdio.h>

using i64 = long long;

template<typename T>
struct List_Node {
    List_Node* next = nullptr;
    List_Node* prev = nullptr;
    T data;

    List_Node(T const& v): data(v) {}
};

template<typename Value_Type>
struct List_Iterator {
public:
    List_Node<Value_Type>* node = nullptr;

    List_Iterator(List_Node<Value_Type>* node): node(node) {}

    [[nodiscard]] Value_Type* operator->() const {
        return &node->data;
    }

    [[nodiscard]] Value_Type& operator*() const {
        return node->data;
    }

    List_Iterator& operator++() {
        node = node->next;
        return *this;
    }

    [[nodiscard]] bool operator==(List_Iterator const& other) const {
        return node == other.node;
    }

    [[nodiscard]] bool operator!=(List_Iterator const& other) const {
        return node != other.node;
    }
};

template<typename T>
struct List {
public:
    using node_type = List_Node<T>;
    using iterator = List_Iterator<T>;

private:
    struct alignas(alignof(node_type)) Internal_Node {
        node_type* next = nullptr;
        node_type* prev = nullptr;

        Internal_Node() {
            next = (node_type*)this;
            prev = (node_type*)this;
        }
    };

    Internal_Node internal_node;

public:
    List() = default;

    List(List const& other) {
        iterator i = begin();
        for(T& v: other) {
            i = insert(i, v);
        }
    }

    List(List&& other) {
        if(other.begin() != other.end()) {
            internal_node.next = other.internal_node.next;
            internal_node.next->prev = (node_type*)&internal_node;
            internal_node.prev = other.internal_node.prev;
            internal_node.prev->next = (node_type*)&internal_node;
        } // else nothing
    }

    [[nodiscard]] iterator begin() {
        return internal_node.next;
    }

    [[nodiscard]] iterator end() {
        return (node_type*)&internal_node;
    }

    iterator insert(iterator const position, T const& v) {
        node_type* const node = new node_type(v);
        node->prev = position.node->prev;
        node->prev->next = node;
        node->next = position.node;
        node->next->prev = node;
        return node;
    }
};

template<typename T>
List<T> merge(List<T>& l1, List<T>& l2) {
    List<T> list;
    List_Iterator<T> i = list.begin();
    for(T& v: l1) {
        i = list.insert(i, v);
    }

    for(T& v: l2) {
        i = list.insert(i, v);
    }

    return list;
}

int main() {
    srand(0);
    List<i64> list;
    for(i64 i = 0; i < 1000; ++i) {
        list.insert(list.begin(), rand());
    }

    i64 average_100 = 0;
    i64 average_300 = 0;
    i64 average_500 = 0;
    i64 average_800 = 0;
    i64 average_rand = 0;
    for(i64 tc = 0; tc < 100; ++tc) {
        List<i64>::iterator iter_100 = list.begin();
        auto tb_100 = std::chrono::high_resolution_clock::now();
        for(i64 i = 100; i > 0; --i) {
            ++iter_100;
        }
        auto te_100 = std::chrono::high_resolution_clock::now();
        average_100 += std::chrono::duration_cast<std::chrono::nanoseconds>(te_100 - tb_100).count();

        List<i64>::iterator iter_300 = list.begin();
        auto tb_300 = std::chrono::high_resolution_clock::now();
        for(i64 i = 300; i > 0; --i) {
            ++iter_300;
        }
        auto te_300 = std::chrono::high_resolution_clock::now();
        average_300 += std::chrono::duration_cast<std::chrono::nanoseconds>(te_300 - tb_300).count();

        List<i64>::iterator iter_500 = list.begin();
        auto tb_500 = std::chrono::high_resolution_clock::now();
        for(i64 i = 500; i > 0; --i) {
            ++iter_500;
        }
        auto te_500 = std::chrono::high_resolution_clock::now();
        average_500 += std::chrono::duration_cast<std::chrono::nanoseconds>(te_500 - tb_500).count();

        List<i64>::iterator iter_800 = list.begin();
        auto tb_800 = std::chrono::high_resolution_clock::now();
        for(i64 i = 800; i > 0; --i) {
            ++iter_800;
        }
        auto te_800 = std::chrono::high_resolution_clock::now();
        average_800 += std::chrono::duration_cast<std::chrono::nanoseconds>(te_800 - tb_800).count();

        List<i64>::iterator iter_rand = list.begin();
        auto tb_rand = std::chrono::high_resolution_clock::now();
        for(i64 i = rand() % 1000; i > 0; --i) {
            ++iter_rand;
        }
        auto te_rand = std::chrono::high_resolution_clock::now();
        average_rand += std::chrono::duration_cast<std::chrono::nanoseconds>(te_rand - tb_rand).count();
    }

    std::cout << "access 100: " << (average_100 / 100) << "ns\n";
    std::cout << "access 300: " << (average_300 / 100) << "ns\n";
    std::cout << "access 500: " << (average_500 / 100) << "ns\n";
    std::cout << "access 800: " << (average_800 / 100) << "ns\n";
    std::cout << "access rand: " << (average_rand / 100) << "ns\n";

    List<i64> list1;
    for(i64 i = 0; i < 10; ++i) {
        list1.insert(list1.begin(), rand());
    }

    List<i64> list2;
    for(i64 i = 0; i < 10; ++i) {
        list2.insert(list2.begin(), rand());
    }

    std::cout << "list1\n";
    for(i64 v: list1) {
        std::cout << v << ", ";
    }
    std::cout << '\n';

    std::cout << "list2\n";
    for(i64 v: list2) {
        std::cout << v << ", ";
    }
    std::cout << '\n';

    List<i64> merged = merge(list1, list2);
    std::cout << "merged\n";
    for(i64 v: merged) {
        std::cout << v << ", ";
    }
    std::cout << '\n';

    return 0;
}
