#include <glang_sema/diagnostics.hpp>

#include <anton/format.hpp>

#include <glang_diagnostics/utilities.hpp>

namespace glang {
  Error err_symbol_redefinition(FE_Context& ctx, ast::Node const* original,
                                ast::Node const* redefined)
  {
    ANTON_UNUSED(original);
    Error error = error_from_source(ctx.allocator, redefined->source_info);
    error.diagnostic = "redefinition of symbol";
    return error;
  }

  Error err_symbol_undefined(FE_Context& ctx, ast::Node const* use)
  {
    Error error = error_from_source(ctx.allocator, use->source_info);
    error.diagnostic = "use of undefined symbol";
    return error;
  }

  Error err_called_symbol_is_not_function(FE_Context& ctx,
                                          ast::Stmt_Call const* call,
                                          ast::Node const* symbol)
  {
    ANTON_UNUSED(symbol);
    Error error = error_from_source(ctx.allocator, call->source_info);
    error.diagnostic =
      anton::format(ctx.allocator, "called symbol '{}' is not a function",
                    call->identifier.value);
    return error;
  }
} // namespace glang
