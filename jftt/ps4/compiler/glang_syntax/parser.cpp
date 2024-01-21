#include <glang_syntax/parser.hpp>

#include <anton/optional.hpp>

#include <glang_core/source_info.hpp>
#include <glang_frontend/fe_context.hpp>
#include <glang_lexer/lexer.hpp>
#include <glang_syntax/syntax.hpp>

namespace glang {
  using namespace anton::literals;
  using anton::Optional;

  class Token_Stream_State {
  public:
    Token const* current;
    i64 offset;
  };

  struct Token_Stream {
  public:
    Token_Stream(Token const* begin, Token const* end)
      : current(begin), end(end), begin(begin)
    {
    }

    void advance_token()
    {
      if(current != end) {
        ++current;
      }
    }

    [[nodiscard]] Optional<Token> peek_token()
    {
      if(current != end) {
        return *current;
      } else {
        return anton::null_optional;
      }
    }

    [[nodiscard]] Optional<Token> next_token()
    {
      if(current != end) {
        Token token = *current;
        ++current;
        return token;
      } else {
        return anton::null_optional;
      }
    }

    [[nodiscard]] bool match_eof()
    {
      ignore_whitespace_and_comments();
      return current == end;
    }

    void ignore_whitespace_and_comments()
    {
      while(current != end) {
        Token_Kind const kind = current->kind;
        if(kind == Token_Kind::comment | kind == Token_Kind::whitespace) {
          ++current;
        } else {
          break;
        }
      }
    }

    [[nodiscard]] Token_Stream_State get_current_state()
    {
      ignore_whitespace_and_comments();
      return {current, current - begin};
    }

    [[nodiscard]] Token_Stream_State get_current_state_noskip()
    {
      return {current, current - begin};
    }

    [[nodiscard]] bool is_state_end(Token_Stream_State const& state) const
    {
      return state.current == end;
    }

    void restore_state(Token_Stream_State const& state)
    {
      current = state.current;
    }

  private:
    Token const* current;
    Token const* end;
    Token const* begin;
  };

#define EXPECT_TOKEN(token, message, snots)       \
  if(Optional _expect_token = skipmatch(token)) { \
    snots.push_back(ANTON_MOV(*_expect_token));   \
  } else {                                        \
    set_error(message);                           \
    lexer.restore_state(begin_state);             \
    return anton::null_optional;                  \
  }

#define EXPECT_TOKEN2(result, token1, token2, message, snots)      \
  if(Optional _expect_token = skipmatch(result, token1, token2)) { \
    snots.push_back(ANTON_MOV(*_expect_token));                    \
  } else {                                                         \
    set_error(message);                                            \
    lexer.restore_state(begin_state);                              \
    return anton::null_optional;                                   \
  }

#define EXPECT_NODE(fn, snots)                 \
  if(Optional _expect_node = fn()) {           \
    snots.push_back(ANTON_MOV(*_expect_node)); \
  } else {                                     \
    lexer.restore_state(begin_state);          \
    return anton::null_optional;               \
  }

  class Parser {
  public:
    Parser(Allocator* allocator, anton::String_View source_name,
           Token_Stream tokens)
      : _allocator(allocator), _source_name(source_name), lexer(tokens)
    {
    }

    anton::Expected<anton::Array<SNOT>, Error> build_syntax_tree()
    {
      anton::Array<SNOT> syntax_tree{_allocator};
      while(true) {
        if(lexer.match_eof()) {
          return {anton::expected_value, ANTON_MOV(syntax_tree)};
        }

        if(Optional<Syntax_Node> declaration = try_declaration()) {
          syntax_tree.emplace_back(ANTON_MOV(*declaration));
        } else {
          return {anton::expected_error, _last_error.to_error(_source_name)};
        }
      }
    }

  private:
    struct Parse_Error {
      anton::String message;
      i64 line = 0;
      i64 column = 0;
      i64 stream_offset = 0;

      Error to_error(anton::String_View source) const
      {
        return Error{.source = anton::String(source),
                     .diagnostic = message,
                     .extended_diagnostic = ""_s,
                     .line = line,
                     .column = column};
      }
    };

    Allocator* _allocator;
    anton::String_View _source_name;
    Token_Stream lexer;
    Parse_Error _last_error;

