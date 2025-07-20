%code requires {
  #include <string>
  #include "ast.hpp"
}

%{
#include <string>
#include <memory>
#include "ast.hpp"

extern int yylex();
void yyerror(const char *msg);

// Define the global root pointer
ASTNode* root = nullptr;
%}

%union {
    std::string* str;
    ASTNode* node;
}

%token <str> IDENTIFIER
%token CREATE TABLE PRIMARY KEY NUMBER_TYPE
%token LPAREN RPAREN SEMICOLON

%type <node> statement table_decl

%%

program:
    statement SEMICOLON {
      root = $1;
      return 0;
    }
    ;

statement:
    table_decl
    ;

table_decl:
    CREATE TABLE IDENTIFIER LPAREN IDENTIFIER PRIMARY KEY RPAREN {
        printf("Yay!");
        $$ = new CreateTableNode(*$3, *$5);
        delete $3; delete $5;
    }
    ;

%%

void yyerror(const char *msg) {
    fprintf(stderr, "Parse error: %s\n", msg);
}
