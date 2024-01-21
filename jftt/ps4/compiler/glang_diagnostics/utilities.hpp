#pragma once

#include <anton/optional.hpp>

#include <glang_core/source_info.hpp>
#include <glang_diagnostics/error.hpp>

namespace glang {
  // error_from_source
  // Create empty Error with source information filled out.
  //
  [[nodiscard]] Error error_from_source(Allocator* allocator,
                                        Source_Info const& info);
  [[nodiscard]] Error error_from_source(Allocator* allocator,
                                        anton::String_View source_path,
                                        i64 line, i64 column);

  // format_diagnostic_location
  // Format the location of the diagnostic into a standardised format.
  //
  [[nodiscard]] anton::String format_diagnostic_location(
    Allocator* allocator, anton::String_View source_path, i64 line, i64 column);
  [[nodiscard]] anton::String
  format_diagnostic_location(Allocator* allocator, Source_Info const& info);
} // namespace glang
