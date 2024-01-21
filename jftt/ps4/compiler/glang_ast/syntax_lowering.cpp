#include "glang_syntax/syntax.hpp"
#include <glang_ast/syntax_lowering.hpp>

#include <anton/expected.hpp>
#include <anton/intrinsics.hpp>
#include <anton/optional.hpp>

#include <glang_ast/ast.hpp>
#include <glang_core/memory.hpp>
#include <glang_frontend/fe_context.hpp>
#include <glang_syntax/syntax_accessors.hpp>

namespace glang {
  using namespace anton::literals;

  [[nodiscard]] static ast::Identifier
  lower_identifier(FE_Context const& ctx, Syntax_Token const& token)
  {
    anton::String const* const value =
      allocate<anton::String>(ctx.allocator, token.value, ctx.allocator);
    return ast::Identifier{*value, token.source_info};
  }

  [[nodiscard]] static ast::Lt_Integer*
  lower_lt_integer(FE_Context const& ctx, Syntax_Node const& node)
  {
    Syntax_Token const& value_token = get_expr_lt_integer_value(node);

    i32 const value = anton::str_to_i64(value_token.value, 10);
    return allocate<ast::Lt_Integer>(ctx.allocator, value, node.source_info);
  }

  [[nodiscard]] static ast::Lt_Integer*
  lower_lt_integer(FE_Context const& ctx, Syntax_Token const& token)
  {
    i32 const value = anton::str_to_i64(token.value, 10);
    return allocate<ast::Lt_Integer>(ctx.allocator, value, token.source_info);
  }

  [[nodiscard]] static ast::Expr_Binary_Kind
  token_to_binary_kind(Syntax_Token const& token)
  {
    switch(token.kind) {
    case Syntax_Node_Kind::tk_plus:
      return ast::Expr_Binary_Kind::add;
    case Syntax_Node_Kind::tk_minus:
      return ast::Expr_Binary_Kind::sub;
    case Syntax_Node_Kind::tk_asterisk:
      return ast::Expr_Binary_Kind::mul;
    case Syntax_Node_Kind::tk_slash:
      return ast::Expr_Binary_Kind::div;
    case Syntax_Node_Kind::tk_percent:
      return ast::Expr_Binary_Kind::mod;
    case Syntax_Node_Kind::tk_equals:
      return ast::Expr_Binary_Kind::eq;
    case Syntax_Node_Kind::tk_neq:
      return ast::Expr_Binary_Kind::neq;
    case Syntax_Node_Kind::tk_langle:
      return ast::Expr_Binary_Kind::lt;
    case Syntax_Node_Kind::tk_rangle:
      return ast::Expr_Binary_Kind::gt;
    case Syntax_Node_Kind::tk_lteq:
      return ast::Expr_Binary_Kind::leq;
    case Syntax_Node_Kind::tk_gteq:
      return ast::Expr_Binary_Kind::geq;
    default:
      ANTON_UNREACHABLE("invalid token kind");
    }
  }

  [[nodiscard]] static anton::Expected<ast::Expr*, Error>
  lower_expr(FE_Context const& ctx, Syntax_Node const& node)
  {
    switch(node.kind) {
    case Syntax_Node_Kind::expr_identifier: {
      Syntax_Token const& value_token = get_expr_identifier_value(node);
      anton::String const* const value = allocate<anton::String>(
        ctx.allocator, value_token.value, ctx.allocator);
      return {anton::expected_value,
              allocate<ast::Expr_Identifier>(ctx.allocator, *value,
                                             node.source_info)};
    } break;

    case Syntax_Node_Kind::expr_binary: {
      Syntax_Node const& lhs_node = get_expr_binary_lhs(node);
      anton::Expected<ast::Expr*, Error> lhs = lower_expr(ctx, lhs_node);
      if(!lhs) {
        return ANTON_MOV(lhs);
      }

      Syntax_Node const& rhs_node = get_expr_binary_rhs(node);
      anton::Expected<ast::Expr*, Error> rhs = lower_expr(ctx, rhs_node);
      if(!rhs) {
        return ANTON_MOV(rhs);
      }

      Syntax_Token const& operator_token = get_expr_binary_operator(node);
      ast::Expr_Binary_Kind const kind = token_to_binary_kind(operator_token);
      return {anton::expected_value,
              allocate<ast::Expr_Binary>(ctx.allocator, kind, lhs.value(),
                                         rhs.value(), node.source_info)};
    } break;

    case Syntax_Node_Kind::expr_index: {
      Syntax_Token const& identifier_token = get_expr_index_base(node);
      ast::Identifier identifier = lower_identifier(ctx, identifier_token);
      Syntax_Node const& index_node = get_expr_index_index(node);
      anton::Expected<ast::Expr*, Error> index = lower_expr(ctx, index_node);
      if(!index) {
        return ANTON_MOV(index);
      }

      return {anton::expected_value,
              allocate<ast::Expr_Index>(ctx.allocator, identifier,
                                        index.value(), node.source_info)};
    } break;

    case Syntax_Node_Kind::expr_lt_integer: {
      return {anton::expected_value, lower_lt_integer(ctx, node)};
    } break;

    default:
      ANTON_UNREACHABLE("invalid node kind");
    }
  }

