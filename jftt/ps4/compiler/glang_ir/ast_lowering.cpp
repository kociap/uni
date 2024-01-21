#include "glang_ir/ir.hpp"
#include <glang_ir/ast_lowering.hpp>

#include <anton/flat_hash_map.hpp>
#include <anton/intrusive_list.hpp>

#include <glang_core/memory.hpp>

namespace glang {
  struct Lowering_Context {
  public:
    ir::Context* ir_ctx;
    Allocator* allocator;

  private:
    anton::Flat_Hash_Map<ast::Decl_Procedure const*, ir::Function*> functions;
    anton::Flat_Hash_Map<ast::Node const*, ir::Instr_variable*> definitions;

  public:
    Lowering_Context(ir::Context& ir_ctx)
      : ir_ctx(&ir_ctx), allocator(ir_ctx.allocator), functions(allocator),
        definitions(allocator)
    {
    }

    [[nodiscard]] i64 get_next_id()
    {
      return ir_ctx->get_next_id();
    }

    [[nodiscard]] i64 get_next_block_label()
    {
      return ir_ctx->get_next_block_label();
    }

    void add_function(ast::Decl_Procedure const* procedure,
                      ir::Function* function)
    {
      functions.emplace(procedure, function);
    }

    ir::Function* get_function(ast::Decl_Procedure const* procedure)
    {
      auto iter = functions.find(procedure);
      if(iter != functions.end()) {
        return iter->value;
      } else {
        return nullptr;
      }
    }

    void add_definition(ast::Node const* node, ir::Instr_variable* variable)
    {
      definitions.emplace(node, variable);
    }

    ir::Instr_variable* get_definition(ast::Node const* node)
    {
      auto iter = definitions.find(node);
      if(iter != definitions.end()) {
        return iter->value;
      } else {
        return nullptr;
      }
    }
  };

  struct Builder {
  private:
    ir::Block* insert_block = nullptr;

  public:
    ir::Function* function;

    Builder(ir::Function* function): function(function) {}

    void set_insert_block(ir::Block* const block)
    {
      insert_block = block;
    }

    void insert(ir::Instr* const node)
    {
      ANTON_ASSERT(insert_block != nullptr, "insert_block has not been set");
      insert_block->insert(node);
    }
  };

  [[nodiscard]] static ir::Instr*
  lower_expression(Lowering_Context& ctx, Builder& builder,
                   ast::Expr const* const generic_node);

  [[nodiscard]] static ir::Instr*
  forward_parameter(Lowering_Context& ctx, ast::Expr const* const generic_node)
  {
    switch(generic_node->node_kind) {
    case ast::Node_Kind::expr_identifier: {
      auto const node = static_cast<ast::Expr_Identifier const*>(generic_node);
      ir::Instr_variable* const definition =
        ctx.get_definition(node->definition);
      ANTON_ASSERT(definition != nullptr, "node has no IR definition");
      return definition;
    } break;

    case ast::Node_Kind::expr_index:
      ANTON_UNREACHABLE("expr_index cannot be forwarded");
    case ast::Node_Kind::expr_lt_integer:
      ANTON_UNREACHABLE("expr_lt_integer cannot be forwarded");
    case ast::Node_Kind::expr_binary:
      ANTON_UNREACHABLE("expr_binary cannot be forwarded");

    default:
      ANTON_UNREACHABLE("unhandled node kind");
    }
  }

