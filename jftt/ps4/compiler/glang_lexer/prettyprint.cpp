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