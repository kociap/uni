#include "glang_ir/ir.hpp"
#include <anton/allocator.hpp>
#include <anton/expected.hpp>
#include <anton/filesystem.hpp>
#include <anton/format.hpp>
#include <anton/sort.hpp>
#include <anton/stdio.hpp>
#include <anton/string.hpp>

#include <glang_ast/syntax_lowering.hpp>
#include <glang_codegen_vm/ir_lowering.hpp>
#include <glang_codegen_vm/passes.hpp>
#include <glang_core/types.hpp>
#include <glang_frontend/fe_context.hpp>
#include <glang_ir/ast_lowering.hpp>
#include <glang_ir/passes.hpp>
#include <glang_lexer/lexer.hpp>
#include <glang_sema/sema.hpp>
#include <glang_syntax/parser.hpp>

#include <glang_codegen_vm/prettyprint.hpp>
#include <glang_ir/prettyprint.hpp>

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

    anton::Expected<ast::Node_List, Error> syntax_lowering_result =
      lower_syntax_to_ast(fe_ctx, parse_result.value());
    if(!syntax_lowering_result) {
      return {anton::expected_error,
              syntax_lowering_result.error().format(
                allocator, options.extended_diagnostics)};
    }

    ast::Node_List const ast_nodes = syntax_lowering_result.value();

    anton::Expected<void, Error> sema_result = run_sema(fe_ctx, ast_nodes);
    if(!sema_result) {
      return {
        anton::expected_error,
        sema_result.error().format(allocator, options.extended_diagnostics)};
    }

    ir::Context ir_ctx(allocator);

#define EMIT_LIR 1
#define RUN_OPT 1

    IList<ir::Function> ir_functions = lower_ast_to_ir(ir_ctx, ast_nodes);
    // There be dragons beyond this point. And other atrocities you might want
    // to not behold yourself.
    for(ir::Function* const fn: ir_functions) {
      canonicalise_brcond(ir_ctx, fn);
      lower_regs_to_mem(ir_ctx, fn);

#if RUN_OPT
      while(true) {
        bool progress = false;
        progress |= opt_coalesce_loads(ir_ctx, fn);
        if(!progress) {
          break;
        }
      }
#endif

      anton::quick_sort(
        fn->locals.begin(), fn->locals.end(),
        [](ir::Instr_variable* const lhs, ir::Instr_variable* const rhs) {
          return lhs->referrers.size() > rhs->referrers.size();
        });
    }

#if EMIT_LIR
    IList<lir::Instr> lir_instructions =
      lower_ir_to_lir(allocator, ir_functions);

    assign_addresses(lir_instructions);
#endif

    anton::fs::Output_File_Stream out;
    if(!out.open(outfile)) {
      return {anton::expected_error,
              anton::String("outfile could not be opened", allocator)};
    }

#if !EMIT_LIR
    for(ir::Function* function: ir_functions) {
      prettyprint_ir(allocator, out, function);
    }
#else
    prettyprint_lir(allocator, out, lir_instructions);
#endif

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

  anton::String infile(argv[1]);
  anton::String outfile(argv[2]);
  Options options;
  options.enable_case_sensitive_keywords = true;
  anton::Expected<void, anton::String> result =
    compile(&allocator, infile, outfile, options);
  if(!result) {
    anton::print(result.error());
    return 1;
  }
  return 0;
}
