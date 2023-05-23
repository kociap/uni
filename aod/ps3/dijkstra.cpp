#include <graph.hpp>

#include <queue>

struct Node {
  Vertex const* vertex;
  i64 distance;
};

[[nodiscard]] static bool priority_compare(Node const& lhs, Node const& rhs) {
  return lhs.distance > rhs.distance;
}

Result shortest_path_dijkstra(Graph const& graph, Vertex const& source) {
  // We initialise to maximum value of i32 to indicate an infinity. We cannot
  // use maximum value of i64 becasue that will lead to overflow and erroneous
  // behaviour in the later part of the algorithm.
  std::vector<i64> distances(graph.vertices.size(), maximum_i32);
  std::vector<i32> parents(graph.vertices.size(), -1);
  std::vector<char> visited(graph.vertices.size(), false);
  std::priority_queue<Node, std::vector<Node>,
                      bool (*)(Node const&, Node const&)>
    queue(priority_compare);
  distances[source.index] = 0;
  queue.push(Node(&source, 0));

  while(queue.size() > 0) {
    Node const node = queue.top();
    queue.pop();
    i32 const index = node.vertex->index;
    if(visited[index] == true) {
      continue;
    }

    visited[index] = true;

    i64 const distance = distances[index];
    for(Edge const edge: node.vertex->edges) {
      i64 const updated_distance = distance + edge.weight;
      if(updated_distance < distances[edge.dst]) {
        distances[edge.dst] = updated_distance;
        parents[edge.dst] = index;
        Vertex const* const vertex = &graph.vertices[edge.dst];
        queue.push(Node(vertex, updated_distance));
      }
    }
  }
  return Result{std::move(distances), std::move(parents)};
}
