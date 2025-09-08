%{
#include "parser.hpp"

#include <string>
#include <iostream>
#include <cstdio>

extern int yylineno;
extern char* yytext;

YYSTYPE* yylval;
YYLTYPE* yylloc;

int yycolumn = 1;

#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line = yylineno; \
    yylloc->first_column = yycolumn; \
    yylloc->last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}

%option yylineno
%option noyywrap

IDENT     [a-zA-Z_][a-zA-Z0-9_]*
NUMBER    [0-9]+
WS        [ \t\r\n]+

%%

"CREATE"         { YY_USER_ACTION return CREATE; }
"TABLE"          { YY_USER_ACTION return TABLE; }
"PRIMARY"        { YY_USER_ACTION return PRIMARY; }
"KEY"            { YY_USER_ACTION return KEY; }
"NUMBER"         { YY_USER_ACTION return NUMBER_TYPE; }
{IDENT}          { YY_USER_ACTION yylval->str = new std::string(yytext); return IDENTIFIER; }
"("              { YY_USER_ACTION return LPAREN; }
")"              { YY_USER_ACTION return RPAREN; }
";"              { YY_USER_ACTION return SEMICOLON; }
\n               { yylineno++; yycolumn = 1; yylloc->first_column = 1; }
{WS}             { /* skip */ }
.                { std::fprintf(stderr, "Unexpected token: %s\n", yytext); return *yytext; }

<<EOF>>          { return 0; }

%%

int yylex(YYSTYPE* lval, YYLTYPE* lloc) {
    yylval = lval;
    yylloc = lloc;
    return yylex(); // calls the real Flex scanner
}