  [[nodiscard]] static ir::Instr*
  get_address(Lowering_Context& ctx, Builder& builder,
              ast::Expr const* const generic_node)
  {
    switch(generic_node->node_kind) {
    case ast::Node_Kind::expr_identifier: {
      auto const node = static_cast<ast::Expr_Identifier const*>(generic_node);
      ir::Instr_variable* const definition =
        ctx.get_definition(node->definition);
      ANTON_ASSERT(definition != nullptr, "node has no IR definition");
      if(definition->kind == ir::Variable_Kind::local) {
        return definition;
      } else {
        // Parameters are locations storing an address. We have to load a
        // parameter to obtain the address it is pointing to.
        ir::Instr* const address =
          ir::make_instr_load(*ctx.ir_ctx, node->source_info, definition);
        builder.insert(address);
        return address;
      }
    } break;

    case ast::Node_Kind::expr_index: {
      auto const node = static_cast<ast::Expr_Index const*>(generic_node);
      ir::Instr_variable* const definition =
        ctx.get_definition(node->definition);
      ANTON_ASSERT(definition != nullptr, "node has no IR definition");
      ir::Instr* index = lower_expression(ctx, builder, node->index);
      if(definition->kind == ir::Variable_Kind::local) {
        ir::Instr* const address = ir::make_instr_getelementptr(
          *ctx.ir_ctx, node->source_info, definition, index);
        builder.insert(address);
        return address;
      } else {
        ir::Instr* const address = ir::make_instr_getelementptr_indirect(
          *ctx.ir_ctx, node->source_info, definition, index);
        builder.insert(address);
        return address;
      }
    } break;

    case ast::Node_Kind::expr_lt_integer:
      ANTON_UNREACHABLE("expr_lt_integer is not addressable");
    case ast::Node_Kind::expr_binary:
      ANTON_UNREACHABLE("expr_binary is not addressable");

    default:
      ANTON_UNREACHABLE("unhandled node kind");
    }
  }

  [[nodiscard]] static ir::ALU_Opcode
  binary_to_opcode(ast::Expr_Binary_Kind const kind)
  {
    switch(kind) {
    case ast::Expr_Binary_Kind::add:
      return ir::ALU_Opcode::add;
    case ast::Expr_Binary_Kind::sub:
      return ir::ALU_Opcode::sub;
    case ast::Expr_Binary_Kind::mul:
      return ir::ALU_Opcode::mul;
    case ast::Expr_Binary_Kind::div:
      return ir::ALU_Opcode::div;
    case ast::Expr_Binary_Kind::mod:
      return ir::ALU_Opcode::mod;
    case ast::Expr_Binary_Kind::eq:
      return ir::ALU_Opcode::eq;
    case ast::Expr_Binary_Kind::neq:
      return ir::ALU_Opcode::neq;
    case ast::Expr_Binary_Kind::lt:
      return ir::ALU_Opcode::lt;
    case ast::Expr_Binary_Kind::gt:
      return ir::ALU_Opcode::gt;
    case ast::Expr_Binary_Kind::leq:
      return ir::ALU_Opcode::leq;
    case ast::Expr_Binary_Kind::geq:
      return ir::ALU_Opcode::geq;
    }
  }