    void set_error(anton::String_View const message,
                   Token_Stream_State const& state)
    {
      if(lexer.is_state_end(state)) {
        Token const& token = *(state.current - 1);
        _last_error.message = message;
        _last_error.line = token.end_line;
        _last_error.column = token.end_column;
        _last_error.stream_offset = token.end_offset;
      }

      Token const& token = *state.current;
      if(token.offset >= _last_error.stream_offset) {
        _last_error.message = message;
        _last_error.line = token.line;
        _last_error.column = token.column;
        _last_error.stream_offset = token.offset;
      }
    }

    void set_error(anton::String_View const message)
    {
      Token_Stream_State const state = lexer.get_current_state_noskip();
      if(lexer.is_state_end(state)) {
        Token const& token = *(state.current - 1);
        _last_error.message = message;
        _last_error.line = token.end_line;
        _last_error.column = token.end_column;
        _last_error.stream_offset = token.end_offset;
      }

      Token const& token = *state.current;
      if(token.offset >= _last_error.stream_offset) {
        _last_error.message = message;
        _last_error.line = token.line;
        _last_error.column = token.column;
        _last_error.stream_offset = token.offset;
      }
    }

    [[nodiscard]] Source_Info src_info(Token_Stream_State const& start,
                                       Token_Stream_State const& end)
    {
      Token const& start_token = *start.current;
      Token const& end_token = *(end.current - 1);
      return Source_Info{.source_path = _source_name,
                         .line = start_token.line,
                         .column = start_token.column,
                         .offset = start_token.offset,
                         .end_line = end_token.end_line,
                         .end_column = end_token.end_column,
                         .end_offset = end_token.end_offset};
    }

    // match
    // Matches the next token with a specified type in the token stream.
    //
    // Parameters:
    // type - the type of the token to match.
    //
    // Returns:
    // Syntax_Token of the specified type or null_optional if the next token's type is not type.
    //
    [[nodiscard]] Optional<Syntax_Token> match(Token_Kind const type)
    {
      Optional<Token> const token_data = lexer.peek_token();
      if(!token_data) {
        return anton::null_optional;
      }

      Token_Kind const token_kind = token_data->kind;
      if(token_kind == type) {
        Token_Stream_State const begin_state = lexer.get_current_state_noskip();
        lexer.advance_token();
        Token_Stream_State const end_state = lexer.get_current_state_noskip();
        anton::String7_View const value = token_data->value;
        // Token_Kind and Syntax_Node_Kind have overlapping values, therefore we convert one to the other via a cast.
        Syntax_Node_Kind const syntax_kind =
          static_cast<Syntax_Node_Kind>(token_kind);
        Source_Info const source = src_info(begin_state, end_state);
        return Syntax_Token(
          syntax_kind, anton::String(value.begin(), value.end(), _allocator),
          source);
      } else {
        return anton::null_optional;
      }
    }

    // skipmatch
    // Matches the next token with a specified type in the token stream skipping whitespace and
    // comments. Does not match comments or whitespaces.
    //
    // Parameters:
    // type - the type of the token to match. Must not be Token_Kind::comment or
    //        Token_Kind::whitespace.
    //
    // Returns:
    // Syntax_Token of the specified type or null_optional if the next token's type is not type.
    //
    [[nodiscard]] Optional<Syntax_Token> skipmatch(Token_Kind const type)
    {
      lexer.ignore_whitespace_and_comments();
      return match(type);
    }

    // match
    // Matches identifier token with a specified content.
    //
    // Parameters:
    // value - the required identifier value.
    //
    // Returns:
    // An identifier Syntax_Token if the value of the token is the same as the parameter.
    // null_optional otherwise.
    //
    [[nodiscard]] Optional<Syntax_Token> match(anton::String7_View const value)
    {
      Token_Stream_State const begin_state = lexer.get_current_state_noskip();
      Optional<Token> const token_data = lexer.peek_token();
      if(!token_data) {
        return anton::null_optional;
      }

      Token_Kind const token_kind = token_data->kind;
      anton::String7_View const token_value = token_data->value;
      if(token_kind == Token_Kind::identifier && token_value == value) {
        lexer.advance_token();
        Token_Stream_State const end_state = lexer.get_current_state_noskip();
        Source_Info const source = src_info(begin_state, end_state);
        return Syntax_Token(
          Syntax_Node_Kind::identifier,
          anton::String(token_value.begin(), token_value.end(), _allocator),
          source);
      } else {
        return anton::null_optional;
      }
    }

