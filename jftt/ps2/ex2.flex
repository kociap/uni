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

"<![CDATA["(.|\n)*"]]>" { printf("*2nd rule*%s", yytext); }
"<?"(.|\n)*"?>" { printf("*6th rule*%s", yytext); }

"<"{NAME}({WS}{ATTRIBUTE})*{WS}?"/>" { printf("*3rd rule*%s", yytext); }
"<"{NAME}({WS}{ATTRIBUTE})*{WS}?">" { printf("*3rd rule*%s", yytext); }
"</"{NAME}{WS}?">" {printf("*4th rule*%s", yytext); }

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
