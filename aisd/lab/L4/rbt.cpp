#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using i64 = long long;
using i32 = int;

template<typename T, typename Key_Less, typename Pointer_Access, typename Pointer_Assign>
struct RB_Tree {
public:
    enum struct Color { red, black };

    struct Node {
    private:
        Node* _parent;
        Node* _left;
        Node* _right;
        T _key;
        Color _color;

        friend struct RB_Tree;

        Node(Node* parent, T key, Color color, Node* left = nullptr, Node* right = nullptr)
            : _parent(parent), _key(key), _left(left), _right(right), _color(color) {}
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

        Color color() const {
            return _color;
        }
    };

public:
    RB_Tree(Key_Less key_less, Pointer_Access ptr_access, Pointer_Assign ptr_assign)
        : _key_less(key_less), _pointer_access(ptr_access), _pointer_assign(ptr_assign) {}
    ~RB_Tree() {
        free_subtree(_root);
    }

#define D(p) (_pointer_access(), *p)
#define A(l, r) (_pointer_assign(), l = r)
#define AR(p) (_pointer_assign(), p)

    Node const* insert(T k) {
        Node* prev = nullptr;
        Node* node = AR(_root);
        while(node != nullptr) {
            prev = AR(node);
            if(_key_less(k, D(node)._key)) {
                node = AR(D(node)._left);
            } else {
                node = AR(D(node)._right);
            }
        }

        Node* const new_node = AR(new Node(nullptr, k, Color::red));
        if(prev == nullptr) {
            _root = AR(new_node);
        } else {
            D(new_node)._parent = AR(prev);
            if(_key_less(D(new_node)._key, D(prev)._key)) {
                D(prev)._left = AR(new_node);
            } else {
                D(prev)._right = AR(new_node);
            }
        }
        insert_fixup(new_node);
        return new_node;
    }

    Node const* find(T const& k) {
        Node* node = AR(_root);
        while(node != nullptr) {
            if(_key_less(k, D(node)._key)) {
                node = AR(D(node)._left);
            } else if(_key_less(D(node)._key, k)) {
                node = AR(D(node)._right);
            } else {
                break;
            }
        }
        return node;
    }

