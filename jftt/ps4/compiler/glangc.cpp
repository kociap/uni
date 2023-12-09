#include "anton/stream.hpp"
#include "glang_syntax/syntax.hpp"
#include <anton/allocator.hpp>
#include <anton/console.hpp>
#include <anton/expected.hpp>
#include <anton/filesystem.hpp>
#include <anton/format.hpp>
#include <anton/string.hpp>

#include <glang_core/types.hpp>
#include <glang_frontend/fe_context.hpp>
#include <glang_lexer/lexer.hpp>
#include <glang_syntax/parser.hpp>

namespace glang {
  using namespace anton::literals;

  [[nodiscard]] anton::Expected<anton::String, anton::String>
  read_file(Allocator* const allocator, anton::String const& path)
  {
    anton::fs::Input_File_Stream file;
    if(!file.open(path)) {
      return {
        anton::expected_error,
        anton::format(allocator, u8"could not open '{}' for reading", path)};
    }

    file.seek(anton::Seek_Dir::end, 0);
    i64 size = file.tell();
    file.seek(anton::Seek_Dir::beg, 0);
    anton::String file_contents{anton::reserve, size, allocator};
    file_contents.force_size(size);
    file.read(file_contents.data(), size);
    return {anton::expected_value, ANTON_MOV(file_contents)};
  }

  [[nodiscard]] anton::String_View stringify(Token_Kind const kind)
  {
    switch(kind) {
    case Token_Kind::identifier:
      return "identifier"_sv;
    case Token_Kind::comment:
      return "comment"_sv;
    case Token_Kind::whitespace:
      return "whitespace"_sv;
    case Token_Kind::kw_procedure:
      return "kw_procedure"_sv;
    case Token_Kind::kw_is:
      return "kw_is"_sv;
    case Token_Kind::kw_in:
      return "kw_in"_sv;
    case Token_Kind::kw_end:
      return "kw_end"_sv;
    case Token_Kind::kw_program:
      return "kw_program"_sv;
    case Token_Kind::kw_if:
      return "kw_if"_sv;
    case Token_Kind::kw_then:
      return "kw_then"_sv;
    case Token_Kind::kw_else:
      return "kw_else"_sv;
    case Token_Kind::kw_endif:
      return "kw_endif"_sv;
    case Token_Kind::kw_while:
      return "kw_while"_sv;
    case Token_Kind::kw_do:
      return "kw_do"_sv;
    case Token_Kind::kw_endwhile:
      return "kw_endwhile"_sv;
    case Token_Kind::kw_repeat:
      return "kw_repeat"_sv;
    case Token_Kind::kw_until:
      return "kw_until"_sv;
    case Token_Kind::kw_read:
      return "kw_read"_sv;
    case Token_Kind::kw_write:
      return "kw_write"_sv;
    case Token_Kind::kw_t:
      return "kw_t"_sv;
    case Token_Kind::tk_lbracket:
      return "tk_lbracket"_sv;
    case Token_Kind::tk_rbracket:
      return "tk_rbracket"_sv;
    case Token_Kind::tk_lparen:
      return "tk_lparen"_sv;
    case Token_Kind::tk_rparen:
      return "tk_rparen"_sv;
    case Token_Kind::tk_langle:
      return "tk_langle"_sv;
    case Token_Kind::tk_rangle:
      return "tk_rangle"_sv;
    case Token_Kind::tk_semicolon:
      return "tk_semicolon"_sv;
    case Token_Kind::tk_colon:
      return "tk_colon"_sv;
    case Token_Kind::tk_comma:
      return "tk_comma"_sv;
    case Token_Kind::tk_plus:
      return "tk_plus"_sv;
    case Token_Kind::tk_minus:
      return "tk_minus"_sv;
    case Token_Kind::tk_asterisk:
      return "tk_asterisk"_sv;
    case Token_Kind::tk_slash:
      return "tk_slash"_sv;
    case Token_Kind::tk_percent:
      return "tk_percent"_sv;
    case Token_Kind::tk_bang:
      return "tk_bang"_sv;
    case Token_Kind::tk_equals:
      return "tk_equals"_sv;
    case Token_Kind::lt_dec_integer:
      return "lt_dec_integer"_sv;
    }
  }

