#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using i64 = long long;
using i32 = int;

template<typename T, typename Key_Less, typename Pointer_Access, typename Pointer_Assign>
struct Binary_Search_Tree {
public:
    struct Node {
    private:
        Node* _parent;
        Node* _left;
        Node* _right;
        T _key;

        friend struct Binary_Search_Tree;

        Node(Node* parent, T key, Node* left = nullptr, Node* right = nullptr): _parent(parent), _key(key), _left(left), _right(right) {}
        Node(Node const& other) = default;
        Node(Node&& other) = default;
        ~Node() = default;
        Node& operator=(Node const& other) = default;
        Node& operator=(Node&& other) = default;

    public:
        Node const* parent() const {
            return _parent;
        }

        Node const* right() const {
            return _right;
        }

        Node const* left() const {
            return _left;
        }

        T const& key() const {
            return _key;
        }
    };

public:
    Binary_Search_Tree(Key_Less key_less, Pointer_Access ptr_access, Pointer_Assign ptr_assign)
        : _key_less(key_less), _pointer_access(ptr_access), _pointer_assign(ptr_assign) {}
    ~Binary_Search_Tree() {
        free_subtree(_root);
    }

#define D(p) (_pointer_access(), *p)
#define A(l, r) (_pointer_assign(), l = r)

    Node const* insert(T k) {
        Node* prev = nullptr;
        Node* node = nullptr;
        A(node, _root);
        while(node != nullptr) {
            A(prev, node);
            if(_key_less(k, D(node)._key)) {
                A(node, D(node)._left);
            } else {
                A(node, D(node)._right);
            }
        }

        Node* new_node = nullptr;
        A(new_node, new Node(nullptr, k));
        if(prev == nullptr) {
            A(_root, new_node);
        } else {
            A(D(new_node)._parent, prev);
            if(_key_less(D(new_node)._key, D(prev)._key)) {
                A(D(prev)._left, new_node);
            } else {
                A(D(prev)._right, new_node);
            }
        }
        return new_node;
    }

    Node const* find(T const& k) {
        Node* node = nullptr;
        A(node, _root);
        while(node != nullptr) {
            if(_key_less(k, D(node)._key)) {
                A(node, D(node)._left);
            } else if(_key_less(D(node)._key, k)) {
                A(node, D(node)._right);
            } else {
                break;
            }
        }
        return node;
    }

    void erase(Node const* const n) {
        Node* parent = nullptr;
        A(parent, D(n)._parent);
        Node* left = nullptr;
        A(left, D(n)._left);
        Node* right = nullptr;
        A(right, D(n)._right);
        if(left == nullptr) {
            transplant(n, right);
        } else if(right == nullptr) {
            transplant(n, left);
        } else {
            Node* y = nullptr;
            A(y, right);
            while(D(y)._left != nullptr) {
                A(y, D(y)._left);
            }
            if(D(y)._parent != n) {
                transplant(y, D(y)._right);
                A(D(y)._right, right);
                A(D(right)._parent, y);
            }

            transplant(n, y);
            A(D(y)._left, left);
            A(D(left)._parent, y);
        }
        delete n;
    }

    [[nodiscard]] Node const* root() const {
        return _root;
    }

    [[nodiscard]] i64 height() const {
        return calculate_height(_root);
    }

    [[nodiscard]] i64 calculate_height_iterative(Node** buffer) {
        if(_root == nullptr) {
            return 0;
        }

        Node** begin = buffer;
        Node** end = buffer;
        *end = _root;
        ++end;
        i64 height = 0;
        while(end - begin > 0) {
            height++;
            i64 nodes = end - begin;
            while(nodes--) {
                Node* const current = *begin;
                if(current->_left != nullptr) {
                    *end = current->_left;
                    ++end;
                }
                if(current->_right != nullptr) {
                    *end = current->_right;
                    ++end;
                }
                ++begin;
            }
        }
        return height;
    }

private:
    Node* _root = nullptr;
    Key_Less _key_less;
    Pointer_Access _pointer_access;
    Pointer_Assign _pointer_assign;

    i64 calculate_height(Node const* const node) const {
        if(node == nullptr) {
            return 0;
        }

        i64 const left = calculate_height(D(node)._left);
        i64 const right = calculate_height(D(node)._right);
        return 1 + (left > right ? left : right);
    }

    void free_subtree(Node* const node) {
        if(node != nullptr) {
            free_subtree(D(node)._left);
            free_subtree(D(node)._right);
            delete node;
        }
    }

    void transplant(Node const* u, Node* v) {
        if(D(u)._parent == nullptr) {
            A(_root, v);
        } else if(u == D(D(u)._parent)._left) {
            A(D(D(u)._parent)._left, v);
        } else {
            A(D(D(u)._parent)._right, v);
        }

        if(v != nullptr) {
            A(D(v)._parent, D(u)._parent);
        }
    }

#undef D
#undef A
};

struct BST_Key_Less {
public:
    BST_Key_Less(i64* counter): counter(counter) {}

    [[nodiscard]] bool operator()(i64 const lhs, i64 const rhs) const {
        *counter += 1;
        return lhs < rhs;
    }

private:
    i64* counter = nullptr;
};

struct BST_Access {
public:
    BST_Access(i64* counter): counter(counter) {}

    void operator()() const {
        *counter += 1;
    }

private:
    i64* counter = nullptr;
};

struct BST_Assign {
public:
    BST_Assign(i64* counter): counter(counter) {}

    void operator()() const {
        *counter += 1;
    }

private:
    i64* counter = nullptr;
};

using BST_Node = Binary_Search_Tree<i64, BST_Key_Less, BST_Access, BST_Assign>::Node;

