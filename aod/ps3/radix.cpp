#include <graph.hpp>

#include <math.h>
#include <stdio.h>
#include <vector>

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

struct Bucket {
public:
  using value_t = Node;

  std::vector<value_t> values;
  i32 high;
  i32 low;

  [[nodiscard]] i32 get_width() const {
    return high - low + 1;
  }
};

struct Radix_Heap {
public:
  using bucket_t = Bucket;
  using value_t = Bucket::value_t;

private:
  std::vector<bucket_t> _buckets;
  i32 _size = 0;

public:
  Radix_Heap(i32 const max_label) {
    i32 const bucket_count = ceil(log2(max_label + 1));
    _buckets.resize(bucket_count);
  }

  void insert(Vertex const* const vertex, i32 const label) {
    _size += 1;
    for(bucket_t& bucket: _buckets) {
      if(label >= bucket.low) {
        bucket.values.push_back(Node(vertex, label));
        return;
      }
    }
    printf("error: no bucket found for label %d\n", label);
  }

  Vertex const* extract() {
    _size -= 1;
    bucket_t* non_empty = nullptr;
    for(bucket_t& bucket: _buckets) {
      if(bucket.values.size() > 0) {
        non_empty = &bucket;
        break;
      }
    }

    // non_empty is not a nullptr at this point unless the user violated our
    // invariants and called extract when there are 0 elements in the heap.
    if(non_empty->get_width() != 1) {
      redistribute(non_empty);
      non_empty = &_buckets[0];
    }

    value_t value = non_empty->values.back();
    non_empty->values.pop_back();
    return value.vertex;
  }

  [[nodiscard]] i32 size() const {
    return _size;
  }

private:
  void redistribute(bucket_t* const bucket) {
    // Find min label.
    i32 min_label = bucket->values[0].label;
    for(value_t const& value: bucket->values) {
      min_label = min(min_label, value.label);
    }

    i32 const index = bucket - _buckets.data();
    i32 increment = 0;
    i32 width = 1;
    i32 low = min_label;
    for(i32 i = 0; i < index; i += 1) {
      bucket_t& bucket = _buckets[i];
      bucket.low = low;
      bucket.high = low + width - 1;
      low += width;
      width += increment;
      increment *= 2;
      if(increment == 0) {
        increment += 1;
      }
    }

    for(value_t& v: bucket->values) {
      for(bucket_t& b: _buckets) {
        if(v.label >= b.low) {
          b.values.push_back(std::move(v));
          break;
        }
      }
    }
    _buckets.erase(_buckets.begin() + index);
  }
};

std::vector<i64> shortest_path_radix(Graph const& graph, Vertex const& source) {
  // We initialise to maximum value of i32 to indicate an infinity. We cannot
  // use maximum value of i64 becasue that will lead to overflow and erroneous
  // behaviour in the later part of the algorithm.
  std::vector<i64> distances(graph.vertices.size(), maximum_i32);
  std::vector<char> visited(graph.vertices.size(), false);
  distances[source.index] = 0;

  i32 max_weight = 0;
  for(Vertex const& v: graph.vertices) {
    for(Edge const& e: v.edges) {
      max_weight = max(max_weight, e.weight);
    }
  }

  Radix_Heap heap(max_weight);
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
        Vertex const* const dst = &graph.vertices[edge.dst];
        heap.insert(dst, updated_distance);
      }
    }
  }
  return distances;
}
