#include <glang_sema/sema.hpp>

#include <glang_core/scoped_map.hpp>
#include <glang_sema/diagnostics.hpp>

namespace glang {
  using Symbol_Table = Scoped_Map<anton::String_View, ast::Node*>;

  [[nodiscard]] static anton::Expected<void, Error>
  add_symbol(FE_Context& ctx, Symbol_Table& symtab,
             ast::Node* const generic_node)
  {
    // Get identifier.
    anton::String_View identifier;
    switch(generic_node->node_kind) {
    case ast::Node_Kind::decl_procedure: {
      auto const node = static_cast<ast::Decl_Procedure*>(generic_node);
      identifier = node->identifier.value;
    } break;

    case ast::Node_Kind::variable: {
      auto const node = static_cast<ast::Variable*>(generic_node);
      identifier = node->identifier.value;
    } break;

    case ast::Node_Kind::procedure_parameter: {
      auto const node = static_cast<ast::Procedure_Parameter*>(generic_node);
      identifier = node->identifier.value;
    } break;

    default:
      ANTON_UNREACHABLE("unhandled node kind");
    }
    // Verify the symbol is not already present in the symbol table.
    ast::Node* const* const original_symbol = symtab.find_entry(identifier);
    if(original_symbol != nullptr) {
      return {anton::expected_error,
              err_symbol_redefinition(ctx, *original_symbol, generic_node)};
    }

    symtab.add_entry(identifier, generic_node);
    return anton::expected_value;
  }

#define ANALYSE_EXPRESSION(ctx, symtab, node) \
  {                                           \
    anton::Expected<void, Error> result =     \
      analyse_expression(ctx, symtab, node);  \
    if(!result) {                             \
      return ANTON_MOV(result);               \
    }                                         \
  }

#define ANALYSE_STATEMENT(ctx, symtab, node) \
  {                                          \
    anton::Expected<void, Error> result =    \
      analyse_statement(ctx, symtab, node);  \
    if(!result) {                            \
      return ANTON_MOV(result);              \
    }                                        \
  }

  [[nodiscard]] static anton::Expected<void, Error>
  analyse_expression(FE_Context& ctx, Symbol_Table& symtab,
                     ast::Expr* const generic_node)
  {
    switch(generic_node->node_kind) {
    case ast::Node_Kind::expr_lt_integer: {
      // Nothing.
    } break;

    case ast::Node_Kind::expr_identifier: {
      auto const node = static_cast<ast::Expr_Identifier*>(generic_node);
      ast::Node* const* const symbol = symtab.find_entry(node->value);
      if(symbol == nullptr) {
        return {anton::expected_error, err_symbol_undefined(ctx, node)};
      }

      node->definition = *symbol;
    } break;

    case ast::Node_Kind::expr_index: {
      auto const node = static_cast<ast::Expr_Index*>(generic_node);
      ast::Node* const* const symbol =
        symtab.find_entry(node->identifier.value);
      if(symbol == nullptr) {
        return {anton::expected_error, err_symbol_undefined(ctx, node)};
      }

      node->definition = *symbol;

      ANALYSE_EXPRESSION(ctx, symtab, node->index);
    } break;

    case ast::Node_Kind::expr_binary: {
      auto const node = static_cast<ast::Expr_Binary*>(generic_node);
      ANALYSE_EXPRESSION(ctx, symtab, node->lhs);
      ANALYSE_EXPRESSION(ctx, symtab, node->rhs);
    } break;

    default:
      ANTON_UNREACHABLE("unhandled node kind");
    }
    return anton::expected_value;
  }

