#pragma once

#include <anton/iterators.hpp>
#include <anton/swap.hpp>
#include <anton/type_traits.hpp>

#include <glang_core/types.hpp>

namespace glang {
  template<typename T>
  struct IList_Node {
    T* next;
    T* prev;
  };

  template<typename T>
  void ilist_insert_before(IList_Node<T>* const position,
                           IList_Node<T>* const node)
  {
    T* const prev = position->prev;
    node->next = static_cast<T*>(position);
    position->prev = static_cast<T*>(node);
    node->prev = prev;
    prev->next = static_cast<T*>(node);
  }

  template<typename T>
  void ilist_insert_after(IList_Node<T>* const position,
                          IList_Node<T>* const node)
  {
    T* const next = position->next;
    node->prev = static_cast<T*>(position);
    position->next = static_cast<T*>(node);
    node->next = next;
    next->prev = static_cast<T*>(node);
  }

  template<typename T>
  void ilist_erase(IList_Node<T>* const node)
  {
    T* const next = node->next;
    T* const prev = node->prev;
    prev->next = next;
    next->prev = prev;
  }

  template<typename T>
  void ilist_splice(IList_Node<T>* position, IList_Node<T>* begin,
                    IList_Node<T>* end)
  {
    while(begin != end) {
      IList_Node<T>* next = begin->next;
      ilist_erase(begin);
      ilist_insert_after(position, begin);
      position = begin;
      begin = next;
    }
  }

  template<typename Node_Type>
  struct IList_Iterator {
  public:
    using value_type = Node_Type;
    using node_type = Node_Type;
    using pointer = value_type*;
    using reference = value_type*;
    using difference_type = i64;
    using iterator_category = anton::Bidirectional_Iterator_Tag;

    node_type* node = nullptr;

    IList_Iterator() = default;
    IList_Iterator(node_type* node): node(node) {}
    IList_Iterator(IList_Iterator const&) = default;
    IList_Iterator(IList_Iterator&&) = default;
    ~IList_Iterator() = default;
    IList_Iterator& operator=(IList_Iterator const&) = default;
    IList_Iterator& operator=(IList_Iterator&&) = default;

    // Conversion operator to the const version of the iterator.
    [[nodiscard]] operator IList_Iterator<node_type const>() const
    {
      return node;
    }

    [[nodiscard]] pointer operator->() const
    {
      return static_cast<node_type*>(node);
    }

    [[nodiscard]] reference operator*() const
    {
      return static_cast<node_type*>(node);
    }

    IList_Iterator& operator++()
    {
      node = node->next;
      return *this;
    }

    [[nodiscard]] IList_Iterator operator++(int)
    {
      IList_Iterator v{node};
      node = node->next;
      return v;
    }

    IList_Iterator& operator--()
    {
      node = node->prev;
      return *this;
    }

    [[nodiscard]] IList_Iterator operator--(int)
    {
      IList_Iterator v{node};
      node = node->prev;
      return v;
    }

    [[nodiscard]] bool operator==(IList_Iterator const& other) const
    {
      return node == other.node;
    }

    [[nodiscard]] bool operator!=(IList_Iterator const& other) const
    {
      return node != other.node;
    }
  };

  template<typename Node>
  struct IList {
  public:
    using size_type = i64;
    using value_type = Node;
    using node_type = anton::remove_const<Node>;
    using reference = Node&;
    using pointer = Node*;
    using iterator = IList_Iterator<Node>;
    using const_iterator = IList_Iterator<Node const>;

  private:
    IList_Node<node_type> _internal_node;

  public:
    IList();
    IList(IList const& other) = delete;
    IList(IList&& other);
    IList& operator=(IList const& other) = delete;
    IList& operator=(IList&& other);
    ~IList() = default;

    [[nodiscard]] iterator begin();
    [[nodiscard]] iterator end();
    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
    [[nodiscard]] const_iterator cbegin() const;
    [[nodiscard]] const_iterator cend() const;

    // size
    // Calculates the size of the list by traversing the nodes.
    //
    // Returns:
    // The number of nodes in the list.
    //
    // Complexity:
    // O(n) where n is the number of nodes in the list.
    //
    [[nodiscard]] size_type size() const;

    // insert
    // Insert a node before position.
    //
    // Parameters:
    // position - iterator to the node before which to insert the new nodes.
    //     node - the new node to be inserted.
    //
    // Returns:
    // Iterator to the inserted node.
    //
    iterator insert(const_iterator position, pointer node);

    // insert_front
    // Insert a node at the front of the list.
    //
    // Parameters:
    // node - the new node to be inserted.
    //
    // Returns:
    // Iterator to the inserted node.
    //
    iterator insert_front(pointer node);

    // insert_back
    // Insert a node at the back of the list.
    //
    // Parameters:
    // node - the new node to be inserted.
    //
    // Returns:
    // Iterator to the inserted node.
    //
    iterator insert_back(pointer node);

    // splice
    //
    iterator splice(iterator position, IList& other);

    // erase
    // Erase the node at position.
    //
    // Parameters:
    // position - iterator to the node to erase.
    //
    void erase(const_iterator position);

    // erase
    // Erase a range of nodes defined by [first, last[.
    //
    // Parameters:
    // first - the iterator to the beginning of the range.
    //  last - the iterator past the end of the range.
    //
    void erase(const_iterator first, const_iterator last);

    // erase_front
    // Erase the first node from the list.
    // If the list is empty, the behaviour is undefined.
    //
    void erase_front();

    // erase_back
    // Erase the last node from the list.
    // If the list is empty, the behaviour is undefined.
    //
    void erase_back();

    // clear
    // Unlink all nodes present in the list.
    //
    void clear();