  ir::Instr* lower_expression(Lowering_Context& ctx, Builder& builder,
                              ast::Expr const* const generic_node)
  {
    switch(generic_node->node_kind) {
    case ast::Node_Kind::expr_lt_integer: {
      auto const node = static_cast<ast::Lt_Integer const*>(generic_node);
      ir::Instr* const instr =
        ir::make_instr_constant(*ctx.ir_ctx, node->source_info, node->value);
      builder.insert(instr);
      return instr;
    } break;

    case ast::Node_Kind::expr_identifier: {
      auto const node = static_cast<ast::Expr_Identifier const*>(generic_node);
      ir::Instr_variable* const definition =
        ctx.get_definition(node->definition);
      ANTON_ASSERT(definition != nullptr, "node has no IR definition");
      if(definition->kind == ir::Variable_Kind::local) {
        ir::Instr* const instr =
          ir::make_instr_load(*ctx.ir_ctx, node->source_info, definition);
        builder.insert(instr);
        return instr;
      } else {
        // Parameters are locations storing an address. We have to load a
        // parameter to obtain the address it is pointing to.
        ir::Instr* const instr = ir::make_instr_load_indirect(
          *ctx.ir_ctx, node->source_info, definition);
        builder.insert(instr);
        return instr;
      }
    } break;

    case ast::Node_Kind::expr_index: {
      auto const node = static_cast<ast::Expr_Index const*>(generic_node);
      ir::Instr_variable* const definition =
        ctx.get_definition(node->definition);
      ANTON_ASSERT(definition != nullptr, "node has no IR definition");
      ir::Instr* const index = lower_expression(ctx, builder, node->index);
      if(definition->kind == ir::Variable_Kind::local) {
        ir::Instr* const address = ir::make_instr_getelementptr(
          *ctx.ir_ctx, node->source_info, definition, index);
        builder.insert(address);
        ir::Instr* const instr =
          ir::make_instr_load(*ctx.ir_ctx, node->source_info, address);
        builder.insert(instr);
        return instr;
      } else {
        ir::Instr* const address = ir::make_instr_getelementptr_indirect(
          *ctx.ir_ctx, node->source_info, definition, index);
        builder.insert(address);
        ir::Instr* const instr =
          ir::make_instr_load(*ctx.ir_ctx, node->source_info, address);
        builder.insert(instr);
        return instr;
      }
    } break;

    case ast::Node_Kind::expr_binary: {
      auto const node = static_cast<ast::Expr_Binary const*>(generic_node);
      ir::Instr* const rhs = lower_expression(ctx, builder, node->rhs);
      ir::Instr* const lhs = lower_expression(ctx, builder, node->lhs);
      ir::ALU_Opcode const opcode = binary_to_opcode(node->kind);
      ir::Instr_ALU* const instr =
        ir::make_instr_alu(*ctx.ir_ctx, node->source_info, opcode, lhs, rhs);
      builder.insert(instr);
      return instr;
    } break;

    default:
      ANTON_UNREACHABLE("unhandled node kind");
    }
  }

  static void lower_statement(Lowering_Context& ctx, Builder& builder,
                              ast::Node const* node);

  static void lower_statement_list(Lowering_Context& ctx, Builder& builder,
                                   ast::Node_List const statement_list)
  {
    for(ast::Node const* statement: statement_list) {
      lower_statement(ctx, builder, statement);
    }
  }

