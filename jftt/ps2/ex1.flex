%{
int line_number = 1;
int word_count = 0;
%}

WS  [\t ]
NWS [^\t\n ]

%%

{NWS}+ { printf("%s", yytext); word_count += 1; }

^{WS}*\n { line_number += 1; }

^{WS}+ { }

{WS}+$ { }

{WS}+ { printf(" "); }

\n { printf("\n"); line_number += 1; }

<<EOF>> {
  printf("\nlines: %d; words: %d\n", line_number, word_count);
  return 0;
}

%%

int yywrap()
{
  return 1;
}

int main(int argc, char **argv)
{
  if ( argc > 1 ) {
    yyin = fopen( argv[1], "r" );
  } else {
    yyin = stdin;
  }

  yylex();
}
