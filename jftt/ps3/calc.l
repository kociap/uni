%option noyywrap
%{
#include "calc.tab.h"

int yylex();
%}

%x IN_COMMENT

%%

\#(.|\\\n)*   { return COMMENT; };

\\\n  { };

\+            { return PLUS; };
\-            { return MINUS; };
\*            { return ASTERISK; };
\/            { return RSLASH; };
%             { return PERCENT; };
\^            { return HAT; };
\(            { return LPAREN; };
\)            { return RPAREN; };

[[:digit:]]+  { int d; sscanf(yytext, "%d", &d); yylval = d; return INTEGER; };

\n            { return NEWLINE; };
[[:blank:]]   { };
.             { return ERROR; };

%%