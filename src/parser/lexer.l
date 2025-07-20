%{
#include "parser.tab.hpp"  // Token declarations from Bison
#include <string>

extern int yylineno;
extern char* yytext;

void yyerror(const char* s);
%}

%option noyywrap

IDENT     [a-zA-Z_][a-zA-Z0-9_]*
NUMBER    [0-9]+
WS        [ \t\r\n]+

%%

"CREATE"         { return CREATE; }
"TABLE"          { return TABLE; }
"PRIMARY"        { return PRIMARY; }
"KEY"            { return KEY; }
"NUMBER"         { return NUMBER_TYPE; }
{IDENT}          { yylval.str = new std::string(yytext); return IDENTIFIER; }
"("              { return LPAREN; }
")"              { return RPAREN; }
";"              { return SEMICOLON; }
{WS}             { /* skip */ }
.                { yyerror("Unknown character"); }

%%
