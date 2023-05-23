#include <types.hpp>

#include <optional>
#include <string_view>

#include <stdio.h>
#include <stdlib.h>

#include <client.hpp>

static void help(char const* const name) {
  printf("Usage: %s [OPTION]... IP PORT ID DEPTH\n", name);
  printf("\n");
  printf("  IP - IPv4 of the server.\n");
  printf("  PORT - Port of the server.\n");
  printf("  ID - The ID of the player (1 or 2).\n");
  printf("  DEPTH - The maximum search depth of the minmax (1 through 10).\n");
  printf("\n");
  printf("OPTIONS\n");
  printf("  -h, --help\n");
  printf("    Display the help page.\n");
  printf("  -l, --local\n");
  printf(
    "    Do not attempt to connect to the server, instead play locally via "
    "tui.\n");
}

struct Options {
  bool help = false;
  bool local = false;
};

struct Arguments {
  Options options;
  std::string_view ip;
  std::string_view port;
  Player player = Player::x;
  i32 depth = 0;
};

std::optional<Arguments> parse_arguments(i32 const argc,
                                         char const* const* const argv) {
  Arguments arguments;
  i32 i = 1;
  while(i < argc) {
    std::string_view option(argv[i]);
    if(option == "-h" || option == "--help") {
      arguments.options.help = true;
      return arguments;
    }

    if(option == "-l" || option == "--local") {
      arguments.options.local = true;
      i += 1;
    } else {
      // Not an option. End parsing.
      if(!option.starts_with("-")) {
        break;
      }

      printf("warning: unrecognised option: %s\n", argv[i]);
      i += 1;
    }
  }

  if(i >= argc) {
    printf("error: missing IP argument\n");
    return std::nullopt;
  }

  arguments.ip = argv[i];
  i += 1;

  if(i >= argc) {
    printf("error: missing PORT argument\n");
    return std::nullopt;
  }

  arguments.port = argv[i];
  i += 1;

  if(i >= argc) {
    printf("error: missing ID argument\n");
    return std::nullopt;
  }

  i32 const id = atoi(argv[i]);
  if(id != 1 && id != 2) {
    printf("error: invalid value %d for parameter ID. must be 1 or 2\n", id);
    return std::nullopt;
  }
  arguments.player = static_cast<Player>(id - 1);
  i += 1;

  if(i >= argc) {
    printf("error: missing DEPTH argument\n");
    return std::nullopt;
  }

  arguments.depth = atoi(argv[i]);
  if(arguments.depth < 1 || arguments.depth > 10) {
    printf(
      "error: invalid value %d for parameter DEPTH. must be between 1 and 10\n",
      arguments.depth);
    return std::nullopt;
  }
  i += 1;

  return arguments;
}

constexpr i32 RETURN_HELP = 2;
constexpr i32 RETURN_SUCCESS = 0;
constexpr i32 RETURN_FAILURE = 1;

int main(int const argc, char const* const* const argv) {
  std::optional<Arguments> result = parse_arguments(argc, argv);
  if(!result) {
    return RETURN_FAILURE;
  }

  Arguments& arguments = result.value();
  if(arguments.options.help) {
    help(argv[0]);
    return RETURN_HELP;
  }

  if(arguments.options.local) {
    play_local(arguments.player, arguments.depth);
  } else {
    play_online(arguments.ip, arguments.port, arguments.player,
                arguments.depth);
  }

  return RETURN_SUCCESS;
}
