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
NUMBER    [+-]?([0-9]+)(\.[0-9]+)?(e?[0-9]+(\.[0-9]+)?)?
WS        [ \t\r\n]+
STRING    \'([^\\\n]|(\\.))*\'

%%

"CREATE"         { YY_USER_ACTION return KW_CREATE; }
"INSERT"         { YY_USER_ACTION return KW_INSERT; }
"INTO"           { YY_USER_ACTION return KW_INTO; }
"FALSE"          { YY_USER_ACTION return KW_FALSE; }
"TRUE"           { YY_USER_ACTION return KW_TRUE; }
"VALUES"         { YY_USER_ACTION return KW_VALUES; }
"DROP"           { YY_USER_ACTION return KW_DROP; }
"UNTYPED"        { YY_USER_ACTION return KW_UNTYPED; }
"TABLE"          { YY_USER_ACTION return KW_TABLE; }
"PRIMARY"        { YY_USER_ACTION return KW_PRIMARY; }
"KEY"            { YY_USER_ACTION return KW_KEY; }
"NUMBER"         { YY_USER_ACTION return KW_NUMBER_T; }
"NULL"           { YY_USER_ACTION return KW_NULL; }
"NOT"            { YY_USER_ACTION return KW_NOT; }
"UNIQUE"         { YY_USER_ACTION return KW_UNIQUE; }
"DEFAULT"        { YY_USER_ACTION return KW_DEFAULT; }
"("              { YY_USER_ACTION return LPAREN; }
")"              { YY_USER_ACTION return RPAREN; }
";"              { YY_USER_ACTION return SEMICOLON; }
","              { YY_USER_ACTION return COMMA; }
\n               { yylineno++; yycolumn = 1; yylloc->first_column = 1; }

{IDENT}          { YY_USER_ACTION yylval->str = new std::string(yytext); return IDENTIFIER; }
{STRING}         { YY_USER_ACTION yylval->str = new std::string(yytext); return LITERAL_STRING; }
{NUMBER}         { YY_USER_ACTION yylval->str = new std::string(yytext); return LITERAL_NUMBER; }
{WS}             { /* skip */ }
.                { std::fprintf(stderr, "Unexpected token: %s\n", yytext); return *yytext; }

<<EOF>>          { return 0; }

%%

int yylex(YYSTYPE* lval, YYLTYPE* lloc) {
    yylval = lval;
    yylloc = lloc;
    return yylex(); // calls the real Flex scanner
}
