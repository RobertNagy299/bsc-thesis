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
%destructor { delete $$; } <strList>

%destructor {
  for (auto p : *$$) {
    delete p;
  }
  delete $$;
} <nodeList>



%token SEMICOLON LPAREN RPAREN COMMA
%token KW_CREATE KW_UNTYPED KW_TABLE KW_NOT KW_KEY KW_PRIMARY KW_DEFAULT KW_UNIQUE KW_DROP
%token KW_INSERT KW_INTO KW_FALSE KW_TRUE KW_NUMBER_T KW_NULL KW_VALUES

%token <str> IDENTIFIER LITERAL_STRING LITERAL_NUMBER

%type <node> program statement tabl_crea untyped_col_def tabl_drop tabl_insert
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
    | tabl_drop { $$ = $1; }
    | tabl_insert { $$ = $1; }
    ;

tabl_crea
    : KW_CREATE KW_UNTYPED KW_TABLE IDENTIFIER LPAREN untyped_col_defs RPAREN {
        $$ = new CreateUntypedTableNode(*$4, *$6);
        delete $4; delete $6;
    }
    ;

tabl_drop
    : KW_DROP KW_TABLE IDENTIFIER {
      $$ = new DropTableNode(*$3);
      delete $3;
      $3 = nullptr;
    }

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
    | KW_NULL
    | KW_FALSE
    | KW_TRUE 
    | LITERAL_STRING  {
      $$ = new std::string(*$1);
      delete $1; 
      $1 = nullptr;
    } 

tabl_insert 
    : KW_INSERT KW_INTO IDENTIFIER opt_col_list KW_VALUES values_list {
        $$ = new InsertNode(*$4, *$6);
        delete $4; delete $6;
        $4 = nullptr; $6 = nullptr;
      }
    ;

opt_col_list 
    : /* empty */ {
      $$ = new std::vector<std::string>();
    }
    | LPAREN col_list RPAREN {
        $$ = $1;
      }
    ;

col_list 
    : col_list COMMA IDENTIFIER {
        
      }
    | IDENTIFIER
    ;

values_list
    : values_list COMMA value_record
    | value_record
    ;

value_record 
    : LPAREN literal_list RPAREN
    ;

literal_list 
    : literal_list COMMA literal_value
    | literal_value
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
