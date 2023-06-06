#include <edmondskarp.hpp>
#include <graph.hpp>
#include <options.hpp>
#include <types.hpp>
#include <utility.hpp>

#include <stdlib.h>

#include <charconv>
#include <iostream>
#include <random>

static void help(char const* const name) {
  printf("Usage: %s [OPTION]...\n", name);
  printf("\n");
  printf("OPTION\n");
  printf(" -h, --help            display the help page\n");
  printf(
    " -s K, --size K        set the graph size of each partition to 2^K. K "
    "must be in [1, 16]\n");
  printf(
    " -d D, --degree D      the degree of the vertices in the partition 1\n");
  printf(" -p, --print-matching  print the edges within the matching\n");
}

[[nodiscard]] static Graph construct_graph(i32 const size, i32 const degree) {
  i32 const vert_count = 1 << size;
  std::random_device rd;
  std::mt19937 generator(rd());
  Graph graph;
  graph.vertices.resize(2 * vert_count + 2);
  for(i32 src = 0; src < vert_count; src += 1) {
    Edges& edges = graph.vertices[src];
    std::uniform_int_distribution<i32> distribution(0, vert_count - 1);
    for(i32 d = 0; d < degree; d += 1) {
      i32 const dst = vert_count + distribution(generator);
      if(has_edge(graph, src, dst)) {
        d -= 1;
        continue;
      }

      Edges& reverse_edges = graph.vertices[dst];

      Edge edge;
      edge.dst = dst;
      edge.src = src;
      edge.flow = 0;
      edge.capacity = 1;
      edge.reverse_edge = reverse_edges.size();

      Edge reverse_edge;
      reverse_edge.dst = src;
      reverse_edge.src = dst;
      reverse_edge.flow = 0;
      reverse_edge.capacity = 1;
      reverse_edge.reverse_edge = edges.size();

      edges.push_back(edge);
      reverse_edges.push_back(reverse_edge);
    }
  }

  i32 const source_index = 2 * vert_count;
  Edges& source_edges = graph.vertices[source_index];
  for(i32 v1 = 0; v1 < vert_count; v1 += 1) {
    Edge edge;
    edge.src = source_index;
    edge.dst = v1;
    edge.capacity = 1;
    source_edges.push_back(edge);
  }

  i32 const sink_index = 2 * vert_count + 1;
  for(i32 v2 = vert_count; v2 < 2 * vert_count; v2 += 1) {
    Edge edge;
    edge.src = v2;
    edge.dst = sink_index;
    edge.capacity = 1;
    graph.vertices[v2].push_back(edge);
  }

  return graph;
}

int main(int const argc, char const* const* const argv) {
  constexpr i32 RETURN_FAILURE = 1;
  constexpr i32 RETURN_SUCCESS = 0;
  constexpr i32 RETURN_HELP = 2;

  constexpr i32 option_help = 0;
  constexpr i32 option_size = 1;
  constexpr i32 option_print_matching = 2;
  constexpr i32 option_degree = 3;

  std::ios::sync_with_stdio(false);

  Timer timer;
  timer.start();

  Option_Definition const definitions[] = {
    {"-h", option_help, false},
    {"--help", option_help, false},
    // Size of the graph.
    {"-s", option_size, true},
    {"--size", option_size, true},
    {"-d", option_degree, true},
    {"--degree", option_degree, true},
    // Whether to print the matching edges.
    {"-p", option_print_matching, false},
    {"--print-matching", option_print_matching, false}};
  std::optional<Parse_Result> result = parse_options(definitions, argc, argv);
  if(!result) {
    return RETURN_FAILURE;
  }

  i32 size = 1;
  i32 degree = 1;
  bool print_matching = false;
  for(Option const& option: result->options) {
    switch(option.id) {
      case option_help:
        help(argv[0]);
        return RETURN_HELP;

      case option_size:
        std::from_chars(option.value.begin(), option.value.end(), size);
        break;

      case option_degree:
        std::from_chars(option.value.begin(), option.value.end(), degree);
        break;

      case option_print_matching:
        print_matching = true;
        break;
    }
  }

  Graph graph = construct_graph(size, degree);
  i32 augmenting_paths = 0;
  i32 const source_index = graph.size() - 2;
  i32 const sink_index = graph.size() - 1;
  edmonds_karp(graph, source_index, sink_index, augmenting_paths);
  i32 matchings = 0;
  for(Edges const& edges: graph.vertices) {
    for(Edge const& edge: edges) {
      if(edge.dst != sink_index && edge.flow > 0 && edge.src < edge.dst) {
        matchings += 1;
      }
    }
  }

  i64 const time = timer.end_us();
  std::cout << size << ',' << degree << ',';
  std::cout << matchings << ',';
  if(print_matching) {
    for(Edges const& edges: graph.vertices) {
      for(Edge const& edge: edges) {
        if(edge.dst != sink_index && edge.flow > 0 && edge.src < edge.dst) {
          std::cout << edge.src << " -> " << edge.dst << '\n';
        }
      }
    }
  }

  std::cout << time << "\n";
  // std::cerr << augmenting_paths << '\n';

  return RETURN_SUCCESS;
}
