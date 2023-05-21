#include <graph.hpp>

#include <vector>

struct Bucket_Queue {
private:
  using value_t = Vertex const*;
  using bucket_t = std::vector<value_t>;

  std::vector<bucket_t> _buckets;
  i32 _size = 0;

public:
  void insert(value_t const v, i32 const distance) {
    if(_buckets.size() <= distance) {
      _buckets.resize(distance + 1);
    }

    _buckets[distance].push_back(v);
    _size += 1;
  }

  [[nodiscard]] value_t extract() {
    for(i32 i = 0; bucket_t & bucket: _buckets) {
      if(bucket.size() > 0) {
        value_t v = bucket.back();
        bucket.pop_back();
        _size -= 1;
        return v;
      }
      i += 1;
    }

    return nullptr;
  }

  [[nodiscard]] i32 size() const {
    return _size;
  }
};

std::vector<i64> shortest_path_dial(Graph const& graph, Vertex const& source) {
  // We initialise to maximum value of i32 to indicate an infinity. We cannot
  // use maximum value of i64 becasue that will lead to overflow and erroneous
  // behaviour in the later part of the algorithm.
  std::vector<i64> distances(graph.vertices.size(), maximum_i32);
  std::vector<char> visited(graph.vertices.size(), false);
  distances[source.index] = 0;
  Bucket_Queue bq;
  bq.insert(&source, 0);

  while(bq.size() > 0) {
    Vertex const* const vertex = bq.extract();
    i32 const index = vertex->index;
    if(visited[index] == true) {
      continue;
    }

    visited[index] = true;

    i64 const distance = distances[index];
    for(Edge const edge: vertex->edges) {
      i64 const updated_distance = distance + edge.weight;
      if(updated_distance < distances[edge.dst]) {
        distances[edge.dst] = updated_distance;
        Vertex const* const vertex = &graph.vertices[edge.dst];
        bq.insert(vertex, edge.weight);
      }
    }
  }
  return distances;
}
