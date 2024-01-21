#include <glang_diagnostics/utilities.hpp>

namespace glang {
  using namespace anton::literals;

  Error error_from_source(Allocator* const allocator, Source_Info const& info)
  {
    return Error{.line = info.line,
                 .column = info.column,
                 .end_line = info.end_line,
                 .end_column = info.end_column,
                 .source = anton::String(info.source_path, allocator),
                 .diagnostic = anton::String(allocator),
                 .extended_diagnostic = anton::String(allocator)};
  }

  Error error_from_source(Allocator* const allocator,
                          anton::String_View const source_path, i64 const line,
                          i64 const column)
  {
    return Error{.line = line,
                 .column = column,
                 .end_line = line,
                 .end_column = column + 1,
                 .source = anton::String(source_path, allocator),
                 .diagnostic = anton::String(allocator),
                 .extended_diagnostic = anton::String(allocator)};
  }

  anton::String format_diagnostic_location(Allocator* const allocator,
                                           anton::String_View const source_path,
                                           i64 const line, i64 const column)
  {
    return anton::concat(allocator, source_path, u8":"_sv,
                         anton::to_string(allocator, line), u8":"_sv,
                         anton::to_string(allocator, column), u8": "_sv);
  }

  anton::String format_diagnostic_location(Allocator* const allocator,
                                           Source_Info const& info)
  {
    return format_diagnostic_location(allocator, info.source_path, info.line,
                                      info.column);
  }
} // namespace glang
