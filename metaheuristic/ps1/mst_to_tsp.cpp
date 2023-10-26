#include <mst_to_tsp.hpp>

#include <anton/array.hpp>

#define VISITED reinterpret_cast<void*>(0x1)

[[nodiscard]] static anton::Array<Vertex*> dfs(Graph& graph)
{
  anton::Array<Vertex*> path;
  anton::Array<Vertex*> stack{anton::variadic_construct, &graph.vertices[0]};
  while(stack.size() > 0) {
    Vertex* vertex = stack.back();
    stack.pop_back();
    if(vertex->data == VISITED) {
      continue;
    }

    vertex->data = VISITED;

    path.push_back(vertex);

    for(Edge const& edge: vertex->edges) {
      Vertex* next = graph.vertices.data() + edge.dst;
      stack.push_back(next);
    }
  }

  return path;
}

anton::Array<Edge> tsp_cycle_from_mst(Graph& graph)
{
  anton::Array<Edge> cycle;
  anton::Array<Vertex*> path = dfs(graph);
  Edge const starting_edge = calculate_edge_between(*path.back(), *path[0]);
  cycle.push_back(starting_edge);
  auto first = path.begin();
  auto second = first + 1;
  auto end = path.end();
  for(; second != end; first += 1, second += 1) {
    Edge const edge = calculate_edge_between(**first, **second);
    cycle.push_back(edge);
  }
  return cycle;
}
