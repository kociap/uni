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
  printf(" -h, --help        display the help page\n");
  printf(
    " -s K, --size K    set the graph size to 2^K. K must be in [1, 16]\n");
  printf(" -p, --print-flow  print the flow at each edge\n");
}

[[nodiscard]] static Graph construct_graph(i32 const size) {
  std::random_device rd;
  std::mt19937 generator(rd());
  i32 const vert_count = 1 << size;
  Graph graph;
  graph.vertices.resize(vert_count);
  for(i32 src = 0; src < vert_count; src += 1) {
    Edges& edges = graph.vertices[src];
    for(i32 k = 0; k < size; k += 1) {
      i32 const dst = src | (1 << k);
      if(dst == src) {
        continue;
      }

      Edge edge;
      edge.dst = dst;
      edge.src = src;
      edge.flow = 0;
      // Calculate capacity.
      i32 const hamming_src = __builtin_popcount(src);
      i32 const zero_src = size - hamming_src;
      i32 const hamming_dst = __builtin_popcount(dst);
      i32 const zero_dst = size - hamming_dst;
      i32 const l = max(max(hamming_src, zero_src), max(hamming_dst, zero_dst));
      std::uniform_int_distribution<i32> distribution(1, 1 << l);
      edge.capacity = distribution(generator);

      edges.push_back(edge);
    }
  }
  return graph;
}

int main(int const argc, char const* const* const argv) {
  constexpr i32 RETURN_FAILURE = 1;
  constexpr i32 RETURN_SUCCESS = 0;
  constexpr i32 RETURN_HELP = 2;

  constexpr i32 option_help = 0;
  constexpr i32 option_size = 1;
  constexpr i32 option_print_flow = 2;

  std::ios::sync_with_stdio(false);

  Timer timer;
  timer.start();

  Option_Definition const definitions[] = {
    {"-h", option_help, false},
    {"--help", option_help, false},
    // Size of the graph.
    {"-s", option_size, true},
    {"--size", option_size, true},
    // Whether to print the flow on each edge.
    {"-p", option_print_flow, false},
    {"--print-flow", option_print_flow, false}};
  std::optional<Parse_Result> result = parse_options(definitions, argc, argv);
  if(!result) {
    return RETURN_FAILURE;
  }

  i32 size = 1;
  bool print_flow = false;
  for(Option const& option: result->options) {
    switch(option.id) {
      case option_help:
        help(argv[0]);
        return RETURN_HELP;

      case option_size:
        std::from_chars(option.value.begin(), option.value.end(), size);
        break;

      case option_print_flow:
        print_flow = true;
        break;
    }
  }

  Graph graph = construct_graph(size);
  i32 augmenting_paths = 0;
  i32 const maximum_flow =
    edmonds_karp(graph, 0, (1 << size) - 1, augmenting_paths);
  i64 const time = timer.end_us();
  std::cout << size << ',';
  std::cout << maximum_flow << ',';
  if(print_flow) {
    for(Edges const& edges: graph.vertices) {
      for(Edge const& edge: edges) {
        std::cout << edge.src << " -> " << edge.dst << ": " << edge.flow << "/"
                  << edge.capacity << '\n';
      }
    }
  }

  std::cout << time << ",";
  std::cout << augmenting_paths << '\n';

  return RETURN_SUCCESS;
}
