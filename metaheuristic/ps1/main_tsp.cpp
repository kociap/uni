#include <anton/console.hpp>
#include <anton/format.hpp>
#include <anton/string.hpp>

#include <loader.hpp>
#include <mst.hpp>
#include <mst_to_tsp.hpp>

using namespace anton::literals;

int main(int argc, char** argv)
{
  anton::STDOUT_Stream stdout;
  anton::STDERR_Stream stderr;

  if(argc < 2) {
    stderr.write(anton::format("{} <file>\n"_sv, argv[0]));
    return 1;
  }

  anton::String file(argv[1]);
  Graph graph = load_graph(file);
  anton::Array<Edge> mst_edges = compute_mst_kruskal(graph);

  for(Edge const& edge: mst_edges) {
    graph.vertices[edge.src].edges.push_back(edge);
    Edge const reverse{.src = edge.dst, .dst = edge.src, .weight = edge.weight};
    graph.vertices[edge.dst].edges.push_back(reverse);
  }

  anton::Array<Edge> cycle = tsp_cycle_from_mst(graph);
  i64 cycle_weight = 0;
  for(Edge const& edge: cycle) {
    cycle_weight += edge.weight;
  }

  // stdout.write(anton::format("{}\n", cycle_weight));

  for(Edge const& edge: cycle) {
    Vertex& src = graph.vertices[edge.src];
    Vertex& dst = graph.vertices[edge.dst];
    stdout.write(anton::format("{} {} {} {}\n"_sv, src.x, src.y, dst.x, dst.y));
  }

  return 0;
}
