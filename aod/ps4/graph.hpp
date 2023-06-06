#pragma once

#include <types.hpp>

#include <vector>

struct Edge {
  i32 src = -1;
  i32 dst = -1;
  i32 reverse_edge = -1;
  i32 flow = 0;
  i32 capacity = 0;
};

using Edges = std::vector<Edge>;

struct Graph {
  std::vector<Edges> vertices;

  [[nodiscard]] Edges& operator[](i32 const index) {
    return vertices[index];
  }

  [[nodiscard]] i32 size() const {
    return vertices.size();
  }
};

[[nodiscard]] inline bool has_edge(Graph const& graph, i32 const src,
                                   i32 const dst) {
  for(Edge const& edge: graph.vertices[src]) {
    if(edge.dst == dst) {
      return true;
    }
  }
  return false;
}
