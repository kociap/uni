%{
%}

WS [ \t\n\r]+
CHAR [^-]
NAMESTARTCHAR [^-!?/]
NAME {NAMESTARTCHAR}{CHAR}+
VALUE "\""[^"]*"\""
ATTRIBUTE {NAME}{WS}?"="{WS}?{VALUE}

%%

"<!--"({CHAR}|("-"{CHAR}))*"-->" { }

"<![CDATA["(.|\n)*"]]>" { printf("%s", yytext); }
"<?"(.|\n)*"?>" { printf("%s", yytext); }

"<"{NAME}({WS}{ATTRIBUTE})*{WS}?"/>" { printf("%s", yytext); }
"<"{NAME}({WS}{ATTRIBUTE})*{WS}?">" { printf("%s", yytext); }
"</"{NAME}{WS}?">" {printf("%s", yytext); }

"<!"[A-Z][^>]*">" { printf("%s", yytext); }

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