    // skipmatch
    // Matches identifier token with a specified content.
    //
    // Parameters:
    // value - the required identifier value.
    //
    // Returns:
    // An identifier Syntax_Token if the value of the token is the same as the parameter.
    // null_optional otherwise.
    //
    [[nodiscard]] Optional<Syntax_Token>
    skipmatch(anton::String7_View const value)
    {
      lexer.ignore_whitespace_and_comments();
      return match(value);
    }

    // combine
    // Combines two tokens that appear next to each other in the source.
    // Tokens are combined in the order they are passed to the function.
    // Only value and source_info members of the tokens are combined.
    //
    // Parameters:
    //    result_type - Syntax_Node_Kind to assign to the result token.
    // token1, token2 - tokens to combine.
    //
    // Returns:
    // Combined Syntax_Token with type member set to result_type.
    //
    [[nodiscard]] Syntax_Token combine(Syntax_Node_Kind const result_type,
                                       Syntax_Token const& token1,
                                       Syntax_Token const& token2)
    {
      Source_Info source_info;
      // Both tokens have the same souce_path;
      source_info.source_path = token1.source_info.source_path;
      source_info.line =
        anton::math::min(token1.source_info.line, token2.source_info.line);
      source_info.column =
        anton::math::min(token1.source_info.column, token2.source_info.column);
      source_info.offset =
        anton::math::min(token1.source_info.offset, token2.source_info.offset);
      source_info.end_line = anton::math::max(token1.source_info.end_line,
                                              token2.source_info.end_line);
      source_info.end_column = anton::math::max(token1.source_info.end_column,
                                                token2.source_info.end_column);
      source_info.end_offset = anton::math::max(token1.source_info.end_offset,
                                                token2.source_info.end_offset);
      return Syntax_Token(result_type,
                          anton::concat(_allocator, token1.value, token2.value),
                          source_info);
    }

    // match
    //
    [[nodiscard]] Optional<Syntax_Token>
    match(Syntax_Node_Kind const result_type, Token_Kind const tk_type1,
          Token_Kind const tk_type2)
    {
      Token_Stream_State const begin_state = lexer.get_current_state_noskip();
      Optional tk1 = match(tk_type1);
      if(!tk1) {
        lexer.restore_state(begin_state);
        return anton::null_optional;
      }

      Optional tk2 = match(tk_type2);
      if(!tk2) {
        lexer.restore_state(begin_state);
        return anton::null_optional;
      }

      return combine(result_type, *tk1, *tk2);
    }

    // skipmatch
    //
    [[nodiscard]] Optional<Syntax_Token>
    skipmatch(Syntax_Node_Kind const result_type, Token_Kind const tk_type1,
              Token_Kind const tk_type2)
    {
      lexer.ignore_whitespace_and_comments();
      return match(result_type, tk_type1, tk_type2);
    }

    Optional<Syntax_Node> expect_procedure_parameter()
    {
      Token_Stream_State const begin_state = lexer.get_current_state();
      anton::Array<SNOT> snots{_allocator};
      if(Optional kw_t = match(Token_Kind::kw_t)) {
        snots.push_back(ANTON_MOV(*kw_t));
      }

      EXPECT_TOKEN(Token_Kind::identifier, "expected identifier"_sv, snots);
      Token_Stream_State const end_state = lexer.get_current_state_noskip();
      Source_Info const source = src_info(begin_state, end_state);
      return Syntax_Node(Syntax_Node_Kind::procedure_parameter,
                         ANTON_MOV(snots), source);
    }

