#include <chrono>
#include <optional>
#include <stdio.h>
#include <string_view>

#include <debug.hpp>
#include <heuristic.hpp>
#include <solver.hpp>

struct Timer {
  private:
  std::chrono::high_resolution_clock::time_point start_time;

  public:
  void start() {
    start_time = std::chrono::high_resolution_clock::now();
  }

  [[nodiscard]] i64 end_ns() {
    auto const end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time -
                                                                start_time)
      .count();
  }

  [[nodiscard]] i64 end_ms() {
    auto const end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                                 start_time)
      .count();
  }
};

static void help(char const* const name) {
  printf("Usage: %s [OPTION]... CONFIGURATION\n", name);
  printf("\n");
  printf(
    "CURRENTLY CONFIGURATION IS A HARDCODED 48 MOVES CONFIGURATION AND IS NOT "
    "READ FROM THE STDIN\n");
  printf("\n");
  printf(" -h, --help       display the help page\n");
  printf(
    " -a, --algorithm  select the algorithm to use. Available options "
    "are: A*, IDA*, BA*\n");
  printf(
    " -e, --heuristic  select the heuristic to use. Available options "
    "are: MD, LC, PDB\n");
}

enum struct Heuristic_Kind {
  manhattan_distance,
  linear_conflict,
  pattern_database,
};

enum struct Algorithm_Kind {
  Astar,
  IDAstar,
  BAstar,
};

static void error_algorithm_heuristic_combination(
  [[maybe_unused]] Algorithm_Kind const algorithm,
  [[maybe_unused]] Heuristic_Kind const heuristic) {
  printf("error: combination of algorithm and heuristic not supported\n");
}

struct Options {
  Algorithm_Kind algorithm = Algorithm_Kind::Astar;
  Heuristic_Kind heuristic = Heuristic_Kind::linear_conflict;
  bool help = false;
};

std::optional<Options> parse_options(i32 const argc,
                                     char const* const* const argv) {
  Options options;
  for(i32 i = 1; i < argc;) {
    std::string_view option(argv[i]);
    if(option == "-h" || option == "--help") {
      options.help = true;
      return options;
    }

    if(option == "-a" || option == "--algorithm") {
      if(i + 1 >= argc) {
        printf("error: missing mandatory argument to %s\n", argv[i]);
        return std::nullopt;
      }

      std::string_view arg(argv[i + 1]);
      if(arg == "A*") {
        options.algorithm = Algorithm_Kind::Astar;
      } else if(arg == "IDA*") {
        options.algorithm = Algorithm_Kind::IDAstar;
      } else if(arg == "BA*") {
        options.algorithm = Algorithm_Kind::BAstar;
      } else {
        printf("error: unrecognised argument to %s: %s\n", argv[i],
               argv[i + 1]);
      }

      i += 2;
    } else if(option == "-e" || option == "--heuristic") {
      if(i + 1 >= argc) {
        printf("error: missing mandatory argument to %s\n", argv[i]);
        return std::nullopt;
      }

      std::string_view arg(argv[i + 1]);
      if(arg == "MD") {
        options.heuristic = Heuristic_Kind::manhattan_distance;
      } else if(arg == "LC") {
        options.heuristic = Heuristic_Kind::linear_conflict;
      } else if(arg == "PDB") {
        options.heuristic = Heuristic_Kind::pattern_database;
      } else {
        printf("error: unrecognised argument to %s: %s\n", argv[i],
               argv[i + 1]);
      }

      i += 2;
    } else {
      // Not an option. End parsing.
      if(!option.starts_with("-")) {
        break;
      }

      printf("warning: unrecognised option: %s\n", argv[i]);
      i += 1;
    }
  }

  return options;
}

namespace puzzle15 {
  static heuristic_t select_forward_heuristic(Heuristic_Kind const kind) {
    switch(kind) {
      case Heuristic_Kind::manhattan_distance:
        return heuristic_manhattan_distance;
      case Heuristic_Kind::linear_conflict:
        return heuristic_linear_conflict;
      case Heuristic_Kind::pattern_database:
        return heuristic_pattern_database;
    }
    __builtin_unreachable();
  }

  static generic_heuristic_t
  select_backward_heuristic(Heuristic_Kind const kind) {
    switch(kind) {
      case Heuristic_Kind::manhattan_distance:
        return heuristic_manhattan_distance_generic;
      case Heuristic_Kind::linear_conflict:
        return heuristic_linear_conflict_generic;
      case Heuristic_Kind::pattern_database:
        return heuristic_pattern_database_generic;
    }
    __builtin_unreachable();
  }
} // namespace puzzle15

