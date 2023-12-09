#include <glang_lexer/lexer.hpp>

#include <anton/optional.hpp>
#include <anton/string7.hpp>

#include <glang_frontend/fe_context.hpp>
#include <glang_lexer/diagnostics.hpp>

namespace glang {
  using namespace anton::literals;

  [[nodiscard]] static bool is_whitespace(char32 c)
  {
    return (c <= 32) | (c == 127);
  }

  [[nodiscard]] static bool is_digit(char32 c)
  {
    return (c >= 48) & (c <= 57);
  }

  [[nodiscard]] static bool is_alpha(char32 c)
  {
    return (c >= 97 && c < 123) | (c >= 65 && c < 91);
  }

  [[nodiscard]] static bool is_identifier_character(char32 c)
  {
    return (c == '_') | is_alpha(c);
  }

  [[nodiscard]] static anton::Optional<Token_Kind>
  is_keyword(Allocator* allocator, anton::String7_View string,
             bool const case_sensitive)
  {
    if(!case_sensitive) {
      // This is a terrible hack that works on the assumption that allocator is
      // a bump allocator that does not free the underlaying memory.
      string = anton::to_upper(allocator, string);
    }
    u64 const h = anton::hash(string);
    switch(h) {
    case anton::hash("PROCEDURE"_sv):
      return Token_Kind::kw_procedure;
    case anton::hash("IS"_sv):
      return Token_Kind::kw_is;
    case anton::hash("IN"_sv):
      return Token_Kind::kw_in;
    case anton::hash("END"_sv):
      return Token_Kind::kw_end;
    case anton::hash("PROGRAM"_sv):
      return Token_Kind::kw_program;
    case anton::hash("IF"_sv):
      return Token_Kind::kw_if;
    case anton::hash("THEN"_sv):
      return Token_Kind::kw_then;
    case anton::hash("ELSE"_sv):
      return Token_Kind::kw_else;
    case anton::hash("ENDIF"_sv):
      return Token_Kind::kw_endif;
    case anton::hash("WHILE"_sv):
      return Token_Kind::kw_while;
    case anton::hash("DO"_sv):
      return Token_Kind::kw_do;
    case anton::hash("ENDWHILE"_sv):
      return Token_Kind::kw_endwhile;
    case anton::hash("REPEAT"_sv):
      return Token_Kind::kw_repeat;
    case anton::hash("UNTIL"_sv):
      return Token_Kind::kw_until;
    case anton::hash("READ"_sv):
      return Token_Kind::kw_read;
    case anton::hash("WRITE"_sv):
      return Token_Kind::kw_write;
    case anton::hash("T"_sv):
      return Token_Kind::kw_t;
    default:
      return anton::null_optional;
    }
  }

  struct Source_State {
    i64 offset;
    i64 line;
    i64 column;
  };

  anton::Expected<anton::Array<Token>, Error>
  lex_source(FE_Context const& ctx, anton::String_View const source_path,
             anton::String7_View source)
  {
    anton::Array<Token> tokens(ctx.allocator, anton::reserve, 4096);
    char8 const* const source_begin = source.begin();
    char8 const* current = source.begin();
    char8 const* const end = source.end();
    i64 line = 1;
    i64 column = 1;
    while(current != end) {
      char8 const c = *current;
      if(is_whitespace(c)) {
        Source_State const state{current - source_begin, line, column};
        // Handle whitespace.
        char8 const* const begin = current;
        do {
          if(*current == '\n') {
            line += 1;
            column = 1;
          } else {
            ++column;
          }
          ++current;
        } while(current != end && is_whitespace(*current));
        tokens.push_back(Token{Token_Kind::whitespace,
                               anton::String7_View{begin, current},
                               state.offset, state.line, state.column,
                               current - source_begin, line, column});
      } else if(c == '#') {
        // Handle line comments.
        Source_State const state{current - source_begin, line, column};
        char8 const* const begin = current;
        for(; current != end && *current != '\n'; ++current) {}
        // The loop stops at the newline or the eof. Skip the newline.
        if(current != end) {
          current += 1;
        }
        line += 1;
        column = 1;

        tokens.push_back(Token{Token_Kind::comment,
                               anton::String7_View{begin, current},
                               state.offset, state.line, state.column,
                               current - source_begin, line, column});
      } else if(is_identifier_character(c)) {
        // Handle identifier. Might be a keyword or an actual identifier.
        Source_State const state{current - source_begin, line, column};
        char8 const* const begin = current;
        while(current != end && is_identifier_character(*current)) {
          ++current;
        }

        anton::String7_View const identifier{begin, current};
        column += current - begin;

        anton::Optional<Token_Kind> keyword =
          is_keyword(ctx.allocator, identifier,
                     ctx.options.enable_case_sensitive_keywords);
        if(keyword) {
          tokens.push_back(Token{keyword.value(), identifier, state.offset,
                                 state.line, state.column,
                                 current - source_begin, line, column});
        } else {
          tokens.push_back(Token{Token_Kind::identifier, identifier,
                                 state.offset, state.line, state.column,
                                 current - source_begin, line, column});
        }
      } else if(is_digit(c)) {
        // Handle integers. We match the integral part. The plus and minus signs
        // are not a part of the literals.
        Source_State const state{current - source_begin, line, column};
        char8 const* const integer_begin = current;
        while(current != end && is_digit(*current)) {
          ++current;
          ++column;
        }
        anton::String7_View const integer{integer_begin, current};
        tokens.push_back(Token{Token_Kind::lt_dec_integer, integer,
                               state.offset, state.line, state.column,
                               current - source_begin, line, column});
      } else {
        // Handle tokens.
        Source_State const state{current - source_begin, line, column};
        char8 const* const begin = current;
        Token_Kind token_kind;
        switch(c) {
        case '[':
          token_kind = Token_Kind::tk_lbracket;
          break;
        case ']':
          token_kind = Token_Kind::tk_rbracket;
          break;
        case '(':
          token_kind = Token_Kind::tk_lparen;
          break;
        case ')':
          token_kind = Token_Kind::tk_rparen;
          break;
        case '<':
          token_kind = Token_Kind::tk_langle;
          break;
        case '>':
          token_kind = Token_Kind::tk_rangle;
          break;
        case ';':
          token_kind = Token_Kind::tk_semicolon;
          break;
        case ':':
          token_kind = Token_Kind::tk_colon;
          break;
        case ',':
          token_kind = Token_Kind::tk_comma;
          break;
        case '+':
          token_kind = Token_Kind::tk_plus;
          break;
        case '-':
          token_kind = Token_Kind::tk_minus;
          break;
        case '*':
          token_kind = Token_Kind::tk_asterisk;
          break;
        case '/':
          token_kind = Token_Kind::tk_slash;
          break;
        case '%':
          token_kind = Token_Kind::tk_percent;
          break;
        case '!':
          token_kind = Token_Kind::tk_bang;
          break;
        case '=':
          token_kind = Token_Kind::tk_equals;
          break;
        default:
          return {anton::expected_error,
                  err_lexer_unrecognised_token(
                    ctx, source_path, current - source_begin, line, column)};
        }
        ++current;
        ++column;
        tokens.push_back(Token{token_kind, anton::String7_View{begin, current},
                               state.offset, state.line, state.column,
                               current - source_begin, line, column});
      }
    }
    return {anton::expected_value, ANTON_MOV(tokens)};
  }
} // namespace glang