  void lower_statement(Lowering_Context& ctx, Builder& builder,
                       ast::Node const* const node)
  {
    switch(node->node_kind) {
    case ast::Node_Kind::stmt_assign: {
      auto const stmt = static_cast<ast::Stmt_Assign const*>(node);
      ir::Instr* const dst = get_address(ctx, builder, stmt->dst);
      ir::Instr* const src = lower_expression(ctx, builder, stmt->src);
      ir::Instr_store* const instr =
        ir::make_instr_store(*ctx.ir_ctx, node->source_info, dst, src);
      builder.insert(instr);
    } break;

    case ast::Node_Kind::stmt_if: {
      auto const stmt = static_cast<ast::Stmt_If const*>(node);
      ir::Block* const then_block = make_block(*ctx.ir_ctx, builder.function);
      ir::Block* const else_block = make_block(*ctx.ir_ctx, builder.function);
      ir::Block* const merge_block = make_block(*ctx.ir_ctx, builder.function);
      ir::Instr* const condition =
        lower_expression(ctx, builder, stmt->condition);
      ir::Instr_brcond* const brcond = ir::make_instr_brcond(
        *ctx.ir_ctx, node->source_info, ir::Brcond_Kind::jgt, condition,
        then_block, else_block);
      builder.insert(brcond);
      builder.set_insert_block(then_block);
      lower_statement_list(ctx, builder, stmt->then_branch);
      ir::Instr_branch* const branch_then_to_merge =
        ir::make_instr_branch(*ctx.ir_ctx, node->source_info, merge_block);
      builder.insert(branch_then_to_merge);
      builder.set_insert_block(else_block);
      lower_statement_list(ctx, builder, stmt->else_branch);
      ir::Instr_branch* const branch_else_to_merge =
        ir::make_instr_branch(*ctx.ir_ctx, node->source_info, merge_block);
      builder.insert(branch_else_to_merge);
      builder.set_insert_block(merge_block);
    } break;

    case ast::Node_Kind::stmt_repeat: {
      auto const stmt = static_cast<ast::Stmt_Repeat const*>(node);
      ir::Block* const entry_block = make_block(*ctx.ir_ctx, builder.function);
      ir::Block* const merge_block = make_block(*ctx.ir_ctx, builder.function);
      ir::Instr_branch* const branch_entry =
        ir::make_instr_branch(*ctx.ir_ctx, node->source_info, entry_block);
      builder.insert(branch_entry);
      builder.set_insert_block(entry_block);
      lower_statement_list(ctx, builder, stmt->stmts);
      ir::Instr* const condition =
        lower_expression(ctx, builder, stmt->condition);
      ir::Instr_brcond* const brcond = ir::make_instr_brcond(
        *ctx.ir_ctx, node->source_info, ir::Brcond_Kind::jgt, condition,
        entry_block, merge_block);
      builder.insert(brcond);
      builder.set_insert_block(merge_block);
    } break;

    case ast::Node_Kind::stmt_while: {
      auto const stmt = static_cast<ast::Stmt_While const*>(node);
      ir::Block* const entry_block = make_block(*ctx.ir_ctx, builder.function);
      ir::Block* const main_block = make_block(*ctx.ir_ctx, builder.function);
      ir::Block* const merge_block = make_block(*ctx.ir_ctx, builder.function);
      ir::Instr_branch* const branch_entry =
        ir::make_instr_branch(*ctx.ir_ctx, node->source_info, entry_block);
      builder.insert(branch_entry);
      builder.set_insert_block(entry_block);
      ir::Instr* const condition =
        lower_expression(ctx, builder, stmt->condition);
      ir::Instr_brcond* const brcond = ir::make_instr_brcond(
        *ctx.ir_ctx, node->source_info, ir::Brcond_Kind::jgt, condition,
        main_block, merge_block);
      builder.insert(brcond);
      builder.set_insert_block(main_block);
      lower_statement_list(ctx, builder, stmt->stmts);
      ir::Instr_branch* const branch_main_to_entry =
        ir::make_instr_branch(*ctx.ir_ctx, node->source_info, entry_block);
      builder.insert(branch_main_to_entry);
      builder.set_insert_block(merge_block);
    } break;

    case ast::Node_Kind::stmt_call: {
      auto const stmt = static_cast<ast::Stmt_Call const*>(node);
      ir::Function* const function = ctx.get_function(stmt->procedure);
      ANTON_ASSERT(function != nullptr, "node has no IR function");
      ir::Instr_call* const call =
        ir::make_instr_call(*ctx.ir_ctx, stmt->source_info, function);
      for(ast::Expr const* const arg: stmt->arguments) {
        ir::Instr* const arg_instr = forward_parameter(ctx, arg);
        call->arguments.push_back(arg_instr);
        arg_instr->add_referrer(call);
      }
      builder.insert(call);
    } break;

    case ast::Node_Kind::stmt_read: {
      auto const stmt = static_cast<ast::Stmt_Read const*>(node);
      ir::Instr* const dst = get_address(ctx, builder, stmt->dst);
      ir::Instr_read* const instr =
        ir::make_instr_read(*ctx.ir_ctx, stmt->source_info);
      ir::Instr* const store =
        ir::make_instr_store(*ctx.ir_ctx, stmt->source_info, dst, instr);
      builder.insert(instr);
      builder.insert(store);
    } break;

    case ast::Node_Kind::stmt_write: {
      auto const stmt = static_cast<ast::Stmt_Write const*>(node);
      ir::Instr* const src = lower_expression(ctx, builder, stmt->src);
      ir::Instr_write* const instr =
        ir::make_instr_write(*ctx.ir_ctx, node->source_info, src);
      builder.insert(instr);
    } break;

    default:
      ANTON_UNREACHABLE("unhandled node kind");
    }
  }