struct Print_BST_Context {
    char* left_trace;
    char* right_trace;
};

void internal_print_BST(Print_BST_Context const& ctx, BST_Node const* const root, int const depth, char const prefix) {
    if(root == nullptr) {
        return;
    }

    if(root->left() != nullptr) {
        internal_print_BST(ctx, root->left(), depth + 1, '/');
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

    std::cout << '[' << root->key() << "]\n";
    ctx.left_trace[depth] = ' ';
    if(root->right() != nullptr) {
        ctx.right_trace[depth] = '|';
        internal_print_BST(ctx, root->right(), depth + 1, '\\');
    }
}

void print_BST(BST_Node const* const root) {
    Print_BST_Context ctx;
    ctx.left_trace = (char*)malloc(10000 * sizeof(char));
    ctx.right_trace = (char*)malloc(10000 * sizeof(char));
    for(int i = 0; i < 10000; i++) {
        ctx.left_trace[i] = ' ';
        ctx.left_trace[i] = ' ';
    }
    internal_print_BST(ctx, root, 0, '-');
    free(ctx.left_trace);
    free(ctx.right_trace);
}

struct String_View {
    char const* begin;
    char const* end;
};

struct Output_Buffer {
public:
    char buf[65536];
    char* i;
    char* end;

    Output_Buffer(): i(buf), end(buf + 65536) {}

    ~Output_Buffer() {
        flush();
    }

    void flush() {
        fwrite(buf, 1, i - buf, stdout);
        fflush(stdout);
        i = buf;
    }

    void write(i64 v) {
        if(end - i < 30) {
            fwrite(buf, 1, i - buf, stdout);
            i = buf;
        }

        char buffer[21];
        String_View number = to_string(v, buffer);
        for(; number.begin != number.end; ++number.begin, ++i) {
            *i = *number.begin;
        }
    }

    void write(char c) {
        if(end - i < 1) {
            fwrite(buf, 1, i - buf, stdout);
            i = buf;
        }

        *i = c;
        ++i;
    }

    void write(char const* string) {
        i64 const length = strlen(string);
        if(end - i < length) {
            fwrite(buf, 1, i - buf, stdout);
            i = buf;
        }

        String_View str = {string, string + length};
        for(; str.begin != str.end; ++str.begin, ++i) {
            *i = *str.begin;
        }
    }

private:
    // buffer must be a char array of 21 elements.
    String_View to_string(i64 v, char* buffer) {
        // We don't need null-terminator or initialized elements.
        bool has_sign = false;
        if(v < 0) {
            has_sign = true;
            v = -v;
        }

        i32 i = 20;
        do {
            buffer[i] = (v % 10) | '0';
            --i;
            v /= 10;
        } while(v > 0);
        buffer[i] = '-';
        return {buffer + i + !has_sign, buffer + 21};
    }
};

int main(int argc, char** argv) {
    Output_Buffer out;

    i64 const skip = atoi(argv[1]);

    struct Counters {
        i64 compares = 0;
        i64 accesses = 0;
        i64 assigns = 0;

        void reset() {
            compares = 0;
            accesses = 0;
            assigns = 0;
        }
    };

    Counters counters;
    BST_Key_Less key_less(&counters.compares);
    BST_Access access(&counters.accesses);
    BST_Assign assign(&counters.assigns);
    Binary_Search_Tree<i64, BST_Key_Less, BST_Access, BST_Assign> bst(key_less, access, assign);

    i64 n;
    std::cin >> n;

    BST_Node** const buffer_height = (BST_Node**)malloc(n * sizeof(BST_Node*));
    if(n <= 50) {
        for(i64 i = 0; i < n; ++i) {
            i64 a;
            std::cin >> a;
            std::cout << "\ninsert " << a << "\n";
            bst.insert(a);
            std::cout << bst.height() << '\n';
            print_BST(bst.root());
        }
    } else {
        for(i64 i = 0; i < n; ++i) {
            i64 a;
            scanf("%lld", &a);
            bst.insert(a);
            if(i % skip == 0) {
                out.write(counters.compares);
                out.write(',');
                out.write(counters.assigns);
                out.write(',');
                out.write(counters.accesses);
                out.write(',');
                out.write(bst.calculate_height_iterative(buffer_height));
                out.write('\n');
                // std::cout << counters.compares << ',' << counters.accesses << ',' << counters.assigns << ',';
                // std::cout << bst.height() << '\n';
            }
            counters.reset();
        }
    }

    i64 k;
    std::cin >> k;

    if(k <= 50) {
        for(i64 i = 0; i < k; ++i) {
            i64 a;
            std::cin >> a;
            BST_Node const* node = bst.find(a);
            if(node != nullptr) {
                std::cout << "\nerase " << a << "\n";
                bst.erase(node);
                std::cout << bst.height() << '\n';
                print_BST(bst.root());
            }
        }
    } else {
        for(i64 i = 0; i < k; ++i) {
            i64 a;
            std::cin >> a;
            BST_Node const* node = bst.find(a);
            if(node != nullptr) {
                bst.erase(node);
            }
            if(i % skip == 0) {
                out.write(counters.compares);
                out.write(',');
                out.write(counters.assigns);
                out.write(',');
                out.write(counters.accesses);
                out.write(',');
                out.write(bst.calculate_height_iterative(buffer_height));
                out.write('\n');
                // std::cout << counters.compares << ',' << counters.accesses << ',' << counters.assigns << ',';
                // std::cout << bst.height() << '\n';
            }
            counters.reset();
        }
    }

    out.flush();

    return 0;
}