    Optional<Syntax_Node> try_declaration()
    {
      Token_Stream_State const begin_state = lexer.get_current_state();
      anton::Array<SNOT> snots{_allocator};
      bool is_procedure = false;
      if(Optional kw_procedure = match(Token_Kind::kw_procedure)) {
        is_procedure = true;
        snots.push_back(ANTON_MOV(*kw_procedure));
        // Procedure identifier and parameters.
        EXPECT_TOKEN(Token_Kind::identifier, "expected identifier"_sv, snots);
        Token_Stream_State const params_begin_state = lexer.get_current_state();
        anton::Array<SNOT> params_snots{_allocator};
        EXPECT_TOKEN(Token_Kind::tk_lparen, "expected '('"_sv, params_snots);
        while(true) {
          EXPECT_NODE(expect_procedure_parameter, params_snots);
          if(Optional tk_comma = skipmatch(Token_Kind::tk_comma)) {
            params_snots.push_back(ANTON_MOV(*tk_comma));
          } else {
            break;
          }
        }
        EXPECT_TOKEN(Token_Kind::tk_rparen, "expected ')'"_sv, params_snots);
        Token_Stream_State const params_end_state =
          lexer.get_current_state_noskip();
        Source_Info const params_source =
          src_info(params_begin_state, params_end_state);
        snots.push_back(Syntax_Node(Syntax_Node_Kind::procedure_parameter_list,
                                    ANTON_MOV(params_snots), params_source));
      } else if(Optional kw_program = match(Token_Kind::kw_program)) {
        snots.push_back(ANTON_MOV(*kw_program));
      } else {
        set_error("expected declaration"_sv);
        return anton::null_optional;
      }

      EXPECT_TOKEN(Token_Kind::kw_is, "expected 'IS'"_sv, snots);

      // Procedure declaration list.
      Token_Stream_State const def_begin_state = lexer.get_current_state();
      anton::Array<SNOT> def_snots{_allocator};
      if(Optional kw_in = skipmatch(Token_Kind::kw_in)) {
        Source_Info const def_source =
          src_info(def_begin_state, def_begin_state);
        snots.push_back(
          Syntax_Node(Syntax_Node_Kind::procedure_declaration_list,
                      ANTON_MOV(def_snots), def_source));
        snots.push_back(ANTON_MOV(*kw_in));
      } else {
        while(true) {
          Token_Stream_State const var_begin_state = lexer.get_current_state();
          anton::Array<SNOT> var_snots{_allocator};
          EXPECT_TOKEN(Token_Kind::identifier, "expected_identifier"_sv,
                       var_snots);
          if(Optional tk_lbracket = skipmatch(Token_Kind::tk_lbracket)) {
            var_snots.push_back(ANTON_MOV(*tk_lbracket));
            EXPECT_TOKEN(Token_Kind::lt_dec_integer, "expected integer"_sv,
                         var_snots);
            EXPECT_TOKEN(Token_Kind::tk_rbracket, "expected ']'"_sv, var_snots);
          }
          Token_Stream_State const var_end_state =
            lexer.get_current_state_noskip();
          Source_Info const var_source =
            src_info(var_begin_state, var_end_state);
          def_snots.push_back(Syntax_Node(Syntax_Node_Kind::variable,
                                          ANTON_MOV(var_snots), var_source));

          if(Optional tk_comma = skipmatch(Token_Kind::tk_comma)) {
            def_snots.push_back(ANTON_MOV(*tk_comma));
          } else {
            break;
          }
        }
        Token_Stream_State const def_end_state =
          lexer.get_current_state_noskip();
        Source_Info const def_source = src_info(def_begin_state, def_end_state);
        snots.push_back(
          Syntax_Node(Syntax_Node_Kind::procedure_declaration_list,
                      ANTON_MOV(def_snots), def_source));

        EXPECT_TOKEN(Token_Kind::kw_in, "expected 'IN'"_sv, snots);
      }

      // Statement list.
      Token_Stream_State const stmt_begin_state = lexer.get_current_state();
      Token_Stream_State stmt_end_state;
      anton::Array<SNOT> stmt_snots{_allocator};
      while(true) {
        stmt_end_state = lexer.get_current_state_noskip();
        if(Optional kw_end = skipmatch(Token_Kind::kw_end)) {
          Source_Info const stmt_source =
            src_info(stmt_begin_state, stmt_end_state);
          snots.push_back(Syntax_Node(Syntax_Node_Kind::stmt_list,
                                      ANTON_MOV(stmt_snots), stmt_source));
          snots.push_back(ANTON_MOV(*kw_end));
          break;
        }

        EXPECT_NODE(expect_stmt, stmt_snots);
      }

      Token_Stream_State const end_state = lexer.get_current_state_noskip();
      Source_Info const source = src_info(begin_state, end_state);
      if(is_procedure) {
        return Syntax_Node(Syntax_Node_Kind::decl_procedure, ANTON_MOV(snots),
                           source);
      } else {
        return Syntax_Node(Syntax_Node_Kind::decl_main, ANTON_MOV(snots),
                           source);
      }
    }

