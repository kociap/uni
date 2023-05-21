#include <cstddef>
#include <graph.hpp>
#include <types.hpp>
#include <utility.hpp>

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

struct Option_Definition {
  std::string_view name;
  i32 id = 0;
  bool mandatory = false;
};

struct Option {
  i32 id = 0;
  std::string_view value;
};

struct Parse_Result {
  std::vector<Option> options;
  std::vector<std::string> arguments;
};

[[nodiscard]] std::optional<Parse_Result>
parse_options(std::span<Option_Definition const> const definitions,
              i32 const argc, char const* const* const argv) {
  Parse_Result result;
  i32 i = 1;
  while(i < argc) {
    bool recognised = false;
    std::string_view option(argv[i]);
    // Not an option. End parsing.
    if(!option.starts_with("-")) {
      break;
    }

    for(Option_Definition const& d: definitions) {
      if(d.name != option) {
        continue;
      }

      recognised = true;
      i += 1;

      if(d.mandatory) {
        if(i >= argc) {
          printf("error: missing mandatory argument to %s\n", argv[i - 1]);
          return std::nullopt;
        }

        std::string_view argument(argv[i]);
        result.options.push_back(Option{d.id, argument});
        i += 1;
      }
    }

    if(!recognised) {
      printf("error: unrecognised option: %s\n", argv[i]);
      return std::nullopt;
    }
  }

  while(i < argc) {
    result.arguments.push_back(argv[i]);
    i += 1;
  }

  return result;
}

static void help(char const* const name) {
  printf("Usage: %s [OPTION]... CONFIGURATION\n", name);
  printf("\n");
  printf("OPTION\n");
  printf(" -h, --help       display the help page\n");
  printf(" -p, --problem\n");
  printf(" -d, --data\n");
  printf(" -o, --output\n");
}

[[nodiscard]] static std::string read_line(FILE* const file) {
  std::string result;
  while(true) {
    char const c = fgetc(file);
    if(c == '\n' || c == EOF) {
      break;
    }

    if(c == '\r') {
      // Ignore carriage return.
      continue;
    }

    result += c;
  }
  return result;
}

[[nodiscard]] static char const* read_i32(char const* begin,
                                          char const* const end, i32& v) {
  while(begin != end && (*begin > '9' || *begin < '0')) {
    ++begin;
  }

  v = 0;
  while(begin != end && (*begin >= '0' && *begin <= '9')) {
    v = 10 * v + *begin - '0';
    ++begin;
  }

  return begin;
}

[[nodiscard]] static std::optional<Graph> read_data(std::string const& path) {
  FILE* const file = fopen(path.c_str(), "r");
  File_Guard fguard(file);
  if(!file) {
    printf("error: could not open file \"%s\" for reading\n", path.c_str());
    return std::nullopt;
  }

  Graph graph;
  while(true) {
    std::string line = read_line(file);
    if(line.size() == 0) {
      break;
    }

    if(line[0] == 'c') {
      continue;
    }

    if(line[0] == 'p') {
      i32 n, m;
      char const* b = line.data();
      char const* const e = line.data() + line.size();
      b = read_i32(b, e, n);
      b = read_i32(b, e, m);
      graph.vertices.resize(n);
      for(i32 index = 0; Vertex & v: graph.vertices) {
        v.index = index;
        index += 1;
      }
      continue;
    }

    if(line[0] == 'a') {
      i32 src, dst, w;
      char const* b = line.data();
      char const* const e = line.data() + line.size();
      b = read_i32(b, e, src);
      b = read_i32(b, e, dst);
      b = read_i32(b, e, w);
      graph.vertices[src - 1].edges.push_back(Edge{dst - 1, w});
      continue;
    }

    printf("error: unrecognised attribute %c\n", line[0]);
    break;
  }

  return graph;
}

enum struct Problem_Kind {
  p2p,
  ss,
};

struct Problem_P2P {
  std::vector<std::pair<i32, i32>> queries;
};

struct Problem_SS {
  std::vector<i32> sources;
};

[[nodiscard]] static std::optional<Problem_Kind>
read_problem_kind(std::string const& path) {
  FILE* const file = fopen(path.c_str(), "r");
  File_Guard fguard(file);
  if(!file) {
    printf("error: could not open file \"%s\" for reading\n", path.c_str());
    return std::nullopt;
  }

  i32 line_n = 0;
  while(true) {
    line_n += 1;
    std::string line = read_line(file);
    if(line.size() == 0) {
      printf("error: missing attribute p\n");
      return std::nullopt;
    }

    if(line[0] == 'c') {
      continue;
    }

    if(line[0] == 'p') {
      // The line prefix is 'p aux sp ' which is 9 chars long.
      std::string_view const kind(line.data() + 9, line.data() + 11);
      if(kind == "ss") {
        return Problem_Kind::ss;
      } else if(kind == "p2") {
        return Problem_Kind::p2p;
      } else {
        printf("error: unrecognised problem kind in line %d\n", line_n);
        return std::nullopt;
      }
    }

    printf("error: attribute %c precedes attribute p in line %d\n", line[0],
           line_n);
    return std::nullopt;
  }
}

[[nodiscard]] static std::optional<Problem_P2P>
read_problem_p2p(std::string const& path) {
  FILE* const file = fopen(path.c_str(), "r");
  File_Guard fguard(file);
  if(!file) {
    printf("error: could not open file \"%s\" for reading\n", path.c_str());
    return std::nullopt;
  }

  Problem_P2P problem;
  i32 line_n = 0;
  while(true) {
    line_n += 1;
    std::string line = read_line(file);
    if(line.size() == 0) {
      break;
    }

    if(line[0] == 'c' || line[0] == 'p') {
      continue;
    }

    if(line[0] == 'q') {
      i32 src, dst;
      char const* b = line.data();
      char const* const e = line.data() + line.size();
      b = read_i32(b, e, src);
      b = read_i32(b, e, dst);
      problem.queries.push_back({src, dst});
      continue;
    }

    printf("error: unrecognised attribute %c in line %d\n", line[0], line_n);
    break;
  }

  return problem;
}

