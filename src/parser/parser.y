%code requires {
  #include <string>
  #include "ast.hpp"
}

%code {
  #include <string>
  #include <memory>
  #include <iostream>
  #include <sstream>
  #include <vector>
  #include <cstring>
  #include "ast.hpp"

  int yylex(YYSTYPE* yylval, YYLTYPE* yylloc);
  void yyerror(const YYLTYPE* loc, const char* msg);

  extern int yylineno;
  extern char* yytext;
  extern int yytoken;

  ASTNode* root = nullptr;
}

%define api.pure full
%define parse.error verbose
%defines
%locations

%union {
    std::string* str;
    ASTNode* node;
}

/* Tell Bison how to free semantic values if it discards them. */
%destructor { delete $$; } <str>
%destructor { delete $$; } <node>

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
        std::printf("Yay!");
        $$ = new CreateTableNode(*$3, *$5);
        delete $3; delete $5;
        $3 = nullptr; $5 = nullptr;
    }
    ;

%%

void yyerror(const YYLTYPE* loc, const char* msg) {
    if(!loc || !msg) {
      std::cerr << "Parse error - one of yyerror's arguments is a nullptr, Detailed error report not possible.";
    }
    std::cerr << "Parse error at line " << loc->first_line
              << ", column " << loc->first_column << ": ";
    std::cerr << msg << std::endl;
}