  [[nodiscard]] static anton::Expected<void, Error>
  analyse_statement(FE_Context& ctx, Symbol_Table& symtab,
                    ast::Node* const generic_node)
  {
    switch(generic_node->node_kind) {
    case ast::Node_Kind::stmt_assign: {
      auto const node = static_cast<ast::Stmt_Assign*>(generic_node);
      ANALYSE_EXPRESSION(ctx, symtab, node->dst);
      ANALYSE_EXPRESSION(ctx, symtab, node->src);
    } break;

    case ast::Node_Kind::stmt_call: {
      auto const node = static_cast<ast::Stmt_Call*>(generic_node);
      ast::Node* const* const symbol =
        symtab.find_entry(node->identifier.value);
      if(symbol == nullptr) {
        return {anton::expected_error, err_symbol_undefined(ctx, node)};
      }

      if((**symbol).node_kind != ast::Node_Kind::decl_procedure) {
        return {anton::expected_error,
                err_called_symbol_is_not_function(ctx, node, *symbol)};
      }

      node->procedure = static_cast<ast::Decl_Procedure*>(*symbol);

      for(ast::Expr* const arg: node->arguments) {
        ANALYSE_EXPRESSION(ctx, symtab, arg);
      }
    } break;

    case ast::Node_Kind::stmt_if: {
      auto const node = static_cast<ast::Stmt_If*>(generic_node);
      ANALYSE_EXPRESSION(ctx, symtab, node->condition);
      for(ast::Node* stmt: node->then_branch) {
        ANALYSE_STATEMENT(ctx, symtab, stmt);
      }
      for(ast::Node* stmt: node->else_branch) {
        ANALYSE_STATEMENT(ctx, symtab, stmt);
      }
    } break;

    case ast::Node_Kind::stmt_repeat: {
      auto const node = static_cast<ast::Stmt_Repeat*>(generic_node);
      for(ast::Node* stmt: node->stmts) {
        ANALYSE_STATEMENT(ctx, symtab, stmt);
      }
      ANALYSE_EXPRESSION(ctx, symtab, node->condition);
    } break;

    case ast::Node_Kind::stmt_while: {
      auto const node = static_cast<ast::Stmt_While*>(generic_node);
      ANALYSE_EXPRESSION(ctx, symtab, node->condition);
      for(ast::Node* stmt: node->stmts) {
        ANALYSE_STATEMENT(ctx, symtab, stmt);
      }
    } break;

    case ast::Node_Kind::stmt_read: {
      auto const node = static_cast<ast::Stmt_Read*>(generic_node);
      ANALYSE_EXPRESSION(ctx, symtab, node->dst);
    } break;

    case ast::Node_Kind::stmt_write: {
      auto const node = static_cast<ast::Stmt_Write*>(generic_node);
      ANALYSE_EXPRESSION(ctx, symtab, node->src);
    } break;

    default:
      ANTON_UNREACHABLE("unhandled node kind");
    }
    return anton::expected_value;
  }

  [[nodiscard]] static anton::Expected<void, Error>
  analyse_procedure(FE_Context& ctx, Symbol_Table& symtab,
                    ast::Decl_Procedure* const node)
  {
    symtab.push_scope();
    for(ast::Procedure_Parameter* const parameter: node->parameters) {
      anton::Expected<void, Error> result = add_symbol(ctx, symtab, parameter);
      if(!result) {
        return ANTON_MOV(result);
      }
    }

    for(ast::Variable* const variable: node->declarations) {
      anton::Expected<void, Error> result = add_symbol(ctx, symtab, variable);
      if(!result) {
        return ANTON_MOV(result);
      }
    }

    for(ast::Node* const stmt: node->body) {
      ANALYSE_STATEMENT(ctx, symtab, stmt);
    }

    symtab.pop_scope();
    return anton::expected_value;
  }

  [[nodiscard]] static anton::Expected<void, Error>
  analyse_main(FE_Context& ctx, Symbol_Table& symtab,
               ast::Decl_Main* const node)
  {
    symtab.push_scope();
    for(ast::Variable* const variable: node->declarations) {
      anton::Expected<void, Error> result = add_symbol(ctx, symtab, variable);
      if(!result) {
        return ANTON_MOV(result);
      }
    }

    for(ast::Node* const stmt: node->body) {
      ANALYSE_STATEMENT(ctx, symtab, stmt);
    }

    symtab.pop_scope();
    return anton::expected_value;
  }

  anton::Expected<void, Error> run_sema(FE_Context& ctx,
                                        ast::Node_List const nodes)
  {
    Symbol_Table symtab(ctx.allocator);
    // Add global symbols.
    for(ast::Node* const node: nodes) {
      if(node->node_kind == ast::Node_Kind::decl_procedure) {
        anton::Expected<void, Error> result = add_symbol(ctx, symtab, node);
        if(!result) {
          return ANTON_MOV(result);
        }
      }
    }
    // Run analysis.
    for(ast::Node* const generic_node: nodes) {
      switch(generic_node->node_kind) {
      case ast::Node_Kind::decl_main: {
        auto const node = static_cast<ast::Decl_Main*>(generic_node);
        anton::Expected<void, Error> result = analyse_main(ctx, symtab, node);
        if(!result) {
          return ANTON_MOV(result);
        }
      } break;

      case ast::Node_Kind::decl_procedure: {
        auto const node = static_cast<ast::Decl_Procedure*>(generic_node);
        anton::Expected<void, Error> result =
          analyse_procedure(ctx, symtab, node);
        if(!result) {
          return ANTON_MOV(result);
        }
      } break;

      default:
        ANTON_UNREACHABLE("unhandled node kind");
      }
    }
    return anton::expected_value;
  }
} // namespace glang
