#pragma once

#include <anton/array.hpp>
#include <anton/expected.hpp>
#include <anton/string7_view.hpp>

#include <glang_core/types.hpp>
#include <glang_diagnostics/error.hpp>

namespace glang {
  struct FE_Context;

  enum struct Token_Kind {
    identifier,
    comment,
    whitespace,
    // keywords
    kw_procedure,
    kw_is,
    kw_in,
    kw_end,
    kw_program,
    kw_if,
    kw_then,
    kw_else,
    kw_endif,
    kw_while,
    kw_do,
    kw_endwhile,
    kw_repeat,
    kw_until,
    kw_read,
    kw_write,
    kw_t,
    // separators
    tk_lbracket,
    tk_rbracket,
    tk_lparen,
    tk_rparen,
    tk_langle,
    tk_rangle,
    tk_semicolon,
    tk_colon,
    tk_comma,
    tk_plus,
    tk_minus,
    tk_asterisk,
    tk_slash,
    tk_percent,
    tk_bang,
    tk_equals,
    // literals
    lt_dec_integer,
  };

  struct Token {
    Token_Kind kind;
    anton::String7_View value;
    i64 offset;
    i64 line;
    i64 column;
    i64 end_offset;
    i64 end_line;
    i64 end_column;
  };

  [[nodiscard]] anton::Expected<anton::Array<Token>, Error>
  lex_source(FE_Context const& ctx, anton::String_View source_path,
             anton::String7_View source);
} // namespace glang