  [[nodiscard]] static anton::String_View stringify(Syntax_Node_Kind const kind)
  {
    switch(kind) {
    case Syntax_Node_Kind::identifier:
      return "identifier"_sv;
    case Syntax_Node_Kind::comment:
      return "comment"_sv;
    case Syntax_Node_Kind::whitespace:
      return "whitespace"_sv;
    case Syntax_Node_Kind::kw_procedure:
      return "kw_procedure"_sv;
    case Syntax_Node_Kind::kw_is:
      return "kw_is"_sv;
    case Syntax_Node_Kind::kw_in:
      return "kw_in"_sv;
    case Syntax_Node_Kind::kw_end:
      return "kw_end"_sv;
    case Syntax_Node_Kind::kw_program:
      return "kw_program"_sv;
    case Syntax_Node_Kind::kw_if:
      return "kw_if"_sv;
    case Syntax_Node_Kind::kw_then:
      return "kw_then"_sv;
    case Syntax_Node_Kind::kw_else:
      return "kw_else"_sv;
    case Syntax_Node_Kind::kw_endif:
      return "kw_endif"_sv;
    case Syntax_Node_Kind::kw_while:
      return "kw_while"_sv;
    case Syntax_Node_Kind::kw_do:
      return "kw_do"_sv;
    case Syntax_Node_Kind::kw_endwhile:
      return "kw_endwhile"_sv;
    case Syntax_Node_Kind::kw_repeat:
      return "kw_repeat"_sv;
    case Syntax_Node_Kind::kw_until:
      return "kw_until"_sv;
    case Syntax_Node_Kind::kw_read:
      return "kw_read"_sv;
    case Syntax_Node_Kind::kw_write:
      return "kw_write"_sv;
    case Syntax_Node_Kind::kw_t:
      return "kw_t"_sv;
    case Syntax_Node_Kind::tk_lbracket:
      return "tk_lbracket"_sv;
    case Syntax_Node_Kind::tk_rbracket:
      return "tk_rbracket"_sv;
    case Syntax_Node_Kind::tk_lparen:
      return "tk_lparen"_sv;
    case Syntax_Node_Kind::tk_rparen:
      return "tk_rparen"_sv;
    case Syntax_Node_Kind::tk_langle:
      return "tk_langle"_sv;
    case Syntax_Node_Kind::tk_rangle:
      return "tk_rangle"_sv;
    case Syntax_Node_Kind::tk_semicolon:
      return "tk_semicolon"_sv;
    case Syntax_Node_Kind::tk_colon:
      return "tk_colon"_sv;
    case Syntax_Node_Kind::tk_comma:
      return "tk_comma"_sv;
    case Syntax_Node_Kind::tk_plus:
      return "tk_plus"_sv;
    case Syntax_Node_Kind::tk_minus:
      return "tk_minus"_sv;
    case Syntax_Node_Kind::tk_asterisk:
      return "tk_asterisk"_sv;
    case Syntax_Node_Kind::tk_slash:
      return "tk_slash"_sv;
    case Syntax_Node_Kind::tk_percent:
      return "tk_percent"_sv;
    case Syntax_Node_Kind::tk_bang:
      return "tk_bang"_sv;
    case Syntax_Node_Kind::tk_equals:
      return "tk_equals"_sv;
    case Syntax_Node_Kind::lt_dec_integer:
      return "lt_dec_integer"_sv;
    case Syntax_Node_Kind::tk_neq:
      return "tk_neq"_sv;
    case Syntax_Node_Kind::tk_lteq:
      return "tk_lteq"_sv;
    case Syntax_Node_Kind::tk_gteq:
      return "tk_gteq"_sv;
    case Syntax_Node_Kind::tk_assign:
      return "tk_assign"_sv;
    case Syntax_Node_Kind::variable:
      return "variable"_sv;
    case Syntax_Node_Kind::fn_parameter:
      return "fn_parameter"_sv;
    case Syntax_Node_Kind::fn_parameter_list:
      return "fn_parameter_list"_sv;
    case Syntax_Node_Kind::fn_definition_list:
      return "fn_definition_list"_sv;
    case Syntax_Node_Kind::decl_procedure:
      return "decl_procedure"_sv;
    case Syntax_Node_Kind::decl_main:
      return "decl_main"_sv;
    case Syntax_Node_Kind::call_arg_list:
      return "call_arg_list"_sv;
    case Syntax_Node_Kind::expr_identifier:
      return "expr_identifier"_sv;
    case Syntax_Node_Kind::expr_binary:
      return "expr_binary"_sv;
    case Syntax_Node_Kind::expr_index:
      return "expr_index"_sv;
    case Syntax_Node_Kind::expr_lt_integer:
      return "expr_lt_integer"_sv;
    case Syntax_Node_Kind::stmt_list:
      return "stmt_list"_sv;
    case Syntax_Node_Kind::stmt_call:
      return "stmt_call"_sv;
    case Syntax_Node_Kind::stmt_if:
      return "stmt_if"_sv;
    case Syntax_Node_Kind::stmt_while:
      return "stmt_while"_sv;
    case Syntax_Node_Kind::stmt_repeat:
      return "stmt_repeat"_sv;
    case Syntax_Node_Kind::stmt_assign:
      return "stmt_assign"_sv;
    case Syntax_Node_Kind::stmt_read:
      return "stmt_read"_sv;
    case Syntax_Node_Kind::stmt_write:
      return "stmt_write"_sv;
    }
  }

