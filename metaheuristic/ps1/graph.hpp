#pragma once

#include <anton/array.hpp>

#include <types.hpp>

struct Edge {
  i64 src;
  i64 dst;
  i64 weight;
};

struct Vertex {
  anton::Array<Edge> edges;
  void* data;
  i64 index;
  i64 x;
  i64 y;
};

struct Graph {
  anton::Array<Vertex> vertices;
  anton::Array<Edge> edges;
};

[[nodiscard]] Edge calculate_edge_between(Vertex const& src, Vertex const& dst);
