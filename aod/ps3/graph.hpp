#pragma once

#include <vector>

#include <types.hpp>

struct Edge {
  i32 dst;
  i64 weight;
};

struct Vertex {
  std::vector<Edge> edges;
  i32 index = 0;
};

struct Graph {
  std::vector<Vertex> vertices;
};

struct Result {
  std::vector<i64> distances;
  std::vector<i32> parents;
};

// shortest_path_dijkstra
// Find the lengths of the shortest paths from soruce to all other vertices.
//
Result shortest_path_dijkstra(Graph const& graph, Vertex const& source);

// shortest_path_dial
// Find the lengths of the shortest paths from soruce to all other vertices.
//
Result shortest_path_dial(Graph const& graph, Vertex const& source);

// shortest_path_radix
// Find the lengths of the shortest paths from soruce to all other vertices.
//
Result shortest_path_radix(Graph const& graph, Vertex const& source);
