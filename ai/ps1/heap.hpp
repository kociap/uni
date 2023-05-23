#pragma once

#include <algorithm>
#include <vector>

#include <types.hpp>

template<typename T, typename Heap_Compare, typename Tree_Compare>
struct Heap {
  private:
  enum struct Color {
    black,
    red,
  };

  struct Node {
    T value;

    Node* tree_parent = nullptr;
    Node* tree_left = nullptr;
    Node* tree_right = nullptr;

    Node* heap_parent = nullptr;
    Node* heap_left = nullptr;
    Node* heap_right = nullptr;

    Color tree_color = Color::black;

    template<typename... Args>
    Node(Args&&... args): value(std::forward<Args>(args)...) {}
  };

  Node* tree_root = nullptr;
  Node* heap_root = nullptr;
  i64 nodes = 0;
  Heap_Compare heap_compare;
  Tree_Compare tree_compare;

  public:
  struct iterator {
    friend struct Heap;

private:
    Node* node = nullptr;

    iterator(Node* node): node(node) {}

public:
    iterator() = default;

    [[nodiscard]] T& operator*() const {
      return node->value;
    }

    [[nodiscard]] T& operator->() const {
      return node->value;
    }

    [[nodiscard]] bool operator==(iterator const& other) const {
      return node == other.node;
    }

    [[nodiscard]] bool operator!=(iterator const& other) const {
      return node != other.node;
    }
  };

  public:
  Heap(Heap_Compare heap_compare, Tree_Compare tree_compare)
    : heap_compare(heap_compare), tree_compare(tree_compare) {}

  ~Heap() {
    if(heap_root != nullptr) {
      free_subheap(heap_root);
    }
  }

  [[nodiscard]] iterator end() {
    return iterator();
  }

  template<typename... Args>
  void insert(Args&&... args) {
    Node* const node = new Node(std::forward<Args>(args)...);
    insert_heap(node);
    // insert_tree(node);
    nodes += 1;
  }

  [[nodiscard]] T extract() {
    Node* const node = heap_root;
    erase_heap(node);
    // erase_tree(node);
    T value = std::move(node->value);
    delete node;
    nodes -= 1;
    return value;
  }

  // decrease
  // Update the heap to reflect the decrease in the key.
  //
  void decrease(iterator const i) {
    Node* const node = i.node;
    while(node->heap_parent != nullptr &&
          heap_compare(node->value, node->heap_parent->value)) {
      swap_heap_parent_child(node->heap_parent, node);
    }
  }

  template<typename Key>
  iterator find(Key const& key) {
    Node* node = tree_root;
    while(node != nullptr) {
      if(tree_compare(key, node->value)) {
        node = node->tree_left;
      } else if(tree_compare(node->value, key)) {
        node = node->tree_right;
      } else {
        break;
      }
    }
    return node;
  }

  [[nodiscard]] i64 size() const {
    return nodes;
  }

  private:
  void swap_heap_parent_child(Node* const parent, Node* const child) {
    std::swap(parent->heap_parent, child->heap_parent);
    std::swap(parent->heap_left, child->heap_left);
    std::swap(parent->heap_right, child->heap_right);

    parent->heap_parent = child;
    // One of the child pointers in child now point to itself.
    Node* sibling = nullptr;
    if(child->heap_left == child) {
      child->heap_left = parent;
      sibling = child->heap_right;
    } else {
      child->heap_right = parent;
      sibling = child->heap_left;
    }

    if(sibling != nullptr) {
      sibling->heap_parent = child;
    }

    if(parent->heap_left != nullptr) {
      parent->heap_left->heap_parent = parent;
    }

    if(parent->heap_right != nullptr) {
      parent->heap_right->heap_parent = parent;
    }

    if(child->heap_parent == nullptr) {
      heap_root = child;
    } else {
      if(child->heap_parent->heap_left == parent) {
        child->heap_parent->heap_left = child;
      } else {
        child->heap_parent->heap_right = child;
      }
    }
  }

  void insert_heap(Node* const node) {
    // Calculate the path from the root to the insertion point. Because this
    // is a min heap, we always insert at the left-most free slot of the
    // bottom row.
    //
    // The path to the insertion point is defined by the binary
    // representation of the index of the insertion point which is always
    // nodes + 1. Then, having first discarded the most significant bit, we
    // follow the path taking left when a bit is 0 and right when it is 1.
    //
    // Inspired by libuv https://github.com/libuv/libuv/blob/v1.x/src/heap-inl.h
    //
    u64 path = 0;
    i64 path_length = 0;
    for(i64 n = nodes + 1; n >= 2; path_length += 1, n /= 2) {
      path = (path << 1) | (n & 1);
    }

    Node** parent = &heap_root;
    Node** child = &heap_root;
    while(path_length > 0) {
      parent = child;
      if(path & 1) {
        child = &(*child)->heap_right;
      } else {
        child = &(*child)->heap_left;
      }
      path >>= 1;
      path_length -= 1;
    }

    // Insert the new node.
    node->heap_parent = *parent;
    *child = node;

    // Verify the heap property is maintained by traversing the heap up.
    while(node->heap_parent != nullptr &&
          heap_compare(node->value, node->heap_parent->value)) {
      swap_heap_parent_child(node->heap_parent, node);
    }
  }