  // lower_stmt
  //
  // Returns:
  // nullptr if the statement does not have a representation in the AST.
  //
  [[nodiscard]] static anton::Expected<ast::Node*, Error>
  lower_stmt(FE_Context const& ctx, Syntax_Node const& node);

  [[nodiscard]] static anton::Expected<ast::Node_List, Error>
  lower_stmt_list(FE_Context const& ctx, Syntax_Node const& node)
  {
    ANTON_ASSERT(node.kind == Syntax_Node_Kind::stmt_list,
                 "Syntax_Node is not stmt_list");
    auto& statements =
      *allocate<anton::Array<ast::Node*>>(ctx.allocator, ctx.allocator);
    for(SNOT const& snot: node.children) {
      if(!snot.is_left()) {
        continue;
      }

      Syntax_Node const& stmt_node = snot.left();
      anton::Expected<ast::Node*, Error> stmt = lower_stmt(ctx, stmt_node);
      if(stmt) {
        if(stmt.value() != nullptr) {
          statements.push_back(stmt.value());
        }
      } else {
        return {anton::expected_error, ANTON_MOV(stmt.error())};
      }
    }
    return {anton::expected_value, statements};
  }

  [[nodiscard]] static ast::Variable* lower_variable(FE_Context const& ctx,
                                                     Syntax_Node const& node)
  {
    ast::Identifier const identifier =
      lower_identifier(ctx, get_variable_identifier(node));
    ast::Lt_Integer* size = nullptr;
    anton::Optional<Syntax_Token const&> size_token = get_variable_size(node);
    if(size_token) {
      size = lower_lt_integer(ctx, size_token.value());
    }
    return allocate<ast::Variable>(ctx.allocator, identifier, size,
                                   node.source_info);
  }