int main(int const argc, char** const argv) {
  constexpr i32 RETURN_SUCCESS = 0;
  constexpr i32 RETURN_ERROR = 1;
  constexpr i32 RETURN_HELP = 2;

  // Configuration_Entry configuration[] = {
  //     11, 4,  7,  3, //
  //     13, 5,  6,  2, //
  //     12, 9,  11, 10, //
  //     8,  14, 15, 16 //
  // };
  // 80 moves
  //   Configuration_Entry configuration[] = {
  //     16, 4, 7,  3, //
  //     11, 5, 6,  2, //
  //     13, 9, 11, 10, //
  //     12, 8, 14, 15 //
  //   };
  // 48 moves
  Configuration_Entry configuration[] = {
    2,  5,  13, 12, //
    1,  16, 3,  15, //
    9,  7,  14, 6, //
    10, 11, 8,  4,
  };
  // 15 moves
  // Configuration_Entry configuration[] = {
  //   5,  1,  7,  3, //
  //   9,  2,  11, 4, //
  //   13, 6,  15, 8, //
  //   16, 10, 14, 12, //
  // };
  // Wuja (supposedly 50)
  // Configuration_Entry configuration[] = {
  //   6,  10, 2,  13, //
  //   12, 8,  4,  11, //
  //   3,  5,  16, 7, //
  //   9,  1,  15, 14 //
  // };
  // Configuration_Entry configuration[] = {
  //     2, 7, 5, //
  //     9, 4, 8, //
  //     1, 3, 6 //
  // };
  // Configuration_Entry configuration[] = {
  //     1, 2, //
  //     4, 3, //
  // };

  std::optional<Options> parse_result = parse_options(argc, argv);
  if(!parse_result) {
    return RETURN_ERROR;
  }

  Options& options = parse_result.value();
  if(options.help) {
    help(argv[0]);
    return RETURN_HELP;
  }

  if(!puzzle15::is_solvable(configuration)) {
    printf("configuration not solvable");
    return RETURN_SUCCESS;
  }

  if(options.heuristic == Heuristic_Kind::pattern_database) {
    Timer database_timer;
    database_timer.start();
    puzzle15::build_pattern_database();
    [[maybe_unused]] i64 const database_time = database_timer.end_ms();
    DEBUG_PRINT("database built in %lldms\n", database_time);
  }

  heuristic_t const forward_heuristic =
    puzzle15::select_forward_heuristic(options.heuristic);
  generic_heuristic_t const backward_heuristic =
    puzzle15::select_backward_heuristic(options.heuristic);

  Solution<puzzle15::Configuration> solution;
  Timer timer;
  timer.start();
  switch(options.algorithm) {
    case Algorithm_Kind::Astar: {
      if(forward_heuristic == nullptr) {
        error_algorithm_heuristic_combination(options.algorithm,
                                              options.heuristic);
        return RETURN_ERROR;
      }

      Astar_Parameters astar_parameters = {.starting_configuration =
                                             configuration,
                                           .heuristic = forward_heuristic};
      solution = puzzle15::Astar_solver(astar_parameters);
    } break;

    case Algorithm_Kind::IDAstar: {
      if(forward_heuristic == nullptr) {
        error_algorithm_heuristic_combination(options.algorithm,
                                              options.heuristic);
        return RETURN_ERROR;
      }
      IDAstar_Parameters idastar_parameters = {.starting_configuration =
                                                 configuration,
                                               .heuristic = forward_heuristic};
      solution = puzzle15::IDAstar_solver(idastar_parameters);
    } break;

    case Algorithm_Kind::BAstar: {
      if(forward_heuristic == nullptr || backward_heuristic == nullptr) {
        error_algorithm_heuristic_combination(options.algorithm,
                                              options.heuristic);
        return RETURN_ERROR;
      }

      Bidirectional_Astar_Parameters bastar_parameters = {
        .starting_configuration = configuration,
        .forward_heuristic = forward_heuristic,
        .backward_heuristic = backward_heuristic};
      solution = puzzle15::Bidirectional_Astar_solver(bastar_parameters);
    } break;
  }

  i64 const search_time = timer.end_ms();
  if(!solution.found) {
    printf(
      "solution not found in %lldms (%lld "
      "iterations)\n",
      search_time, solution.iterations);
    return RETURN_SUCCESS;
  }

  printf(
    "solution found in %lldms (%lld "
    "iterations, %lld depth, %lld "
    "explored)\n",
    search_time, solution.iterations, solution.depth, solution.explored);
  for(i32 index = 0; Configuration_View const configuration: solution.path) {
    printf("%d)\n", index);
    puzzle15::print(configuration);
    printf("\n");
    index += 1;
  }

  return RETURN_SUCCESS;
}
