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
    std::vector<ASTNode*>* nodeList;
    std::vector<std::string>* strList;
}

/* Tell Bison how to free semantic values if it discards them. */
%destructor { delete $$; } <str>
%destructor { } <node>
%destructor { delete $$; } <strList>

%destructor {
  for (auto p : *$$) {
    delete p;
  }
  delete $$;
} <nodeList>



%token SEMICOLON LPAREN RPAREN KW_NUMBER_T COMMA
%token KW_CREATE KW_UNTYPED KW_TABLE KW_VALUE KW_NOT KW_NULL KW_KEY KW_PRIMARY KW_DEFAULT KW_UNIQUE

%token <str> IDENTIFIER LITERAL_STRING LITERAL_NUMBER LITERAL_VALUE

%type <node> program statement tabl_crea untyped_col_def
%type <nodeList> untyped_col_defs
%type <strList> opt_col_modifiers
%type <str> col_modifier literal_value


%%
program
    : statement SEMICOLON {
        auto prog = new ProgramNode();
        prog->statements.push_back($1);
        root = prog;   // root is global
        $$ = prog;
    }
    | program statement SEMICOLON {
        auto prog = static_cast<ProgramNode*>($1);
        prog->statements.push_back($2);
        $$ = prog;
    }
    ;

statement
    : tabl_crea { $$ = $1; }
    ;

tabl_crea
    : KW_CREATE KW_UNTYPED KW_TABLE IDENTIFIER LPAREN untyped_col_defs RPAREN {
        $$ = new CreateUntypedTableNode(*$4, *$6);
        delete $4; delete $6;
    }
    ;

untyped_col_defs
    : /* empty */ {
        $$ = new std::vector<ASTNode*>();
    }
    | untyped_col_def {
        $$ = new std::vector<ASTNode*>();
        $$->push_back($1);
    }
    | untyped_col_def COMMA untyped_col_defs {
        $3->insert($3->begin(), $1);
        $$ = $3;
    }
    ;

untyped_col_def
    : IDENTIFIER opt_col_modifiers {
        $$ = new UntypedColumnDefNode(*$1, *$2);
        delete $1; 
        $1 = nullptr;
        delete $2;
        $2 = nullptr;
    }
    ;

opt_col_modifiers
    : /* empty */ {
        $$ = new std::vector<std::string>();
    }
    | opt_col_modifiers col_modifier {
        $1->push_back(*$2);
        $$ = $1;
        delete $2;
        $2 = nullptr;
    }
    ;

col_modifier
    : KW_PRIMARY KW_KEY   { $$ = new std::string("PRIMARY KEY"); }
    | KW_NOT KW_NULL      { $$ = new std::string("NOT NULL"); }
    | KW_UNIQUE        { $$ = new std::string("UNIQUE"); }
    | KW_DEFAULT literal_value { $$ = new std::string("DEFAULT " + *$2); delete $2; $2 = nullptr; }
    ;

literal_value
    : LITERAL_NUMBER 
    | LITERAL_STRING  {
      $$ = new std::string(*$1);
      delete $1; 
      $1 = nullptr;
    } 
%%

void yyerror(const YYLTYPE* loc, const char* msg) {
    if(!loc || !msg) {
      std::cerr << "Parse error - one of yyerror's arguments is a nullptr, Detailed error report not possible.";
    }
    std::cerr << "Parse error at line " << loc->first_line
              << ", column " << loc->first_column << ": ";
    std::cerr << msg << std::endl;
}
