#include <graph.hpp>

#include <array>
#include <deque>
#include <math.h>
#include <stdio.h>

[[nodiscard]] static i32 min(i32 a, i32 b) {
  return a < b ? a : b;
}

[[nodiscard]] static i32 max(i32 a, i32 b) {
  return a > b ? a : b;
}

struct Node {
  Vertex const* vertex;
  i32 label;
};

struct Radix_Heap {
public:
  using bucket_t = std::vector<Node>;
  using value_t = Vertex const*;

private:
  constexpr static i32 bucket_count = sizeof(i32) * 8 + 1;
  std::array<bucket_t, bucket_count> _buckets = {};
  std::array<i32, bucket_count> _bucket_minimum;
  i32 _least = 0;
  i32 _size = 0;

public:
  Radix_Heap() {
    _bucket_minimum.fill(maximum_i32);
  }

  void insert(Vertex const* const vertex, i32 const label) {
    _size += 1;
    i32 const bucket = find_bucket(label);
    _buckets[bucket].push_back(Node(vertex, label));
    _bucket_minimum[bucket] = min(_bucket_minimum[bucket], label);
  }

  value_t extract() {
    _size -= 1;
    pull();
    Node value = _buckets[0].back();
    _buckets[0].pop_back();
    return value.vertex;
  }

  [[nodiscard]] i32 size() const {
    return _size;
  }

private:
  void pull() {
    if(_buckets[0].size() > 0) {
      return;
    }

    // Find non-empty bucket.
    i32 i = 1;
    while(_buckets[i].size() == 0) {
      i += 1;
    }

    _least = _bucket_minimum[i];

    for(Node& v: _buckets[i]) {
      i32 const bucket = find_bucket(v.label);
      _buckets[bucket].push_back(v);
      _bucket_minimum[bucket] = min(_bucket_minimum[bucket], v.label);
    }
    _buckets[i].clear();
    _bucket_minimum[i] = maximum_i32;
  }

  [[nodiscard]] i32 find_bucket(i32 const value) {
    constexpr i32 bits = sizeof(i32) * 8;
    i32 const zeros = __builtin_clz(value ^ _least);
    return value == _least ? 0 : bits - zeros;
  }
};

Result shortest_path_radix(Graph const& graph, Vertex const& source) {
  // We initialise to maximum value of i32 to indicate an infinity. We cannot
  // use maximum value of i64 becasue that will lead to overflow and erroneous
  // behaviour in the later part of the algorithm.
  std::vector<i64> distances(graph.vertices.size(), maximum_i32);
  std::vector<i32> parents(graph.vertices.size(), -1);
  std::vector<char> visited(graph.vertices.size(), false);
  distances[source.index] = 0;

  i32 max_weight = 0;
  for(Vertex const& v: graph.vertices) {
    for(Edge const& e: v.edges) {
      max_weight = max(max_weight, e.weight);
    }
  }

  Radix_Heap heap;
  heap.insert(&source, 0);
  while(heap.size() > 0) {
    Vertex const* const vertex = heap.extract();
    i32 const index = vertex->index;
    if(visited[index] == true) {
      continue;
    }

    visited[index] = true;
    i64 const distance = distances[index];
    for(Edge const& edge: vertex->edges) {
      i32 const updated_distance = distance + edge.weight;
      if(updated_distance < distances[edge.dst]) {
        distances[edge.dst] = updated_distance;
        parents[edge.dst] = index;
        Vertex const* const dst = &graph.vertices[edge.dst];
        heap.insert(dst, updated_distance);
      }
    }
  }
  return Result{std::move(distances), std::move(parents)};
}