    void erase(Node const* const n) {
        Color original_color = n->_color;
        Node* x = nullptr;
        if(D(n)._left == nullptr) {
            x = AR(D(n)._right);
            transplant(n, D(n)._right);
        } else if(D(n)._right == nullptr) {
            x = AR(D(n)._left);
            transplant(n, D(n)._left);
        } else {
            Node* y = AR(D(n)._right);
            // Tree minimum.
            while(D(y)._left != nullptr) {
                y = AR(D(y)._left);
            }
            original_color = D(y)._color;
            x = AR(D(y)._right);
            if(D(y)._parent == n) {
                if(x) {
                    D(x)._parent = y;
                }
            } else {
                transplant(y, D(y)._right);
                D(y)._right = AR(D(n)._right);
                D(D(y)._right)._parent = AR(y);
            }
            transplant(n, y);
            D(y)._left = AR(D(n)._left);
            D(D(y)._left)._parent = AR(y);
            D(y)._color = AR(D(n)._color);
        }
        if(x && original_color == Color::black) {
            erase_fixup(x);
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

    void transplant(Node const* const u, Node* const v) {
        if(D(u)._parent == nullptr) {
            _root = AR(v);
        } else if(u == D(D(u)._parent)._left) {
            D(D(u)._parent)._left = AR(v);
        } else {
            D(D(u)._parent)._right = AR(v);
        }
        if(v != nullptr) {
            D(v)._parent = AR(D(u)._parent);
        }
    }

    void left_rotate(Node* const x) {
        if(D(x)._right == nullptr) {
            return;
        }

        Node* y = AR(D(x)._right);
        D(x)._right = AR(D(y)._left);
        if(D(x)._right != nullptr) {
            D(D(x)._right)._parent = AR(x);
        }
        if(D(x)._parent == nullptr) {
            _root = AR(y);
        } else if(D(D(x)._parent)._left == x) {
            D(D(x)._parent)._left = AR(y);
        } else {
            D(D(x)._parent)._right = AR(y);
        }
        D(y)._parent = AR(D(x)._parent);
        D(y)._left = AR(x);
        D(x)._parent = AR(y);
    }

    void right_rotate(Node* const y) {
        if(D(y)._left == nullptr) {
            return;
        }

        Node* x = AR(D(y)._left);
        D(y)._left = AR(D(x)._right);
        if(D(y)._left != nullptr) {
            D(D(y)._left)._parent = AR(y);
        }
        if(D(y)._parent == nullptr) {
            _root = AR(x);
        } else if(D(D(y)._parent)._left == y) {
            D(D(y)._parent)._left = AR(x);
        } else {
            D(D(y)._parent)._right = AR(x);
        }
        D(x)._parent = AR(D(y)._parent);
        D(x)._right = AR(y);
        D(y)._parent = AR(x);
    }

    void insert_fixup(Node* node) {
        while(D(node)._parent && D(D(node)._parent)._color == Color::red) {
            if(D(node)._parent == D(D(D(node)._parent)._parent)._left) {
                Node* const uncle = AR(D(D(D(node)._parent)._parent)._right);
                if(uncle && D(uncle)._color == Color::red) {
                    D(D(node)._parent)._color = Color::black;
                    D(uncle)._color = Color::black;
                    D(D(D(node)._parent)._parent)._color = Color::red;
                    node = AR(D(D(node)._parent)._parent);
                } else {
                    if(node == D(D(node)._parent)._right) {
                        node = AR(D(node)._parent);
                        left_rotate(node);
                    }
                    D(D(node)._parent)._color = Color::black;
                    D(D(D(node)._parent)._parent)._color = Color::red;
                    right_rotate(D(D(node)._parent)._parent);
                }
            } else {
                Node* const uncle = AR(D(D(D(node)._parent)._parent)._left);
                if(uncle && D(uncle)._color == Color::red) {
                    D(D(node)._parent)._color = Color::black;
                    D(uncle)._color = Color::black;
                    D(D(D(node)._parent)._parent)._color = Color::red;
                    node = AR(D(D(node)._parent)._parent);
                } else {
                    if(node == D(D(node)._parent)._left) {
                        node = AR(D(node)._parent);
                        right_rotate(node);
                    }
                    D(D(node)._parent)._color = Color::black;
                    D(D(D(node)._parent)._parent)._color = Color::red;
                    left_rotate(D(D(node)._parent)._parent);
                }
            }
        }
        D(_root)._color = Color::black;
    }

    void erase_fixup(Node* node) {
        while(node != _root && D(node)._color == Color::black) {
            // w is never a nullptr because if node has a parent, then node is either its left or right child.
            if(node == D(D(node)._parent)._right) {
                Node* w = AR(D(D(node)._parent)._right);
                if(D(w)._color == Color::red) {
                    D(w)._color = Color::black;
                    D(D(node)._parent)._color = Color::red;
                    left_rotate(D(node)._parent);
                    w = AR(D(D(node)._parent)._right);
                }
                if((D(w)._left == nullptr || D(D(w)._left)._color == Color::black) && (D(w)._right == nullptr || D(D(w)._right)._color == Color::black)) {
                    D(w)._color = Color::red;
                    node = AR(D(node)._parent);
                } else {
                    // If the right child is black, then the left child must exist and be red.
                    if((D(w)._right == nullptr || D(D(w)._right)._color == Color::black)) {
                        D(D(w)._left)._color = Color::black;
                        D(w)._color = Color::red;
                        right_rotate(w);
                        w = AR(D(D(node)._parent)._right);
                    }
                    D(w)._color = D(D(node)._parent)._color;
                    D(D(node)._parent)._color = Color::black;
                    if(D(w)._right) {
                        D(D(w)._right)._color = Color::black;
                    }
                    left_rotate(D(node)._parent);
                    node = AR(_root);
                }
            } else {
                Node* w = AR(D(D(node)._parent)._left);
                if(D(w)._color == Color::red) {
                    D(w)._color = Color::black;
                    D(D(node)._parent)._color = Color::red;
                    left_rotate(D(node)._parent);
                    w = AR(D(D(node)._parent)._left);
                }
                if((D(w)._right == nullptr || D(D(w)._right)._color == Color::black) && (D(w)._left == nullptr || D(D(w)._left)._color == Color::black)) {
                    D(w)._color = Color::red;
                    node = AR(D(node)._parent);
                } else {
                    // If the left child is black, then the right child must exist and be red.
                    if((D(w)._left == nullptr || D(D(w)._left)._color == Color::black)) {
                        D(D(w)._right)._color = Color::black;
                        D(w)._color = Color::red;
                        left_rotate(w);
                        w = AR(D(D(node)._parent)._left);
                    }
                    D(w)._color = D(D(node)._parent)._color;
                    D(D(node)._parent)._color = Color::black;
                    if(D(w)._left) {
                        D(D(w)._left)._color = Color::black;
                    }
                    right_rotate(D(node)._parent);
                    node = AR(_root);
                }
            }
        }
        D(node)._color = Color::black;
    }

#undef D
#undef A
};

struct RBT_Key_Less {
public:
    RBT_Key_Less(i64* counter): counter(counter) {}

    [[nodiscard]] bool operator()(i64 const lhs, i64 const rhs) const {
        *counter += 1;
        return lhs < rhs;
    }

private:
    i64* counter = nullptr;
};

struct RBT_Access {
public:
    RBT_Access(i64* counter): counter(counter) {}

    void operator()() const {
        *counter += 1;
    }

private:
    i64* counter = nullptr;
};

struct RBT_Assign {
public:
    RBT_Assign(i64* counter): counter(counter) {}

    void operator()() const {
        *counter += 1;
    }

private:
    i64* counter = nullptr;
};

using RBT_Node = RB_Tree<i64, RBT_Key_Less, RBT_Access, RBT_Assign>::Node;

struct Print_RBT_Context {
    char* left_trace;
    char* right_trace;
};

void internal_print_RBT(Print_RBT_Context const& ctx, RBT_Node const* const root, int const depth, char const prefix) {
    if(root == nullptr) {
        return;
    }

    if(root->left() != nullptr) {
        internal_print_RBT(ctx, root->left(), depth + 1, '/');
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
        internal_print_RBT(ctx, root->right(), depth + 1, '\\');
    }
}

void print_RBT(RBT_Node const* const root) {
    Print_RBT_Context ctx;
    ctx.left_trace = (char*)malloc(10000 * sizeof(char));
    ctx.right_trace = (char*)malloc(10000 * sizeof(char));
    for(int i = 0; i < 10000; i++) {
        ctx.left_trace[i] = ' ';
        ctx.left_trace[i] = ' ';
    }
    internal_print_RBT(ctx, root, 0, '-');
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
    constexpr i64 max_print = 50;
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
    RBT_Key_Less key_less(&counters.compares);
    RBT_Access access(&counters.accesses);
    RBT_Assign assign(&counters.assigns);
    RB_Tree<i64, RBT_Key_Less, RBT_Access, RBT_Assign> bst(key_less, access, assign);

    i64 n;
    std::cin >> n;

    RBT_Node** const buffer_height = (RBT_Node**)malloc(n * sizeof(RBT_Node*));
    if(n <= max_print) {
        for(i64 i = 0; i < n; ++i) {
            i64 a;
            std::cin >> a;
            std::cout << "\ninsert " << a << "\n";
            bst.insert(a);
            std::cout << bst.height() << '\n';
            print_RBT(bst.root());
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

    if(k <= max_print) {
        for(i64 i = 0; i < k; ++i) {
            i64 a;
            std::cin >> a;
            RBT_Node const* node = bst.find(a);
            if(node != nullptr) {
                std::cout << "\nerase " << a << "\n";
                bst.erase(node);
                std::cout << bst.height() << '\n';
                print_RBT(bst.root());
            }
        }
    } else {
        for(i64 i = 0; i < k; ++i) {
            i64 a;
            std::cin >> a;
            RBT_Node const* node = bst.find(a);
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