    // swap
    // Swaps the contents of 2 lists.
    //
    // Compexity:
    // O(1)
    //
    friend void swap(IList& list1, IList& list2)
    {
      using anton::swap;
      // We swap the internal nodes and then fix up the addresses.
      swap(list1._internal_node, list2._internal_node);
      if(list1._internal_node.next == &list2._internal_node) {
        list1._internal_node.next = static_cast<pointer>(&list1._internal_node);
        list1._internal_node.prev = static_cast<pointer>(&list1._internal_node);
      } else {
        list1._internal_node.next->prev =
          static_cast<pointer>(&list1._internal_node);
        list1._internal_node.prev->next =
          static_cast<pointer>(&list1._internal_node);
      }

      if(list2._internal_node.next == &list1._internal_node) {
        list2._internal_node.next = static_cast<pointer>(&list2._internal_node);
        list2._internal_node.prev = static_cast<pointer>(&list2._internal_node);
      } else {
        list2._internal_node.next->prev =
          static_cast<pointer>(&list2._internal_node);
        list2._internal_node.prev->next =
          static_cast<pointer>(&list2._internal_node);
      }
    }
  };

  template<typename Node>
  IList<Node>::IList()
  {
    Node* const internal_node = static_cast<Node*>(&_internal_node);
    internal_node->next = internal_node;
    internal_node->prev = internal_node;
  }

  template<typename Node>
  IList<Node>::IList(IList&& other)
  {
    Node* const internal_node = static_cast<Node*>(&_internal_node);
    internal_node->next = internal_node;
    internal_node->prev = internal_node;
    swap(*this, other);
  }

  template<typename Node>
  IList<Node>& IList<Node>::operator=(IList&& other)
  {
    Node* const internal_node = static_cast<Node*>(&_internal_node);
    internal_node->next = internal_node;
    internal_node->prev = internal_node;
    swap(*this, other);
  }

  template<typename Node>
  auto IList<Node>::begin() -> iterator
  {
    return static_cast<node_type*>(_internal_node.next);
  }

  template<typename Node>
  auto IList<Node>::end() -> iterator
  {
    return static_cast<node_type*>(&_internal_node);
  }

  template<typename Node>
  auto IList<Node>::begin() const -> const_iterator
  {
    return static_cast<node_type const*>(_internal_node.next);
  }

  template<typename Node>
  auto IList<Node>::end() const -> const_iterator
  {
    return static_cast<node_type const*>(&_internal_node);
  }

  template<typename Node>
  auto IList<Node>::cbegin() const -> const_iterator
  {
    return static_cast<node_type const*>(_internal_node.next);
  }

  template<typename Node>
  auto IList<Node>::cend() const -> const_iterator
  {
    return static_cast<node_type const*>(&_internal_node);
  }

  template<typename Node>
  auto IList<Node>::size() const -> size_type
  {
    size_type _size = 0;
    Node const* const internal_node = static_cast<Node const*>(&_internal_node);
    Node const* node = internal_node->next;
    while(node != internal_node) {
      node = node->next;
      ++_size;
    }
    return _size;
  }

  template<typename Node>
  auto IList<Node>::insert(const_iterator position, pointer node) -> iterator
  {
    Node* const next = position.node;
    Node* const prev = next->prev;
    node->prev = prev;
    prev->next = &node;
    node->next = next;
    next->prev = &node;
    return &node;
  }

  template<typename Node>
  auto IList<Node>::insert_front(pointer node) -> iterator
  {
    Node* const next = static_cast<Node*>(&_internal_node);
    Node* const prev = next->prev;
    node->prev = prev;
    prev->next = &node;
    node->next = next;
    next->prev = &node;
    return &node;
  }

  template<typename Node>
  auto IList<Node>::insert_back(pointer node) -> iterator
  {
    Node* const next = static_cast<Node*>(&_internal_node);
    Node* const prev = next->prev;
    node->prev = prev;
    prev->next = node;
    node->next = next;
    next->prev = node;
    return node;
  }

  template<typename Node>
  auto IList<Node>::splice(iterator position, IList& other) -> iterator
  {
    Node* const current = position.node;
    Node* const prev = current->prev;
    Node* const first = other._internal_node.next;
    Node* const last = other._internal_node.prev;
    if(other.size() > 0) {
      prev->next = first;
      first->prev = prev;
      last->next = current;
      current->prev = last;
    }
    other._internal_node.next = reinterpret_cast<Node*>(&other._internal_node);
    other._internal_node.prev = reinterpret_cast<Node*>(&other._internal_node);
    return prev->next;
  }

  template<typename Node>
  void IList<Node>::erase(const_iterator position)
  {
    Node* const node = position.node;
    Node* const next = node->next;
    Node* const prev = node->prev;
    next->prev = prev;
    prev->next = next;
  }

  template<typename Node>
  void IList<Node>::erase(const_iterator first, const_iterator last)
  {
    Node* const next = last.node->next;
    Node* const prev = first.node->prev;
    next->prev = prev;
    prev->next = next;
  }

  template<typename Node>
  void IList<Node>::erase_front()
  {
    Node* const prev = static_cast<Node*>(&_internal_node);
    Node* const node = prev->next;
    Node* const next = node->next;
    next->prev = prev;
    prev->next = next;
  }

  template<typename Node>
  void IList<Node>::erase_back()
  {
    Node* const next = static_cast<Node*>(&_internal_node);
    Node* const node = next->prev;
    Node* const prev = node->prev;
    next->prev = prev;
    prev->next = next;
  }

  template<typename Node>
  void IList<Node>::clear()
  {
    Node* const internal_node = static_cast<Node*>(&_internal_node);
    _internal_node.next = internal_node;
    _internal_node.prev = internal_node;
  }
} // namespace glang