    Syntax_Node expect_stmt_list()
    {
      Token_Stream_State const begin_state = lexer.get_current_state();
      anton::Array<SNOT> snots{_allocator};
      while(true) {
        if(Optional stmt = expect_stmt()) {
          snots.push_back(ANTON_MOV(*stmt));
        } else {
          break;
        }
      }
      Token_Stream_State const end_state = lexer.get_current_state_noskip();
      Source_Info const source = src_info(begin_state, end_state);
      return Syntax_Node(Syntax_Node_Kind::stmt_list, ANTON_MOV(snots), source);
    }

    Optional<Syntax_Node> expect_stmt()
    {
      Token_Stream_State const begin_state = lexer.get_current_state();
      anton::Array<SNOT> snots{_allocator};
      if(Optional kw_if = match(Token_Kind::kw_if)) {
        snots.push_back(ANTON_MOV(*kw_if));
        // Condition.
        EXPECT_NODE(expect_expression, snots);
        EXPECT_TOKEN(Token_Kind::kw_then, "expected 'THEN'"_sv, snots);
        Syntax_Node stmt_list = expect_stmt_list();
        snots.push_back(ANTON_MOV(stmt_list));
        if(Optional kw_else = skipmatch(Token_Kind::kw_else)) {
          snots.push_back(ANTON_MOV(*kw_else));
          Syntax_Node else_stmt_list = expect_stmt_list();
          snots.push_back(ANTON_MOV(else_stmt_list));
        }
        EXPECT_TOKEN(Token_Kind::kw_endif, "expected 'ENDIF'"_sv, snots);
        Token_Stream_State const end_state = lexer.get_current_state_noskip();
        Source_Info const source = src_info(begin_state, end_state);
        return Syntax_Node(Syntax_Node_Kind::stmt_if, ANTON_MOV(snots), source);
      }

      if(Optional kw_while = match(Token_Kind::kw_while)) {
        snots.push_back(ANTON_MOV(*kw_while));
        // Condition.
        EXPECT_NODE(expect_expression, snots);
        EXPECT_TOKEN(Token_Kind::kw_do, "expected 'DO'", snots);
        Syntax_Node stmt_list = expect_stmt_list();
        snots.push_back(ANTON_MOV(stmt_list));
        EXPECT_TOKEN(Token_Kind::kw_endwhile, "expected 'ENDWHILE'", snots);
        Token_Stream_State const end_state = lexer.get_current_state_noskip();
        Source_Info const source = src_info(begin_state, end_state);
        return Syntax_Node(Syntax_Node_Kind::stmt_while, ANTON_MOV(snots),
                           source);
      }

      if(Optional kw_repeat = match(Token_Kind::kw_repeat)) {
        snots.push_back(ANTON_MOV(*kw_repeat));
        Syntax_Node stmt_list = expect_stmt_list();
        snots.push_back(stmt_list);
        EXPECT_TOKEN(Token_Kind::kw_until, "expected 'UNTIL'"_sv, snots);
        // Condition.
        EXPECT_NODE(expect_expression, snots);
        EXPECT_TOKEN(Token_Kind::tk_semicolon, "expected ';'"_sv, snots);
        Token_Stream_State const end_state = lexer.get_current_state_noskip();
        Source_Info const source = src_info(begin_state, end_state);
        return Syntax_Node(Syntax_Node_Kind::stmt_repeat, ANTON_MOV(snots),
                           source);
      }

      if(Optional kw_read = match(Token_Kind::kw_read)) {
        snots.push_back(ANTON_MOV(*kw_read));
        EXPECT_NODE(expect_expr_identifier, snots);
        EXPECT_TOKEN(Token_Kind::tk_semicolon, "expected ';'"_sv, snots);
        Token_Stream_State const end_state = lexer.get_current_state_noskip();
        Source_Info const source = src_info(begin_state, end_state);
        return Syntax_Node(Syntax_Node_Kind::stmt_read, ANTON_MOV(snots),
                           source);
      }

      if(Optional kw_write = match(Token_Kind::kw_write)) {
        snots.push_back(ANTON_MOV(*kw_write));
        EXPECT_NODE(expect_primary_expression, snots);
        EXPECT_TOKEN(Token_Kind::tk_semicolon, "expected ';'"_sv, snots);
        Token_Stream_State const end_state = lexer.get_current_state_noskip();
        Source_Info const source = src_info(begin_state, end_state);
        return Syntax_Node(Syntax_Node_Kind::stmt_write, ANTON_MOV(snots),
                           source);
      }

      if(Optional stmt_call = expect_stmt_call()) {
        return stmt_call;
      }

      if(Optional stmt_assign = expect_stmt_assign()) {
        return stmt_assign;
      }

      set_error("expected statement"_sv);
      lexer.restore_state(begin_state);
      return anton::null_optional;
    }

