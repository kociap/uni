#include <chrono>
#include <iostream>
#include <stdio.h>

using i64 = long long;

template<typename T>
struct SList_Node {
    SList_Node* next = nullptr;
    T data;

    SList_Node(T const& v): data(v) {}
};

template<typename Value_Type>
struct SList_Iterator {
public:
    SList_Node<Value_Type>* node = nullptr;

    SList_Iterator(SList_Node<Value_Type>* node): node(node) {}

    [[nodiscard]] Value_Type* operator->() const {
        return &node->data;
    }

    [[nodiscard]] Value_Type& operator*() const {
        return node->data;
    }

    SList_Iterator& operator++() {
        node = node->next;
        return *this;
    }

    [[nodiscard]] bool operator==(SList_Iterator const& other) const {
        return node == other.node;
    }

    [[nodiscard]] bool operator!=(SList_Iterator const& other) const {
        return node != other.node;
    }
};

template<typename T>
struct SList {
public:
    using node_type = SList_Node<T>;
    using iterator = SList_Iterator<T>;

private:
    struct alignas(alignof(node_type)) Internal_Node {
        node_type* next = nullptr;
    };

    Internal_Node internal_node;

public:
    SList() = default;

    SList(SList const& other) {
        iterator b = before_begin();
        for(auto& v: other) {
            b = insert_after(b, v);
        }
    }

    SList(SList&& other) {
        internal_node.next = other.internal_node.next;
        other.internal_node.next = nullptr;
    }

    [[nodiscard]] iterator before_begin() {
        return (node_type*)&internal_node;
    }

    [[nodiscard]] iterator begin() {
        return internal_node.next;
    }

    [[nodiscard]] iterator end() {
        return nullptr;
    }

    iterator insert_after(iterator const& position, T const& v) {
        node_type* const node = new node_type(v);
        node->next = position.node->next;
        position.node->next = node;
        return node;
    }
};

template<typename T>
SList<T> merge(SList<T>& l1, SList<T>& l2) {
    SList<T> list;
    SList_Iterator<T> i = list.before_begin();
    for(T& v: l1) {
        i = list.insert_after(i, v);
    }

    for(T& v: l2) {
        i = list.insert_after(i, v);
    }

    return list;
}

int main() {
    srand(0);
    SList<i64> list;
    for(i64 i = 0; i < 1000; ++i) {
        list.insert_after(list.before_begin(), rand());
    }

    i64 average_100 = 0;
    i64 average_300 = 0;
    i64 average_500 = 0;
    i64 average_800 = 0;
    i64 average_rand = 0;
    for(i64 tc = 0; tc < 100; ++tc) {
        SList<i64>::iterator iter_100 = list.before_begin();
        auto tb_100 = std::chrono::high_resolution_clock::now();
        for(i64 i = 100; i > 0; --i) {
            ++iter_100;
        }
        auto te_100 = std::chrono::high_resolution_clock::now();
        average_100 += std::chrono::duration_cast<std::chrono::nanoseconds>(te_100 - tb_100).count();

        SList<i64>::iterator iter_300 = list.before_begin();
        auto tb_300 = std::chrono::high_resolution_clock::now();
        for(i64 i = 300; i > 0; --i) {
            ++iter_300;
        }
        auto te_300 = std::chrono::high_resolution_clock::now();
        average_300 += std::chrono::duration_cast<std::chrono::nanoseconds>(te_300 - tb_300).count();

        SList<i64>::iterator iter_500 = list.before_begin();
        auto tb_500 = std::chrono::high_resolution_clock::now();
        for(i64 i = 500; i > 0; --i) {
            ++iter_500;
        }
        auto te_500 = std::chrono::high_resolution_clock::now();
        average_500 += std::chrono::duration_cast<std::chrono::nanoseconds>(te_500 - tb_500).count();

        SList<i64>::iterator iter_800 = list.before_begin();
        auto tb_800 = std::chrono::high_resolution_clock::now();
        for(i64 i = 800; i > 0; --i) {
            ++iter_800;
        }
        auto te_800 = std::chrono::high_resolution_clock::now();
        average_800 += std::chrono::duration_cast<std::chrono::nanoseconds>(te_800 - tb_800).count();

        SList<i64>::iterator iter_rand = list.before_begin();
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

    SList<i64> list1;
    for(i64 i = 0; i < 10; ++i) {
        list1.insert_after(list1.before_begin(), rand());
    }

    SList<i64> list2;
    for(i64 i = 0; i < 10; ++i) {
        list2.insert_after(list2.before_begin(), rand());
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

    SList<i64> merged = merge(list1, list2);
    std::cout << "merged\n";
    for(i64 v: merged) {
        std::cout << v << ", ";
    }
    std::cout << '\n';

    return 0;
}