  anton::Expected<ast::Node*, Error> lower_stmt(FE_Context const& ctx,
                                                Syntax_Node const& node)
  {
    switch(node.kind) {
    case Syntax_Node_Kind::stmt_call: {
      Syntax_Token const& identifier_token = get_stmt_call_identifier(node);
      ast::Identifier const identifier =
        lower_identifier(ctx, identifier_token);

      Syntax_Node const& arguments_node = get_stmt_call_arguments(node);
      auto& arguments =
        *allocate<anton::Array<ast::Expr*>>(ctx.allocator, ctx.allocator);
      for(SNOT const& snot: arguments_node.children) {
        if(snot.is_left()) {
          anton::Expected<ast::Expr*, Error> expression =
            lower_expr(ctx, snot.left());
          if(expression) {
            arguments.push_back(expression.value());
          } else {
            return {anton::expected_error, ANTON_MOV(expression.error())};
          }
        }
      }

      return {anton::expected_value,
              allocate<ast::Stmt_Call>(ctx.allocator, identifier, arguments,
                                       node.source_info)};
    } break;

    case Syntax_Node_Kind::stmt_if: {
      anton::Expected<ast::Expr*, Error> condition =
        lower_expr(ctx, get_stmt_if_condition(node));
      if(!condition) {
        return {anton::expected_error, ANTON_MOV(condition.error())};
      }

      anton::Expected<ast::Node_List, Error> then_branch =
        lower_stmt_list(ctx, get_stmt_if_then_branch(node));
      if(!then_branch) {
        return {anton::expected_error, ANTON_MOV(then_branch.error())};
      }

      ast::Node_List else_branch;
      if(anton::Optional<Syntax_Node const&> else_node =
           get_stmt_if_else_branch(node)) {
        anton::Expected<ast::Node_List, Error> result =
          lower_stmt_list(ctx, else_node.value());
        if(result) {
          else_branch = result.value();
        } else {
          return {anton::expected_error, ANTON_MOV(result.error())};
        }
      }

      return {anton::expected_value,
              allocate<ast::Stmt_If>(ctx.allocator, condition.value(),
                                     then_branch.value(), else_branch,
                                     node.source_info)};
    } break;

    case Syntax_Node_Kind::stmt_while: {
      anton::Expected<ast::Expr*, Error> condition =
        lower_expr(ctx, get_stmt_while_condition(node));
      if(!condition) {
        return {anton::expected_error, ANTON_MOV(condition.error())};
      }

      anton::Expected<ast::Node_List, Error> statements =
        lower_stmt_list(ctx, get_stmt_while_statements(node));
      if(!statements) {
        return {anton::expected_error, ANTON_MOV(statements.error())};
      }

      return {anton::expected_value,
              allocate<ast::Stmt_While>(ctx.allocator, condition.value(),
                                        statements.value(), node.source_info)};
    } break;

    case Syntax_Node_Kind::stmt_repeat: {
      anton::Expected<ast::Node_List, Error> statements =
        lower_stmt_list(ctx, get_stmt_repeat_statements(node));
      if(!statements) {
        return {anton::expected_error, ANTON_MOV(statements.error())};
      }

      anton::Expected<ast::Expr*, Error> condition =
        lower_expr(ctx, get_stmt_repeat_condition(node));
      if(!condition) {
        return {anton::expected_error, ANTON_MOV(condition.error())};
      }

      return {anton::expected_value,
              allocate<ast::Stmt_Repeat>(ctx.allocator, condition.value(),
                                         statements.value(), node.source_info)};
    } break;

    case Syntax_Node_Kind::stmt_assign: {
      Syntax_Node const& dst_node = get_stmt_assign_dst(node);
      anton::Expected<ast::Expr*, Error> dst_result = lower_expr(ctx, dst_node);
      if(!dst_result) {
        return {anton::expected_error, ANTON_MOV(dst_result.error())};
      }

      Syntax_Node const& src_node = get_stmt_assign_src(node);
      anton::Expected<ast::Expr*, Error> src_result = lower_expr(ctx, src_node);
      if(!src_result) {
        return {anton::expected_error, ANTON_MOV(src_result.error())};
      }

      return {anton::expected_value,
              allocate<ast::Stmt_Assign>(ctx.allocator, dst_result.value(),
                                         src_result.value(), node.source_info)};
    } break;

    case Syntax_Node_Kind::stmt_read: {
      anton::Expected<ast::Expr*, Error> dst =
        lower_expr(ctx, get_stmt_read_dst(node));
      if(!dst) {
        return {anton::expected_error, ANTON_MOV(dst.error())};
      }

      return {
        anton::expected_value,
        allocate<ast::Stmt_Read>(ctx.allocator, dst.value(), node.source_info)};
    } break;

    case Syntax_Node_Kind::stmt_write: {
      anton::Expected<ast::Expr*, Error> src =
        lower_expr(ctx, get_stmt_write_src(node));
      if(!src) {
        return {anton::expected_error, ANTON_MOV(src.error())};
      }

      return {anton::expected_value,
              allocate<ast::Stmt_Write>(ctx.allocator, src.value(),
                                        node.source_info)};
    } break;

    default:
      ANTON_UNREACHABLE("unhandled node kind");
    }
  }

  [[nodiscard]] static anton::Expected<ast::Procedure_Parameter*, Error>
  lower_parameter(FE_Context const& ctx, Syntax_Node const& node)
  {
    ANTON_ASSERT(node.kind == Syntax_Node_Kind::procedure_parameter,
                 "node is not procedure_parameter");
    ast::Identifier const identifier =
      lower_identifier(ctx, get_procedure_parameter_identifier(node));
    anton::Optional<Syntax_Token const&> parameter_T =
      get_procedure_parameter_T(node);

    return {anton::expected_value,
            allocate<ast::Procedure_Parameter>(ctx.allocator, identifier,
                                               parameter_T.holds_value(),
                                               node.source_info)};
  }