    Optional<Syntax_Node> expect_stmt_call()
    {
      Token_Stream_State const begin_state = lexer.get_current_state();
      anton::Array<SNOT> snots{_allocator};
      EXPECT_TOKEN(Token_Kind::identifier, "expected identifier", snots);
      EXPECT_TOKEN(Token_Kind::tk_lparen, "expected '('"_sv, snots);
      // Arguments.
      {
        Token_Stream_State const args_begin_state = lexer.get_current_state();
        anton::Array<SNOT> args_snots{_allocator};
        while(true) {
          EXPECT_NODE(expect_expr_identifier, args_snots);
          if(Optional tk_comma = skipmatch(Token_Kind::tk_comma)) {
            args_snots.push_back(ANTON_MOV(*tk_comma));
          } else {
            break;
          }
        }
        Token_Stream_State const args_end_state =
          lexer.get_current_state_noskip();
        Source_Info const source = src_info(args_begin_state, args_end_state);
        snots.push_back(Syntax_Node(Syntax_Node_Kind::call_arg_list,
                                    ANTON_MOV(args_snots), source));
      }
      EXPECT_TOKEN(Token_Kind::tk_rparen, "expected ')'"_sv, snots);
      EXPECT_TOKEN(Token_Kind::tk_semicolon, "expected ';'"_sv, snots);
      Token_Stream_State const end_state = lexer.get_current_state_noskip();
      Source_Info const source = src_info(begin_state, end_state);
      return Syntax_Node(Syntax_Node_Kind::stmt_call, ANTON_MOV(snots), source);
    }

    Optional<Syntax_Node> expect_stmt_assign()
    {
      Token_Stream_State const begin_state = lexer.get_current_state();
      anton::Array<SNOT> snots{_allocator};
      EXPECT_NODE(expect_expr_identifier, snots);
      EXPECT_TOKEN2(Syntax_Node_Kind::tk_assign, Token_Kind::tk_colon,
                    Token_Kind::tk_equals, "expected ':='"_sv, snots);
      EXPECT_NODE(expect_expression, snots);
      EXPECT_TOKEN(Token_Kind::tk_semicolon, "expected ';'"_sv, snots);
      Token_Stream_State const end_state = lexer.get_current_state_noskip();
      Source_Info const source = src_info(begin_state, end_state);
      return Syntax_Node(Syntax_Node_Kind::stmt_assign, ANTON_MOV(snots),
                         source);
    }

    Optional<Syntax_Node> expect_expression()
    {
      return try_expr_binary();
    }

