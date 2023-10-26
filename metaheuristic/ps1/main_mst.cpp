#include <anton/console.hpp>
#include <anton/format.hpp>
#include <anton/string.hpp>

#include <loader.hpp>
#include <mst.hpp>

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

  // MST weight
  i64 mst_weight = 0;
  for(Edge const& edge: mst_edges) {
    mst_weight += edge.weight;
  }

  // stdout.write(anton::format("{}\n"_sv, mst_weight));

  for(Edge const& edge: mst_edges) {
    Vertex& src = graph.vertices[edge.src];
    Vertex& dst = graph.vertices[edge.dst];
    stdout.write(anton::format("{} {} {} {}\n"_sv, src.x, src.y, dst.x, dst.y));
  }

  return 0;
}
