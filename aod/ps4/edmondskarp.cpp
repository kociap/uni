#include <edmondskarp.hpp>

#include <utility.hpp>

#include <queue>

i32 edmonds_karp(Graph& graph, i32 const source, i32 const sink,
                 i32& augmenting_paths) {
  i32 flow = 0;
  while(true) {
    std::queue<i32> queue;
    std::vector<Edge*> predecessor(graph.size(), nullptr);
    queue.push(source);
    while(queue.size() > 0) {
      i32 const current = queue.front();
      queue.pop();
      for(Edge& edge: graph[current]) {
        bool const no_predecessor = predecessor[edge.dst] == nullptr;
        bool const dst_not_source = edge.dst != source;
        bool const has_flow = edge.capacity > edge.flow;
        if(no_predecessor && dst_not_source && has_flow) {
          predecessor[edge.dst] = &edge;
          queue.push(edge.dst);
        }
      }
    }

    if(predecessor[sink] == nullptr) {
      // No augmenting path has been found. Terminate.
      return flow;
    }

    augmenting_paths += 1;
    i32 maximum_flow = maximum_i32;
    for(Edge* edge = predecessor[sink]; edge != nullptr;
        edge = predecessor[edge->src]) {
      maximum_flow = min(maximum_flow, edge->capacity - edge->flow);
    }

    for(Edge* edge = predecessor[sink]; edge != nullptr;
        edge = predecessor[edge->src]) {
      edge->flow += maximum_flow;
      if(edge->reverse_edge != -1) {
        Edge& reverse_edge = graph[edge->dst][edge->reverse_edge];
        reverse_edge.flow += maximum_flow;
      }
    }
    flow += maximum_flow;
  }
  return flow;
}