  void erase_heap(Node* const node) {
    // Calculate path to the last node. Explanation of the ideas and the
    // algorithm is in insert_heap.
    u64 path = 0;
    i64 path_length = 0;
    for(i64 n = nodes; n >= 2; path_length += 1, n /= 2) {
      path = (path << 1) | (n & 1);
    }

    Node** max = &heap_root;
    while(path_length > 0) {
      if(path & 1) {
        max = &(*max)->heap_right;
      } else {
        max = &(*max)->heap_left;
      }
      path >>= 1;
      path_length -= 1;
    }

    // Unlink the max node.
    Node* unlinked = *max;
    *max = nullptr;

    if(unlinked == node) {
      // We are either removing the max node or the last node in the tree.
      if(unlinked == heap_root) {
        heap_root = nullptr;
      }
      return;
    }

    // Replace the node to be deleted with the max node.
    unlinked->heap_left = node->heap_left;
    unlinked->heap_right = node->heap_right;
    unlinked->heap_parent = node->heap_parent;

    if(unlinked->heap_left != nullptr) {
      unlinked->heap_left->heap_parent = unlinked;
    }

    if(unlinked->heap_right != nullptr) {
      unlinked->heap_right->heap_parent = unlinked;
    }

    if(unlinked->heap_parent == nullptr) {
      heap_root = unlinked;
    } else {
      if(unlinked->heap_parent->heap_left == node) {
        unlinked->heap_parent->heap_left = unlinked;
      } else {
        unlinked->heap_parent->heap_right = unlinked;
      }
    }

    // Verify the heap property by traversing the heap down.
    while(true) {
      Node* smallest = unlinked;
      if(unlinked->heap_left != nullptr &&
         heap_compare(unlinked->heap_left->value, smallest->value)) {
        smallest = unlinked->heap_left;
      }
      if(unlinked->heap_right != nullptr &&
         heap_compare(unlinked->heap_right->value, smallest->value)) {
        smallest = unlinked->heap_right;
      }

      if(smallest == unlinked) {
        break;
      }

      swap_heap_parent_child(unlinked, smallest);
    }
  }

  void insert_tree(Node* const node) {
    Node* parent = tree_root;
    {
      Node* child = tree_root;
      while(child != nullptr) {
        parent = child;
        if(tree_compare(node->value, child->value)) {
          child = child->tree_left;
        } else {
          child = child->tree_right;
        }
      }
    }

    node->tree_color = Color::red;
    if(parent == nullptr) {
      tree_root = node;
    } else {
      node->tree_parent = parent;
      if(tree_compare(node->value, parent->value)) {
        parent->tree_left = node;
      } else {
        parent->tree_right = node;
      }
    }
    tree_insert_fixup(node);
  }

  void erase_tree(Node const* const node) {
    Color original_color = node->tree_color;
    Node* x = nullptr;
    if(node->tree_left == nullptr) {
      x = node->tree_right;
      tree_transplant(node, node->tree_right);
    } else if(node->tree_right == nullptr) {
      x = node->tree_left;
      tree_transplant(node, node->tree_left);
    } else {
      Node* y = node->tree_right;
      // Tree minimum.
      while(y->tree_left != nullptr) {
        y = y->tree_left;
      }
      original_color = y->tree_color;
      x = y->tree_right;
      if(y->tree_parent == node) {
        if(x) {
          x->tree_parent = y;
        }
      } else {
        tree_transplant(y, y->tree_right);
        y->tree_right = node->tree_right;
        y->tree_right->tree_parent = y;
      }
      tree_transplant(node, y);
      y->tree_left = node->tree_left;
      y->tree_left->tree_parent = y;
      y->tree_color = node->tree_color;
    }

    if(x && original_color == Color::black) {
      tree_erase_fixup(x);
    }
  }

  void tree_transplant(Node const* const u, Node* const v) {
    if(u->tree_parent == nullptr) {
      tree_root = v;
    } else if(u == u->tree_parent->tree_left) {
      u->tree_parent->tree_left = v;
    } else {
      u->tree_parent->tree_right = v;
    }
    if(v != nullptr) {
      v->tree_parent = u->tree_parent;
    }
  }

  void left_rotate(Node* const x) {
    if(x->tree_right == nullptr) {
      return;
    }

    Node* const y = x->tree_right;
    x->tree_right = y->tree_left;
    if(x->tree_right != nullptr) {
      x->tree_right->tree_parent = x;
    }
    if(x->tree_parent == nullptr) {
      tree_root = y;
    } else if(x->tree_parent->tree_left == x) {
      x->tree_parent->tree_left = y;
    } else {
      x->tree_parent->tree_right = y;
    }
    y->tree_parent = x->tree_parent;
    y->tree_left = x;
    x->tree_parent = y;
  }

