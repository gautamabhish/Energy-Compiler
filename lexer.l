%option noyywrap
%{
/* Include AST header BEFORE parser.tab.h so generated header types are known. */
#include "ast.hpp"
#include "parser.tab.h"   /* bison-generated header (tokens & yylval) */
#include <cstring>
%}

%%
"model"        { return MODEL; }
"tensor"       { return TENSOR; }
"print"        { return PRINT; }
"matmul"       { return MATMUL; }
"conv2d"       { return CONV2D; }
"relu"         { return RELU; }
"softmax"      { return SOFTMAX; }

"{"            { return LBRACE; }
"}"            { return RBRACE; }
"["            { return LBRACKET; }
"]"            { return RBRACKET; }
"("            { return LPAREN; }
")"            { return RPAREN; }
","            { return COMMA; }
"="            { return ASSIGN; }
"+"            { return PLUS; }

[0-9]+         { yylval.num = atoi(yytext); return NUM; }
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.str = strdup(yytext); return ID; }

[ \t\r\n]+     ;   /* ignore whitespace */
.              { printf("Unknown char: %s\n", yytext); }

%%
