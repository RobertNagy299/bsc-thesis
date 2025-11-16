%option noyywrap
%option yylineno
%option bison-locations

%{
#include "parser.hpp"

#include <string>
#include <iostream>
#include <cstdio>

extern int yylineno;
extern char* yytext;

YYSTYPE* yylval;
YYLTYPE* yylloc;

%}

IDENT     [a-zA-Z_][a-zA-Z0-9_]*
NUMBER    [+-]?([0-9]+)(\.[0-9]+)?(e?[0-9]+(\.[0-9]+)?)?
WS        [ \t\r]+
STRING    \'([^\\\n]|(\\.))*\'

%%

"CREATE"         {  return KW_CREATE; }
"WHERE"          {  return KW_WHERE; }
"LIKE"           {  return KW_LIKE; }
"AND"            {  return KW_AND; }
"OR"             {  return KW_OR; }

"<"              {  return OP_LT; }
">"              {  return OP_GT; }
"="              {  return OP_EQ; }
"<="             {  return OP_LE; }
">="             {  return OP_GE; }
"!="             {  return OP_NE; }

"SELECT"         {  return KW_SELECT; }
"FROM"           {  return KW_FROM; }
"INSERT"         {  return KW_INSERT; }
"INTO"           {  return KW_INTO; }
"FALSE"          {  return KW_FALSE; }
"TRUE"           {  return KW_TRUE; }
"VALUES"         {  return KW_VALUES; }
"DROP"           {  return KW_DROP; }
"UNTYPED"        {  return KW_UNTYPED; }
"TABLE"          {  return KW_TABLE; }
"PRIMARY"        {  return KW_PRIMARY; }
"KEY"            {  return KW_KEY; }
"NUMBER"         {  return KW_NUMBER_T; }
"NULL"           {  return KW_NULL; }
"NOT"            {  return KW_NOT; }
"IS"             {  return KW_IS; }
"UNIQUE"         {  return KW_UNIQUE; }
"DEFAULT"        {  return KW_DEFAULT; }
"("              {  return LPAREN; }
")"              {  return RPAREN; }
";"              {  return SEMICOLON; }
","              {  return COMMA; }
"*"              {  return ASTERISK; }
"\n"             { }

{IDENT}          {  yylval->str = new std::string(yytext); return IDENTIFIER; }
{STRING}         {  yylval->str = new std::string(yytext); return LITERAL_STRING; }
{NUMBER}         {  yylval->str = new std::string(yytext); return LITERAL_NUMBER; }
{WS}             { /* skip */ }
.                { std::fprintf(stderr, "Unexpected token: %s\n", yytext); return *yytext; }

<<EOF>>          { return 0; }

%%