  void right_rotate(Node* const y) {
    if(y->tree_left == nullptr) {
      return;
    }

    Node* const x = y->tree_left;
    y->tree_left = x->tree_right;
    if(y->tree_left != nullptr) {
      y->tree_left->tree_parent = y;
    }
    if(y->tree_parent == nullptr) {
      tree_root = x;
    } else if(y->tree_parent->tree_left == y) {
      y->tree_parent->tree_left = x;
    } else {
      y->tree_parent->tree_right = x;
    }
    x->tree_parent = y->tree_parent;
    x->tree_right = y;
    y->tree_parent = x;
  }

  void tree_insert_fixup(Node* node) {
    while(node->tree_parent && node->tree_parent->tree_color == Color::red) {
      if(node->tree_parent == node->tree_parent->tree_parent->tree_left) {
        Node* const uncle = node->tree_parent->tree_parent->tree_right;
        if(uncle && uncle->tree_color == Color::red) {
          node->tree_parent->tree_color = Color::black;
          uncle->tree_color = Color::black;
          node->tree_parent->tree_parent->tree_color = Color::red;
          node = node->tree_parent->tree_parent;
        } else {
          if(node == node->tree_parent->tree_right) {
            node = node->tree_parent;
            left_rotate(node);
          }
          node->tree_parent->tree_color = Color::black;
          node->tree_parent->tree_parent->tree_color = Color::red;
          right_rotate(node->tree_parent->tree_parent);
        }
      } else {
        Node* const uncle = node->tree_parent->tree_parent->tree_left;
        if(uncle && uncle->tree_color == Color::red) {
          node->tree_parent->tree_color = Color::black;
          uncle->tree_color = Color::black;
          node->tree_parent->tree_parent->tree_color = Color::red;
          node = node->tree_parent->tree_parent;
        } else {
          if(node == node->tree_parent->tree_left) {
            node = node->tree_parent;
            right_rotate(node);
          }
          node->tree_parent->tree_color = Color::black;
          node->tree_parent->tree_parent->tree_color = Color::red;
          left_rotate(node->tree_parent->tree_parent);
        }
      }
    }
    tree_root->tree_color = Color::black;
  }

  void tree_erase_fixup(Node* node) {
    while(node != tree_root && node->tree_color == Color::black) {
      // w is never a nullptr because if node has a parent, then node is either its left or right child.
      if(node == node->tree_parent->tree_right) {
        Node* w = node->tree_parent->tree_right;
        if(w->tree_color == Color::red) {
          w->tree_color = Color::black;
          node->tree_parent->tree_color = Color::red;
          left_rotate(node->tree_parent);
          w = node->tree_parent->tree_right;
        }
        if((w->tree_left == nullptr ||
            w->tree_left->tree_color == Color::black) &&
           (w->tree_right == nullptr ||
            w->tree_right->tree_color == Color::black)) {
          w->tree_color = Color::red;
          node = node->tree_parent;
        } else {
          // If the right child is black, then the left child must exist and be red.
          if((w->tree_right == nullptr ||
              w->tree_right->tree_color == Color::black)) {
            w->tree_left->tree_color = Color::black;
            w->tree_color = Color::red;
            right_rotate(w);
            w = node->tree_parent->tree_right;
          }
          w->tree_color = node->tree_parent->tree_color;
          node->tree_parent->tree_color = Color::black;
          if(w->tree_right) {
            w->tree_right->tree_color = Color::black;
          }
          left_rotate(node->tree_parent);
          node = tree_root;
        }
      } else {
        Node* w = node->tree_parent->tree_left;
        if(w->tree_color == Color::red) {
          w->tree_color = Color::black;
          node->tree_parent->tree_color = Color::red;
          left_rotate(node->tree_parent);
          w = node->tree_parent->tree_left;
        }
        if((w->tree_right == nullptr ||
            w->tree_right->tree_color == Color::black) &&
           (w->tree_left == nullptr ||
            w->tree_left->tree_color == Color::black)) {
          w->tree_color = Color::red;
          node = node->tree_parent;
        } else {
          // If the left child is black, then the right child must exist and be red.
          if((w->tree_left == nullptr ||
              w->tree_left->tree_color == Color::black)) {
            w->tree_right->tree_color = Color::black;
            w->tree_color = Color::red;
            left_rotate(w);
            w = node->tree_parent->tree_left;
          }
          w->tree_color = node->tree_parent->tree_color;
          node->tree_parent->tree_color = Color::black;
          if(w->tree_left) {
            w->tree_left->tree_color = Color::black;
          }
          right_rotate(node->tree_parent);
          node = tree_root;
        }
      }
    }
    node->tree_color = Color::black;
  }

  void free_subheap(Node* const node) {
    if(node->heap_left) {
      free_subheap(node->heap_left);
    }

    if(node->heap_right) {
      free_subheap(node->heap_right);
    }

    delete node;
  }
};