    // try_expr_binary
    //
    Optional<Syntax_Node> try_expr_binary()
    {
      // Parsing binary expressions consists primarily of repeatedly calling two procedures to
      // construct the expression trees - insert_operator and insert_expression.
      // insert_expression - inserts an expression into a "free slot" (if a binary expression
      //   does not have either right or left expression, we consider it a "free slot").
      // insert_operator - does the actual work of building the binary expression tree.
      //   Depending on the associativity of an operator and its precedence, the following might
      //   happen:
      //   - an operator is left associative: if the operator has a **higher or equal** precedence
      //     than a node we are visiting, then we make the operator a new parent of the node.
      //     Otherwise we descend to the right subtree.
      //   - an operator is right associative: if the operator has a **higher** precedence than a
      //     node we are visiting, then we make the operator a new parent of the node. Otherwise we
      //     descend to the right subtree.
      //   The difference between left and right associative operators is subtle, yet it results in
      //   vastly different expression trees being constructed.
      // Precedence is ranked from 1 (highest).

      auto insert_operator = [_allocator = this->_allocator](Syntax_Node& root,
                                                             Syntax_Token op) {
        enum Associativity { ASSOC_LEFT, ASSOC_RIGHT };

        auto get_associativity = [](Syntax_Node_Kind type) -> Associativity {
          return ASSOC_LEFT;
        };

        auto get_precedence = [](Syntax_Node_Kind type) -> i32 {
          // Precedence values are exactly the same as in the GLSL Specification.
          switch(type) {
          case Syntax_Node_Kind::tk_equals:
          case Syntax_Node_Kind::tk_neq:
            return 8;

          case Syntax_Node_Kind::tk_langle:
          case Syntax_Node_Kind::tk_rangle:
          case Syntax_Node_Kind::tk_gteq:
          case Syntax_Node_Kind::tk_lteq:
            return 7;

          case Syntax_Node_Kind::tk_plus:
          case Syntax_Node_Kind::tk_minus:
            return 5;

          case Syntax_Node_Kind::tk_asterisk:
          case Syntax_Node_Kind::tk_slash:
          case Syntax_Node_Kind::tk_percent:
            return 4;

          default:
            ANTON_UNREACHABLE("invalid operator type");
          }
        };

        i32 const op_prec = get_precedence(op.kind);
        i32 const op_assoc = get_associativity(op.kind);
        Syntax_Node* dest_node = &root;
        while(true) {
          // We will replace the node if it is not a binary expression.
          if(dest_node->kind != Syntax_Node_Kind::expr_binary) {
            break;
          }

          ANTON_ASSERT(dest_node->children.size() == 3,
                       "expr_binary does not have exactly 3 children");
          ANTON_ASSERT(dest_node->children[1].is_right(),
                       "second SNOT of expr_binary is not a Syntax_Token");
          Syntax_Token const& dest_op = dest_node->children[1].right();
          i32 const dest_prec = get_precedence(dest_op.kind);
          if(op_assoc == ASSOC_LEFT) {
            if(op_prec <= dest_prec) {
              dest_node = &dest_node->children[2].left();
            } else {
              break;
            }
          } else {
            if(op_prec < dest_prec) {
              dest_node = &dest_node->children[2].left();
            } else {
              break;
            }
          }
        }

        // Replace dest_node with a new expr_binary node.
        anton::Array<SNOT> snots{_allocator};
        snots.push_back(ANTON_MOV(*dest_node));
        snots.push_back(ANTON_MOV(op));
        *dest_node = Syntax_Node(Syntax_Node_Kind::expr_binary,
                                 ANTON_MOV(snots), Source_Info{});
      };

      auto insert_expression = [](Syntax_Node* root,
                                  Syntax_Node&& expr) -> void {
        // We only ever have to descend down the right children.
        while(root->children.size() == 3) {
          ANTON_ASSERT(root->kind == Syntax_Node_Kind::expr_binary,
                       "Syntax_Node is not expr_binary");
          ANTON_ASSERT(
            root->children[2].is_left(),
            "third child of a binary expression node is not a Syntax_Node");
          root = &root->children[2].left();
        }

        root->children.push_back(ANTON_MOV(expr));
      };

      auto match_binary_operator = [this]() -> Optional<Syntax_Token> {
        // We match operators in the following order:
        // 1. Relational neq, gte, lte,
        // 2. Remaining operators (relational, additive, multiplicative).

        if(Optional op =
             skipmatch(Syntax_Node_Kind::tk_neq, Token_Kind::tk_bang,
                       Token_Kind::tk_equals)) {
          return ANTON_MOV(op);
        }

        if(Optional op =
             skipmatch(Syntax_Node_Kind::tk_lteq, Token_Kind::tk_langle,
                       Token_Kind::tk_equals)) {
          return ANTON_MOV(op);
        }
        if(Optional op =
             skipmatch(Syntax_Node_Kind::tk_gteq, Token_Kind::tk_rangle,
                       Token_Kind::tk_equals)) {
          return ANTON_MOV(op);
        }

        if(Optional op = skipmatch(Token_Kind::tk_equals)) {
          return ANTON_MOV(op);
        }
        if(Optional op = skipmatch(Token_Kind::tk_langle)) {
          return ANTON_MOV(op);
        }
        if(Optional op = skipmatch(Token_Kind::tk_rangle)) {
          return ANTON_MOV(op);
        }
        if(Optional op = skipmatch(Token_Kind::tk_plus)) {
          return ANTON_MOV(op);
        }
        if(Optional op = skipmatch(Token_Kind::tk_minus)) {
          return ANTON_MOV(op);
        }
        if(Optional op = skipmatch(Token_Kind::tk_asterisk)) {
          return ANTON_MOV(op);
        }
        if(Optional op = skipmatch(Token_Kind::tk_slash)) {
          return ANTON_MOV(op);
        }
        if(Optional op = skipmatch(Token_Kind::tk_percent)) {
          return ANTON_MOV(op);
        }

        return anton::null_optional;
      };

      Token_Stream_State const begin_state = lexer.get_current_state();
      Optional root_expr = expect_primary_expression();
      if(!root_expr) {
        return anton::null_optional;
      }

      while(true) {
        if(Optional op = match_binary_operator()) {
          insert_operator(*root_expr, ANTON_MOV(*op));
        } else {
          break;
        }

        if(Optional expr = expect_primary_expression()) {
          insert_expression(&root_expr.value(), ANTON_MOV(*expr));
        } else {
          lexer.restore_state(begin_state);
          return anton::null_optional;
        }
      }

      auto recalculate_source_info = [](auto& recalculate_source_info,
                                        Syntax_Node& node) -> void {
        if(node.kind != Syntax_Node_Kind::expr_binary) {
          return;
        }

        ANTON_ASSERT(node.children[0].is_left(),
                     "the left child of expr_binary is not a Syntax_Node");
        ANTON_ASSERT(node.children[2].is_left(),
                     "the right child of expr_binary is not a Syntax_Node");
        recalculate_source_info(recalculate_source_info,
                                node.children[0].left());
        recalculate_source_info(recalculate_source_info,
                                node.children[2].left());
        Source_Info const& left = node.children[0].left().source_info;
        Source_Info const& right = node.children[2].left().source_info;
        // Both left and right have the same souce_path;
        node.source_info.source_path = left.source_path;
        node.source_info.line = left.line;
        node.source_info.column = left.column;
        node.source_info.offset = left.offset;
        node.source_info.end_line = right.end_line;
        node.source_info.end_column = right.end_column;
        node.source_info.end_offset = right.end_offset;
      };

      // expr_binary nodes do not have their source_info members filled properly because
      // it is impossible to do so while inserting (we do not know the final right child).
      // We have to manually fix up the source_info members.
      recalculate_source_info(recalculate_source_info, *root_expr);

      return root_expr;
    }

