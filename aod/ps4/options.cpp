#include <options.hpp>

std::optional<Parse_Result>
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
      } else {
        result.options.push_back(Option{d.id, ""});
      }

      break;
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
