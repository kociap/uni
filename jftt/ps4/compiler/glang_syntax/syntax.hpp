#pragma once

#include <anton/array.hpp>
#include <anton/string.hpp>

#include <glang_core/either.hpp>
#include <glang_core/source_info.hpp>
#include <glang_core/types.hpp>

namespace glang {
  // Syntax_Node_Kind
  // Overlaps with lexer's Token_Kind.
  //
  enum struct Syntax_Node_Kind {
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

    // compound tokens
    tk_neq, // !=
    tk_lteq, // <=
    tk_gteq, // >=
    tk_assign, // :=

    variable,

    fn_parameter,
    fn_parameter_list,
    fn_definition_list,

    decl_procedure,
    decl_main,

    call_arg_list,

    expr_identifier,
    expr_binary,
    expr_index,
    expr_lt_integer,

    stmt_list,

    stmt_call,
    stmt_if,
    stmt_while,
    stmt_repeat,
    stmt_assign,
    stmt_read,
    stmt_write,
  };

  struct Syntax_Token;
  struct Syntax_Node;

  // Syntax Node Or Token
  using SNOT = Either<Syntax_Node, Syntax_Token>;

  struct Syntax_Token {
    anton::String value;
    Source_Info source_info;
    Syntax_Node_Kind kind;

    Syntax_Token(Syntax_Node_Kind kind, anton::String value,
                 Source_Info const& source_info);
  };

  // Syntax_Node
  // Untyped syntax node containing syntax information.
  //
  struct Syntax_Node {
    anton::Array<SNOT> children;
    Source_Info source_info;
    Syntax_Node_Kind kind;

    Syntax_Node(Syntax_Node_Kind kind, anton::Array<SNOT> array,
                Source_Info const& source_info);
  };
} // namespace glang
