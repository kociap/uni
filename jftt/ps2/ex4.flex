%{
#include <stdlib.h>

void push_arg(long value);
void do_op(char const* op);
char const* get_error(void);
long get_result(void);
void clear_state(void);
%}

INT "-"?[0-9]+
OP "+"|"-"|"*"|"/"|"^"|"%"

%%

{INT} { push_arg(atol(yytext)); }
{OP} { do_op(yytext); }
\n {
  char const* const error = get_error();
  if(error != NULL) {
    printf("%s\n", error);
  } else {
    long const result = get_result();
    printf("= %ld\n", result);
  }
  clear_state();
}

. {}

%%

int yywrap()
{
  return 1;
}

int main(int argc, char **argv)
{
  yylex();
}
