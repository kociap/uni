%{
int emit_docs = 0;
%}

%x DOC_LINE_COMMENT
%x DOC_BLOCK_COMMENT
%x BLOCK_COMMENT
%x LINE_COMMENT
%x STRING
%%

<INITIAL>"\"" { printf("\""); BEGIN(STRING); }
<STRING>"\"" { printf("\""); BEGIN(INITIAL); }

<INITIAL>"/**"|"/*!" { BEGIN(DOC_BLOCK_COMMENT); }
<INITIAL>"/*"|("/\\"\n"*") { BEGIN(BLOCK_COMMENT); }

<DOC_BLOCK_COMMENT,BLOCK_COMMENT>"*/" { BEGIN(INITIAL); }

<INITIAL>"///"|"//!" { BEGIN(DOC_LINE_COMMENT); }
<INITIAL>"//"|("/\\"\n"/") { BEGIN(LINE_COMMENT); }
<DOC_LINE_COMMENT,LINE_COMMENT>"\\"\n { }
<DOC_LINE_COMMENT,LINE_COMMENT>\n { BEGIN(INITIAL); }

<DOC_BLOCK_COMMENT>. {}
<BLOCK_COMMENT>. {}
<DOC_LINE_COMMENT>. {}
<LINE_COMMENT>. {}

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
    if(argv[0] == "-d") {
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
