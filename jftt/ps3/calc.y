%{
#define YYSTYPE int

#include <stdio.h>
#include <calc.h>

int yylex();
int yyerror(char*);

int error = 0;
int hard_error = 0;
%}

%token COMMENT
%token PLUS
%token ASTERISK
%token RSLASH
%token PERCENT
%token HAT
%token LPAREN
%token RPAREN
%token MINUS
%token INTEGER
%token NEWLINE
%token ERROR

%left PLUS MINUS
%left ASTERISK RSLASH PERCENT
%right HAT
%precedence UNARY_MINUS

%%

INPUT:
    %empty { printf(">"); }
  | INPUT STMT NEWLINE { printf(">"); }
  | INPUT NEWLINE { printf(">"); }
;

STMT:
    EXPR  { if(hard_error == 0) {
              printf("%s\n", get_rpn());
              if(error == 0) {
                printf("%d\n", $1);
              }
            }
            clear_rpn();
            hard_error = 0;
            error = 0;
          }
  | COMMENT
;

TERM:
    INTEGER                         { $$ = normalise($1, group_p);
                                      append_rpn_value($$);
                                    }
  | MINUS INTEGER %prec UNARY_MINUS { $$ = unary_minus($2, group_p);
                                      append_rpn_value($$);
                                    }
;

EXPR:
    EXPR PLUS EXPR          { append_rpn_operator('+'); $$ = add($1, $3, group_p); }
  | EXPR MINUS EXPR         { append_rpn_operator('-'); $$ = subtract($1, $3, group_p); }
  | EXPR ASTERISK EXPR      { append_rpn_operator('*'); $$ = multiply($1, $3, group_p); }
  | EXPR RSLASH EXPR        { append_rpn_operator('/');
                              if($3 == 0) {
                                yyerror("division by 0");
                                error = 1;
                              };
                              $$ = divide_unchecked($1, $3, group_p);
                            }
  | EXPR PERCENT EXPR       { append_rpn_operator('%');
                              if($3 == 0) {
                                yyerror("modulo by 0");
                                error = 1;
                              };
                              $$ = mod_unchecked($1, $3, group_p);
                            }
  | EXPR HAT EXPONENT_EXPR  { append_rpn_operator('^'); $$ = power($1, $3, group_p); }
  | LPAREN EXPR RPAREN      { $$ = $2; }
  | TERM
;

EXPONENT_TERM: INTEGER              { $$ = normalise($1, ring_p);
                                      append_rpn_value($$);
                                    }
  | MINUS INTEGER %prec UNARY_MINUS { $$ = unary_minus($2, ring_p);
                                      append_rpn_value($$);
                                    }
;

EXPONENT_EXPR:
    EXPONENT_EXPR PLUS EXPONENT_EXPR      { append_rpn_operator('+'); $$ = add($1, $3, group_p); }
  | EXPONENT_EXPR MINUS EXPONENT_EXPR     { append_rpn_operator('-'); $$ = subtract($1, $3, group_p); }
  | EXPONENT_EXPR ASTERISK EXPONENT_EXPR  { append_rpn_operator('*'); $$ = multiply($1, $3, group_p); }
  | EXPONENT_EXPR RSLASH EXPONENT_EXPR    { append_rpn_operator('/');
                                            if($3 == 0) {
                                              yyerror("division by 0");
                                              error = 1;
                                            };
                                            $$ = divide_unchecked($1, $3, group_p);
                                          }
  | EXPONENT_EXPR PERCENT EXPONENT_EXPR   { append_rpn_operator('%');
                                            if($3 == 0) {
                                              yyerror("modulo by 0");
                                              error = 1;
                                            };
                                            $$ = mod_unchecked($1, $3, group_p);
                                          }
  | EXPONENT_EXPR HAT EXPONENT_EXPR       {
                                            yyerror("chaining of ^ disallowed");
                                            hard_error = 1;
                                          }
  | LPAREN EXPONENT_EXPR RPAREN           { $$ = $2; }
  | EXPONENT_TERM
;

%%

int yyerror(char *s)
{
  if(hard_error == 0 && error == 0) {
    printf("error: %s\n",s);
  }
  return 0;
}

int main()
{
  yyparse();
  return 0;
}