  static void print_syntax_tree(anton::Output_Stream& stream, SNOT const& snot,
                                i64 const indent)
  {
    for(i64 i = 0; i < indent; i += 1) {
      stream.write(" "_sv);
    }
    if(snot.is_left()) {
      Syntax_Node const& node = snot.left();
      stream.write(anton::format(
        "NODE {} [children: {}] @ {}:{}\n"_sv, stringify(node.kind),
        node.children.size(), node.source_info.line, node.source_info.column));
      for(SNOT const& snot: node.children) {
        print_syntax_tree(stream, snot, indent + 2);
      }
    } else {
      Syntax_Token const& node = snot.right();
      stream.write(anton::format(
        "TOKEN {} ['{}'] @ {}:{}\n"_sv, stringify(node.kind), node.value,
        node.source_info.line, node.source_info.column));
    }
  }

  [[nodiscard]] static anton::Expected<void, anton::String>
  compile(Allocator* const allocator, anton::String const& infile,
          anton::String const& outfile, Options const options)
  {
    FE_Context fe_ctx;
    fe_ctx.allocator = allocator;
    fe_ctx.options = options;
    anton::Expected<anton::String, anton::String> source_result =
      read_file(allocator, infile);
    if(!source_result) {
      return {anton::expected_error, ANTON_MOV(source_result.error())};
    }
    anton::String_View const source = source_result.value();
    anton::Expected<anton::Array<Token>, Error> lex_result =
      lex_source(fe_ctx, infile,
                 anton::String7_View{source.bytes_begin(), source.bytes_end()});
    if(!lex_result) {
      return {
        anton::expected_error,
        lex_result.error().format(allocator, options.extended_diagnostics)};
    }

    anton::Expected<anton::Array<SNOT>, Error> parse_result =
      parse_tokens(fe_ctx, infile, lex_result.value());
    if(!parse_result) {
      return {
        anton::expected_error,
        parse_result.error().format(allocator, options.extended_diagnostics)};
    }

    anton::fs::Output_File_Stream out;
    if(!out.open(outfile)) {
      return {anton::expected_error,
              anton::String("outfile could not be opened", allocator)};
    }

    // for(Token const& token: lex_result.value()) {
    //   anton::String_View value{token.value.begin(), token.value.end()};
    //   anton::String_View kind = stringify(token.kind);
    //   out.write(anton::format("{}: {}\n"_sv, kind, value));
    // }

    for(SNOT const& snot: parse_result.value()) {
      print_syntax_tree(out, snot, 0);
    }

    return anton::expected_value;
  }
} // namespace glang

int main(int argc, char** argv)
{
  using namespace glang;
  anton::Arena_Allocator allocator;
  if(argc < 3) {
    return 1;
  }

  anton::STDOUT_Stream stdout;

  anton::String infile(argv[1]);
  anton::String outfile(argv[2]);
  Options options;
  anton::Expected<void, anton::String> result =
    compile(&allocator, infile, outfile, options);
  if(!result) {
    stdout.write(result.error());
    return 1;
  }
  return 0;
}