  [[nodiscard]] static ir::Function*
  lower_function(Lowering_Context& ctx,
                 ast::Decl_Procedure const* const procedure)
  {
    ir::Function* const function = ctx.get_function(procedure);
    for(ast::Procedure_Parameter const* parameter: procedure->parameters) {
      ir::Instr_variable* const instr = ir::make_instr_variable(
        *ctx.ir_ctx, parameter->source_info, ir::Variable_Kind::param,
        parameter->identifier.value);
      function->parameters.push_back(instr);
      ctx.add_definition(parameter, instr);
    }
    for(ast::Variable* const local: procedure->declarations) {
      ir::Instr_variable* const instr = ir::make_instr_variable(
        *ctx.ir_ctx, local->source_info, ir::Variable_Kind::local,
        local->identifier.value);
      if(local->size) {
        instr->size = local->size->value;
      }
      function->locals.push_back(instr);
      ctx.add_definition(local, instr);
    }
    Builder builder(function);
    ir::Block* const entry_block = make_block(*ctx.ir_ctx, builder.function);
    builder.set_insert_block(entry_block);
    lower_statement_list(ctx, builder, procedure->body);
    ir::Instr* return_instr =
      ir::make_instr_ret(*ctx.ir_ctx, function->source_info);
    builder.insert(return_instr);
    function->entry_block = entry_block;
    return function;
  }

  [[nodiscard]] static ir::Function*
  lower_entry(Lowering_Context& ctx, ast::Decl_Main const* const main_procedure)
  {
    ir::Function* const function = allocate<ir::Function>(
      ctx.allocator, ctx.allocator, anton::String(ctx.allocator),
      main_procedure->source_info);
    function->entry = true;
    for(ast::Variable* const local: main_procedure->declarations) {
      ir::Instr_variable* const instr = ir::make_instr_variable(
        *ctx.ir_ctx, local->source_info, ir::Variable_Kind::local,
        local->identifier.value);
      if(local->size) {
        instr->size = local->size->value;
      }
      function->locals.push_back(instr);
      ctx.add_definition(local, instr);
    }
    Builder builder(function);
    ir::Block* const entry_block =
      ir::make_block(*ctx.ir_ctx, builder.function);
    builder.set_insert_block(entry_block);
    lower_statement_list(ctx, builder, main_procedure->body);
    ir::Instr* const end =
      ir::make_instr_end(*ctx.ir_ctx, function->source_info);
    builder.insert(end);
    function->entry_block = entry_block;
    return function;
  }

  [[nodiscard]] IList<ir::Function> lower_ast_to_ir(ir::Context& ir_ctx,
                                                    ast::Node_List ast_nodes)
  {
    Lowering_Context ctx(ir_ctx);
    // Construct ir::Functions to populate the functions map.
    for(ast::Node const* const node: ast_nodes) {
      if(node->node_kind != ast::Node_Kind::decl_procedure) {
        continue;
      }

      auto const decl_procedure = static_cast<ast::Decl_Procedure const*>(node);
      ir::Function* const function = allocate<ir::Function>(
        ctx.allocator, ctx.allocator,
        anton::String(decl_procedure->identifier.value, ctx.allocator),
        node->source_info);
      ctx.add_function(decl_procedure, function);
    }
    // Lower functions and modules.
    IList<ir::Function> functions;
    for(ast::Node const* const node: ast_nodes) {
      switch(node->node_kind) {
      case ast::Node_Kind::decl_procedure: {
        auto const decl_procedure =
          static_cast<ast::Decl_Procedure const*>(node);
        ir::Function* const function = lower_function(ctx, decl_procedure);
        functions.insert_back(function);
      } break;

      case ast::Node_Kind::decl_main: {
        auto const decl_main = static_cast<ast::Decl_Main const*>(node);
        ir::Function* const entry = lower_entry(ctx, decl_main);
        functions.insert_back(entry);
      } break;

      default:
        ANTON_UNREACHABLE("unhandled node kind");
      }
    }
    return functions;
  }
} // namespace glang
