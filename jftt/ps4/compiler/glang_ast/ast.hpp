#pragma once

#include <anton/string.hpp>

#include <glang_core/source_info.hpp>

namespace glang::ast {
  template<typename T>
  struct With_Source {
    T value;
    Source_Info source_info;
  };

  using Identifier = With_Source<anton::String_View>;
  struct Lt_Integer;

  enum struct Node_Kind : u8 {
    variable,

    procedure_parameter,

    decl_procedure,
    decl_main,

    expr_identifier,
    expr_binary,
    expr_index,
    expr_lt_integer,

    stmt_call,
    stmt_if,
    stmt_while,
    stmt_repeat,
    stmt_assign,
    stmt_read,
    stmt_write,
  };

  struct Node {
    Source_Info source_info;
    Node_Kind node_kind;

    constexpr Node(Source_Info const& source_info, Node_Kind node_kind)
      : source_info(source_info), node_kind(node_kind)
    {
    }
  };

  using Node_List = anton::Slice<Node* const>;

  struct Variable: public Node {
    Identifier identifier;
    // nullptr if variable is not an array.
    Lt_Integer* size;

    constexpr Variable(Identifier identifier, Lt_Integer* size,
                       Source_Info const& source_info)
      : Node(source_info, Node_Kind::variable), identifier(identifier),
        size(size)
    {
    }
  };

  struct Procedure_Parameter: public Node {
    Identifier identifier;
    bool array;

    constexpr Procedure_Parameter(Identifier identifier, bool array,
                                  Source_Info const& source_info)
      : Node(source_info, Node_Kind::procedure_parameter),
        identifier(identifier), array(array)
    {
    }
  };

  using Procedure_Parameter_List = anton::Slice<Procedure_Parameter* const>;
  using Procedure_Declaration_List = anton::Slice<Variable* const>;

  struct Decl_Procedure: public Node {
    Identifier identifier;
    Procedure_Parameter_List parameters;
    Procedure_Declaration_List declarations;
    Node_List body;

    constexpr Decl_Procedure(Identifier identifier,
                             Procedure_Parameter_List parameters,
                             Procedure_Declaration_List declarations,
                             Node_List body, Source_Info const& source_info)
      : Node(source_info, Node_Kind::decl_procedure), identifier(identifier),
        parameters(parameters), declarations(declarations), body(body)
    {
    }
  };

  struct Decl_Main: public Node {
    Procedure_Declaration_List declarations;
    Node_List body;

    constexpr Decl_Main(Procedure_Declaration_List declarations, Node_List body,
                        Source_Info const& source_info)
      : Node(source_info, Node_Kind::decl_main), declarations(declarations),
        body(body)
    {
    }
  };

  struct Expr: public Node {
    using Node::Node;
  };

  using Expr_List = anton::Slice<Expr* const>;

  struct Expr_Identifier: public Expr {
    anton::String_View value;
    Node* definition = nullptr;

    constexpr Expr_Identifier(anton::String_View value,
                              Source_Info const& source_info)
      : Expr(source_info, Node_Kind::expr_identifier), value(value)
    {
    }
  };

  struct Expr_Index: public Expr {
    Identifier identifier;
    Expr* index;
    Node* definition = nullptr;

    constexpr Expr_Index(Identifier identifier, Expr* index,
                         Source_Info const& source_info)
      : Expr(source_info, Node_Kind::expr_index), identifier(identifier),
        index(index)
    {
    }
  };

  enum struct Expr_Binary_Kind {
    add,
    sub,
    mul,
    div,
    mod,
    eq,
    neq,
    lt,
    gt,
    leq,
    geq,
  };

  struct Expr_Binary: public Expr {
    Expr* lhs;
    Expr* rhs;
    Expr_Binary_Kind kind;

    constexpr Expr_Binary(Expr_Binary_Kind kind, Expr* lhs, Expr* rhs,
                          Source_Info const& source_info)
      : Expr(source_info, Node_Kind::expr_binary), lhs(lhs), rhs(rhs),
        kind(kind)
    {
    }
  };

  struct Lt_Integer: public Expr {
    i64 value;

    constexpr Lt_Integer(i64 value, Source_Info const& source_info)
      : Expr(source_info, Node_Kind::expr_lt_integer), value(value)
    {
    }
  };

  struct Stmt_Call: public Expr {
    Identifier identifier;
    Expr_List arguments;
    Decl_Procedure* procedure = nullptr;

    constexpr Stmt_Call(Identifier identifier, Expr_List arguments,
                        Source_Info const& source_info)
      : Expr(source_info, Node_Kind::stmt_call), identifier(identifier),
        arguments(arguments)
    {
    }
  };

  struct Stmt_If: public Node {
    Expr* condition;
    Node_List then_branch;
    Node_List else_branch;

    constexpr Stmt_If(Expr* condition, Node_List then_branch,
                      Node_List else_branch, Source_Info const& source_info)
      : Node(source_info, Node_Kind::stmt_if), condition(condition),
        then_branch(then_branch), else_branch(else_branch)
    {
    }
  };

  struct Stmt_While: public Node {
    Expr* condition;
    Node_List stmts;

    constexpr Stmt_While(Expr* condition, Node_List stmts,
                         Source_Info const& source_info)
      : Node(source_info, Node_Kind::stmt_while), condition(condition),
        stmts(stmts)
    {
    }
  };

  struct Stmt_Repeat: public Node {
    Expr* condition;
    Node_List stmts;

    constexpr Stmt_Repeat(Expr* condition, Node_List stmts,
                          Source_Info const& source_info)
      : Node(source_info, Node_Kind::stmt_repeat), condition(condition),
        stmts(stmts)
    {
    }
  };

  struct Stmt_Assign: public Node {
    Expr* dst;
    Expr* src;

    constexpr Stmt_Assign(Expr* dst, Expr* src, Source_Info const& source_info)
      : Node(source_info, Node_Kind::stmt_assign), dst(dst), src(src)
    {
    }
  };

  struct Stmt_Read: public Node {
    Expr* dst;

    constexpr Stmt_Read(Expr* dst, Source_Info const& source_info)
      : Node(source_info, Node_Kind::stmt_read), dst(dst)
    {
    }
  };

  struct Stmt_Write: public Node {
    Expr* src;

    constexpr Stmt_Write(Expr* src, Source_Info const& source_info)
      : Node(source_info, Node_Kind::stmt_write), src(src)
    {
    }
  };
} // namespace glang::ast