    Optional<Syntax_Node> expect_primary_expression()
    {
      Token_Stream_State const begin_state = lexer.get_current_state();
      if(Optional lt_integer = match(Token_Kind::lt_dec_integer)) {
        anton::Array<SNOT> snots{_allocator};
        snots.push_back(ANTON_MOV(*lt_integer));
        Token_Stream_State const end_state = lexer.get_current_state_noskip();
        Source_Info const source = src_info(begin_state, end_state);
        return Syntax_Node(Syntax_Node_Kind::expr_lt_integer, ANTON_MOV(snots),
                           source);
      }

      return expect_expr_identifier();
    };

    Optional<Syntax_Node> expect_expr_identifier()
    {
      Token_Stream_State const begin_state = lexer.get_current_state();
      anton::Array<SNOT> snots{_allocator};
      EXPECT_TOKEN(Token_Kind::identifier, "expected identifier"_sv, snots);
      if(Optional tk_lbracket = skipmatch(Token_Kind::tk_lbracket)) {
        snots.push_back(ANTON_MOV(*tk_lbracket));
        EXPECT_NODE(expect_expression, snots);
        EXPECT_TOKEN(Token_Kind::tk_rbracket, "expected ']'"_sv, snots);
        Token_Stream_State const end_state = lexer.get_current_state_noskip();
        Source_Info const source = src_info(begin_state, end_state);
        return Syntax_Node(Syntax_Node_Kind::expr_index, ANTON_MOV(snots),
                           source);
      } else {
        Token_Stream_State const end_state = lexer.get_current_state_noskip();
        Source_Info const source = src_info(begin_state, end_state);
        return Syntax_Node(Syntax_Node_Kind::expr_identifier, ANTON_MOV(snots),
                           source);
      }
    }
  };

  anton::Expected<anton::Array<SNOT>, Error>
  parse_tokens(FE_Context const& ctx, anton::String_View const source_path,
               anton::Slice<Token const> const tokens)
  {
    Parser parser(ctx.allocator, source_path,
                  Token_Stream(tokens.begin(), tokens.end()));
    anton::Expected<anton::Array<SNOT>, Error> syntax_tree =
      parser.build_syntax_tree();
    return syntax_tree;
  }
} // namespace glang