  [[nodiscard]] static anton::Expected<ast::Procedure_Parameter_List, Error>
  lower_parameter_list(FE_Context const& ctx, Syntax_Node const& node)
  {
    ANTON_ASSERT(node.kind == Syntax_Node_Kind::procedure_parameter_list,
                 "node is not procedure_parameter_list");
    auto& parameters = *allocate<anton::Array<ast::Procedure_Parameter*>>(
      ctx.allocator, ctx.allocator);
    for(SNOT const& snot: node.children) {
      if(!snot.is_left()) {
        continue;
      }

      Syntax_Node const& parameter_node = snot.left();
      anton::Expected<ast::Procedure_Parameter*, Error> parameter =
        lower_parameter(ctx, parameter_node);
      if(parameter) {
        parameters.push_back(parameter.value());
      } else {
        return {anton::expected_error, ANTON_MOV(parameter.error())};
      }
    }
    return {anton::expected_value, parameters};
  }

  [[nodiscard]] static ast::Procedure_Declaration_List
  lower_declaration_list(FE_Context const& ctx, Syntax_Node const& node)
  {
    ANTON_ASSERT(node.kind == Syntax_Node_Kind::procedure_declaration_list,
                 "node is not procedure_declaration_list");
    auto& declarations =
      *allocate<anton::Array<ast::Variable*>>(ctx.allocator, ctx.allocator);
    for(SNOT const& snot: node.children) {
      if(!snot.is_left()) {
        continue;
      }

      Syntax_Node const& parameter_node = snot.left();
      ast::Variable* declaration = lower_variable(ctx, parameter_node);
      declarations.push_back(declaration);
    }
    return declarations;
  }

  [[nodiscard]] static anton::Expected<ast::Decl_Procedure*, Error>
  lower_decl_procedure(FE_Context const& ctx, Syntax_Node const& node)
  {
    ANTON_ASSERT(node.kind == Syntax_Node_Kind::decl_procedure,
                 "node is not decl_procedure");
    ast::Identifier const identifier =
      lower_identifier(ctx, get_decl_procedure_identifier(node));
    anton::Expected<ast::Procedure_Parameter_List, Error> parameters =
      lower_parameter_list(ctx, get_decl_procedure_parameter_list(node));
    if(!parameters) {
      return {anton::expected_error, ANTON_MOV(parameters.error())};
    }

    ast::Procedure_Declaration_List declarations =
      lower_declaration_list(ctx, get_decl_procedure_declaration_list(node));

    anton::Expected<ast::Node_List, Error> body =
      lower_stmt_list(ctx, get_decl_procedure_body(node));
    if(!body) {
      return {anton::expected_error, ANTON_MOV(body.error())};
    }

    return {anton::expected_value,
            allocate<ast::Decl_Procedure>(ctx.allocator, identifier,
                                          parameters.value(), declarations,
                                          body.value(), node.source_info)};
  }

  [[nodiscard]] static anton::Expected<ast::Decl_Main*, Error>
  lower_decl_main(FE_Context const& ctx, Syntax_Node const& node)
  {
    ast::Procedure_Declaration_List declarations =
      lower_declaration_list(ctx, get_decl_main_declaration_list(node));
    anton::Expected<ast::Node_List, Error> body =
      lower_stmt_list(ctx, get_decl_main_body(node));
    if(!body) {
      return {anton::expected_error, ANTON_MOV(body.error())};
    }

    return {anton::expected_value,
            allocate<ast::Decl_Main>(ctx.allocator, declarations, body.value(),
                                     node.source_info)};
  }

  anton::Expected<ast::Node_List, Error>
  lower_syntax_to_ast(FE_Context& ctx, anton::Slice<SNOT const> const syntax)
  {
    auto& abstract =
      *allocate<anton::Array<ast::Node*>>(ctx.allocator, ctx.allocator);
    for(SNOT const& snot: syntax) {
      if(!snot.is_left()) {
        continue;
      }

      Syntax_Node const& syntax_node = snot.left();
      switch(syntax_node.kind) {
      case Syntax_Node_Kind::decl_procedure: {
        anton::Expected<ast::Decl_Procedure*, Error> result =
          lower_decl_procedure(ctx, syntax_node);
        if(!result) {
          return {anton::expected_error, ANTON_MOV(result.error())};
        }

        ast::Decl_Procedure* const decl = result.value();
        abstract.insert(abstract.end(), decl);
      } break;

      case Syntax_Node_Kind::decl_main: {
        anton::Expected<ast::Decl_Main*, Error> result =
          lower_decl_main(ctx, syntax_node);
        if(!result) {
          return {anton::expected_error, ANTON_MOV(result.error())};
        }

        ast::Decl_Main* const decl = result.value();
        abstract.insert(abstract.end(), decl);
      } break;

      default:
        ANTON_UNREACHABLE("unhandled node kind");
      }
    }
    return {anton::expected_value, abstract};
  }
} // namespace glang
