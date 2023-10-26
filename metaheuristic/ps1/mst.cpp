#include <mst.hpp>

#include <anton/algorithm/sort.hpp>

[[nodiscard]] static Vertex* get_root(Vertex* vertex)
{
  while(vertex->data != nullptr) {
    vertex = reinterpret_cast<Vertex*>(vertex->data);
  }
  return vertex;
}

anton::Array<Edge> compute_mst_kruskal(Graph& graph)
{
  for(Vertex& vertex: graph.vertices) {
    vertex.data = nullptr;
  }

  anton::quick_sort(
    graph.edges.begin(), graph.edges.end(),
    [](Edge const& e1, Edge const& e2) { return e1.weight < e2.weight; });

  anton::Array<Edge> result;
  for(Edge const& edge: graph.edges) {
    Vertex& src = graph.vertices[edge.src];
    Vertex& dst = graph.vertices[edge.dst];
    Vertex* const src_root = get_root(&src);
    Vertex* const dst_root = get_root(&dst);
    if(src_root == dst_root) {
      // Vertices are already in the same tree.
      continue;
    }

    src_root->data = dst_root;
    result.push_back(edge);
  }

  return result;
}
