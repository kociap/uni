%{
#include <string.h>
int emit_docs = 0;
%}

%x DOC_LINE_COMMENT
%x DOC_BLOCK_COMMENT
%x BLOCK_COMMENT
%x LINE_COMMENT
%x STRING
%x INCLUDE_PATH
%s INCLUDE
%s MATCHED_SLASH
%%

\\\n {}

<INITIAL>"#include" { printf("#include"); BEGIN(INCLUDE); }
<INCLUDE>"\n" { printf("\n"); BEGIN(INITIAL); }
<INCLUDE>"<" { printf("<"); BEGIN(INCLUDE_PATH); }
<INCLUDE_PATH>">" { printf(">"); BEGIN(INCLUDE); }

<INITIAL>"\"" { printf("\""); BEGIN(STRING); }
<STRING>"\"" { printf("\""); BEGIN(INITIAL); }

<INITIAL>"/" { BEGIN(MATCHED_SLASH); }

<MATCHED_SLASH>"**"|"*!" { if(emit_docs) { printf("/%s", yytext); } BEGIN(DOC_BLOCK_COMMENT); }
<MATCHED_SLASH>"//"|"/!" { if(emit_docs) { printf("/%s", yytext); } BEGIN(DOC_LINE_COMMENT); }
<MATCHED_SLASH>"*" { BEGIN(BLOCK_COMMENT); }
<MATCHED_SLASH>"/" { BEGIN(LINE_COMMENT); }
<MATCHED_SLASH>. { printf("/%s", yytext); BEGIN(INITIAL); }

<DOC_BLOCK_COMMENT>"*/" { if(emit_docs) { printf("*/"); }  BEGIN(INITIAL); }
<BLOCK_COMMENT>"*/" { BEGIN(INITIAL); }

<DOC_LINE_COMMENT,LINE_COMMENT>\n { printf("\n"); BEGIN(INITIAL); }
<DOC_BLOCK_COMMENT,BLOCK_COMMENT>\n {}

<DOC_BLOCK_COMMENT,DOC_LINE_COMMENT>. { if(emit_docs) { printf("%s", yytext); } }
<BLOCK_COMMENT,LINE_COMMENT>. {}

%%

int yywrap()
{
  return 1;
}

int main(int argc, char **argv)
{
  argc -= 1;
  argv += 1;

  if(argc > 0) {
    if(strcmp(argv[0], "-d") == 0) {
      emit_docs = 1;
      argc -= 1;
      argv += 1;
    }
  }

  if(argc > 0) {
    yyin = fopen( argv[0], "r" );
  } else {
    yyin = stdin;
  }

  yylex();
}
