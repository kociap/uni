from enum import Enum

class Lookup_Kind(Enum):
  index = 1
  search = 2

class Node_Kind(Enum):
  token = 1
  node = 2

class Syntax_Member:
  # unwrap has effect only when node_kind is node and lookup is search.
  def __init__(self, node_kind, name, lookup, index, optional = False, unwrap = False, offset = False):
    self.node_kind = node_kind
    self.name = name
    self.lookup = lookup
    self.index = index
    self.optional = optional
    self.unwrap = unwrap
    self.offset = offset

syntax_nodes = [
  {
    "syntax_name": "variable",
    "members": [
      Syntax_Member(Node_Kind.token, "identifier", Lookup_Kind.index, 0),
      Syntax_Member(Node_Kind.token, "size", Lookup_Kind.index, 2, optional = True)
    ]
  },
  {
    "syntax_name": "decl_procedure",
    "members": [
      Syntax_Member(Node_Kind.token, "identifier", Lookup_Kind.index, 1),
      Syntax_Member(Node_Kind.node, "parameter_list", Lookup_Kind.index, 2),
      Syntax_Member(Node_Kind.node, "declaration_list", Lookup_Kind.index, 4),
      Syntax_Member(Node_Kind.node, "body", Lookup_Kind.index, 6)
    ]
  },
  {
    "syntax_name": "decl_main",
    "members": [
      Syntax_Member(Node_Kind.node, "declaration_list", Lookup_Kind.index, 2),
      Syntax_Member(Node_Kind.node, "body", Lookup_Kind.index, 4)
    ]
  },
  {
    "syntax_name": "procedure_parameter",
    "members": [
      Syntax_Member(Node_Kind.token, "T", Lookup_Kind.index, 0, optional = True),
      Syntax_Member(Node_Kind.token, "identifier", Lookup_Kind.search, "identifier"),
    ]
  },
  {
    "syntax_name": "expr_binary",
    "members": [
      Syntax_Member(Node_Kind.node, "lhs", Lookup_Kind.index, 0),
      Syntax_Member(Node_Kind.token, "operator", Lookup_Kind.index, 1),
      Syntax_Member(Node_Kind.node, "rhs", Lookup_Kind.index, 2)
    ]
  },
  {
    "syntax_name": "expr_identifier",
    "members": [
      Syntax_Member(Node_Kind.token, "value", Lookup_Kind.index, 0)
    ]
  },
  {
    "syntax_name": "expr_index",
    "members": [
      Syntax_Member(Node_Kind.token, "base", Lookup_Kind.index, 0),
      Syntax_Member(Node_Kind.node, "index", Lookup_Kind.index, 2),
    ]
  },
  {
    "syntax_name": "expr_lt_integer",
    "members": [
      Syntax_Member(Node_Kind.token, "value", Lookup_Kind.index, 0),
    ]
  },
  {
    "syntax_name": "stmt_if",
    "members": [
      Syntax_Member(Node_Kind.node, "condition", Lookup_Kind.index, 1),
      Syntax_Member(Node_Kind.node, "then_branch", Lookup_Kind.index, 3),
      Syntax_Member(Node_Kind.node, "else_branch", Lookup_Kind.index, 5, optional = True),
    ]
  },
  {
    "syntax_name": "stmt_call",
    "members": [
      Syntax_Member(Node_Kind.token, "identifier", Lookup_Kind.index, 0),
      Syntax_Member(Node_Kind.node, "arguments", Lookup_Kind.index, 2)
    ]
  },
  {
    "syntax_name": "stmt_while",
    "members": [
      Syntax_Member(Node_Kind.node, "condition", Lookup_Kind.index, 1),
      Syntax_Member(Node_Kind.node, "statements", Lookup_Kind.index, 3),
    ]
  },
  {
    "syntax_name": "stmt_repeat",
    "members": [
      Syntax_Member(Node_Kind.node, "statements", Lookup_Kind.index, 1),
      Syntax_Member(Node_Kind.node, "condition", Lookup_Kind.index, 3),
    ]
  },
  {
    "syntax_name": "stmt_read",
    "members": [
      Syntax_Member(Node_Kind.node, "dst", Lookup_Kind.index, 1)
    ]
  },
  {
    "syntax_name": "stmt_write",
    "members": [
      Syntax_Member(Node_Kind.node, "src", Lookup_Kind.index, 1)
    ]
  },
  {
    "syntax_name": "stmt_assign",
    "members": [
      Syntax_Member(Node_Kind.node, "dst", Lookup_Kind.index, 0),
      Syntax_Member(Node_Kind.node, "src", Lookup_Kind.index, 2),
    ]
  },
]
