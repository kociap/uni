#pragma once

#include <types.hpp>

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
  std::vector<std::string_view> arguments;
};

[[nodiscard]] std::optional<Parse_Result>
parse_options(std::span<Option_Definition const> definitions, i32 argc,
              char const* const* argv);
