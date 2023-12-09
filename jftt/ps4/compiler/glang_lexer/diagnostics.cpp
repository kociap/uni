#include <glang_lexer/diagnostics.hpp>

#include <glang_frontend/fe_context.hpp>

namespace glang {
  Error err_lexer_unrecognised_token(FE_Context const& ctx,
                                     anton::String_View const source_path,
                                     i64 const offset, i64 const line,
                                     i64 const column)
  {
    // Error error = error_from_source(ctx.allocator, source_path, line, column);
    // anton::String_View const source =
    //   ctx.source_registry->find_source(source_path)->data;
    // anton::String_View const token = get_source_bit(source, offset, offset + 1);
    // error.diagnostic =
    //   anton::format(ctx.allocator, "error: unrecognised token '{}'"_sv, token);
    // print_source_snippet(ctx, error.extended_diagnostic, source, offset,
    //                      offset + 1, line);
    // return error;
    return Error{};
  }
} // namespace glang