[[nodiscard]] static std::optional<Problem_SS>
read_problem_ss(std::string const& path) {
  FILE* const file = fopen(path.c_str(), "r");
  File_Guard fguard(file);
  if(!file) {
    printf("error: could not open file \"%s\" for reading\n", path.c_str());
    return std::nullopt;
  }

  Problem_SS problem;
  i32 line_n = 0;
  while(true) {
    line_n += 1;
    std::string line = read_line(file);
    if(line.size() == 0) {
      break;
    }

    if(line[0] == 'c' || line[0] == 'p') {
      continue;
    }

    if(line[0] == 's') {
      i32 src;
      char const* b = line.data();
      char const* const e = line.data() + line.size();
      b = read_i32(b, e, src);
      problem.sources.push_back(src);
      continue;
    }

    printf("error: unrecognised attribute %c in line %d\n", line[0], line_n);
    break;
  }

  return problem;
}

int main(int const argc, char const* const* const argv) {
  constexpr i32 RETURN_FAILURE = 1;
  constexpr i32 RETURN_SUCCESS = 0;
  constexpr i32 RETURN_HELP = 2;

  constexpr i32 option_help = 0;
  constexpr i32 option_data = 1;
  constexpr i32 option_output = 2;
  constexpr i32 option_problem = 3;

  Option_Definition const definitions[] = {{"-h", option_help, false},
                                           {"--help", option_help, false},
                                           // Data file.
                                           {"-d", option_data, true},
                                           {"--data", option_data, true},
                                           // Output file.
                                           {"-o", option_output, true},
                                           {"--output", option_output, true},
                                           // Problem specification_file
                                           {"-p", option_problem, true},
                                           {"--problem", option_problem, true}};
  std::optional<Parse_Result> result = parse_options(definitions, argc, argv);
  if(!result) {
    return RETURN_FAILURE;
  }

  std::string data_file;
  std::string output_file;
  std::string problem_file;

  for(Option const& option: result->options) {
    switch(option.id) {
      case option_help:
        help(argv[0]);
        return RETURN_HELP;

      case option_data:
        data_file = option.value;
        break;

      case option_output:
        output_file = option.value;
        break;

      case option_problem:
        problem_file = option.value;
        break;

      default:
        break;
    }
  }

  if(problem_file.size() == 0) {
    printf("error: problem file not specified\n");
    return RETURN_FAILURE;
  }

  if(data_file.size() == 0) {
    printf("error: data file not specified\n");
    return RETURN_FAILURE;
  }

  std::optional<Graph> result_graph = read_data(data_file);
  if(!result_graph) {
    return RETURN_FAILURE;
  }

  std::optional<Problem_Kind> result_kind = read_problem_kind(problem_file);
  if(!result_kind) {
    return RETURN_FAILURE;
  }

  FILE* output_stream = nullptr;
  if(!output_file.empty()) {
    output_stream = fopen(output_file.c_str(), "w");
    if(output_stream == nullptr) {
      printf(
        "error: failed to open output file \"%s\". writing to stdout instead\n",
        output_file.c_str());
    }
  }

  if(output_stream == nullptr) {
    output_stream = stdout;
  }

  Problem_Kind const& kind = result_kind.value();
  Graph& graph = result_graph.value();
  switch(kind) {
    case Problem_Kind::p2p: {
      std::optional<Problem_P2P> result_problem =
        read_problem_p2p(problem_file);
      if(!result_problem) {
        return RETURN_FAILURE;
      }
      Problem_P2P& problem = result_problem.value();
      for(auto const& [src, dst]: problem.queries) {
#if defined(ALGORITHM_DIJKSTRA)
        std::vector<i64> result =
          shortest_path_dijkstra(graph, graph.vertices[src - 1]);
        fprintf(output_stream, "d %d %d %lld\n", src, dst, result[dst - 1]);
#elif defined(ALGORITHM_DIAL)
        std::vector<i64> result =
          shortest_path_dial(graph, graph.vertices[src - 1]);
        fprintf(output_stream, "d %d %d %lld\n", src, dst, result[dst - 1]);
#elif defined(ALGORITHM_RADIX)
        std::vector<i64> result =
          shortest_path_radix(graph, graph.vertices[src - 1]);
        fprintf(output_stream, "d %d %d %lld\n", src, dst, result[dst - 1]);
#else
  #error "algorithm not selected"
#endif
      }

    } break;

    case Problem_Kind::ss: {
      std::optional<Problem_SS> result_problem = read_problem_ss(problem_file);
      if(!result_problem) {
        return RETURN_FAILURE;
      }
      Problem_SS& problem = result_problem.value();
      Timer timer;
      timer.start();
      for(i32 const src: problem.sources) {
#if defined(ALGORITHM_DIJKSTRA)
        std::vector<i64> result =
          shortest_path_dijkstra(graph, graph.vertices[src - 1]);
#elif defined(ALGORITHM_DIAL)
        std::vector<i64> result =
          shortest_path_dial(graph, graph.vertices[src - 1]);
#elif defined(ALGORITHM_RADIX)
        std::vector<i64> result =
          shortest_path_radix(graph, graph.vertices[src - 1]);
#else
  #error "algorithm not selected"
#endif
      }
      i64 const time = timer.end_us();
      fprintf(output_stream, "t %lld.%lld\n", time / 1000, time % 1000);
    } break;
  }
  return RETURN_SUCCESS;
}
