%locations

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

%union {
    std::string* str;
    ASTNode* node;
    std::vector<ASTNode*>* nodeList;
    std::vector<std::string>* strList;
    ColumnListNode* colList;
    ValuesListNode* valuesList;
    ValueRecordNode* valRecord;
    std::vector<LiteralNode*>* litList;
    LiteralNode* literal;
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

// TODO: destructor rules for insert nodes

%token SEMICOLON LPAREN RPAREN COMMA ASTERISK
%token KW_CREATE KW_UNTYPED KW_TABLE KW_NOT KW_KEY KW_PRIMARY KW_DEFAULT KW_UNIQUE KW_DROP
%token KW_INSERT KW_INTO KW_NUMBER_T KW_VALUES KW_TRUE KW_NULL KW_FALSE KW_SELECT KW_FROM

%token <str> IDENTIFIER LITERAL_STRING LITERAL_NUMBER

%type <node> program statement tabl_crea untyped_col_def tabl_drop tabl_insert tabl_select
%type <nodeList> untyped_col_defs
%type <strList> opt_col_modifiers
%type <str> col_modifier
%type <literal> literal_value
%type <litList> literal_list
%type <valRecord> value_record
%type <valuesList> values_list
%type <colList> opt_col_list col_list selection_col_statement

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
    | tabl_select { $$ = $1; }
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
    | KW_DEFAULT literal_value { $$ = new std::string("DEFAULT " + $2->value); delete $2; $2 = nullptr; }
    ;

tabl_insert
    : KW_INSERT KW_INTO IDENTIFIER opt_col_list KW_VALUES values_list {
        $$ = new InsertNode(*$3, $4, $6);
        delete $3; // cleanup IDENTIFIER string
      }
    ;

tabl_select
  : KW_SELECT selection_col_statement KW_FROM IDENTIFIER {
      $$ = new SelectNode($2, *$4);
      delete $4;
    }
  ;

selection_col_statement
  : ASTERISK {
    $$ = nullptr;
  } 
  | col_list {
    $$ = $1;
  }

opt_col_list
    : /* empty */ {
        $$ = nullptr;
      }
    | LPAREN col_list RPAREN {
        $$ = $2;
      }
    ;

col_list
    : IDENTIFIER {
        auto cols = std::vector<std::string>{ *$1 };
        $$ = new ColumnListNode(std::move(cols));
        delete $1;
      }
    | col_list COMMA IDENTIFIER {
        $1->columns.push_back(*$3);
        $$ = $1;
        delete $3;
      }
    ;

values_list
    : value_record {
        auto vec = std::vector<ValueRecordNode*>{ $1 };
        $$ = new ValuesListNode(std::move(vec));
      }
    | values_list COMMA value_record {
        $1->records.push_back($3);
        $$ = $1;
      }
    ;

value_record
    : LPAREN literal_list RPAREN {
        $$ = new ValueRecordNode(std::move(*$2));
        delete $2;
      }
    ;

literal_list
    : literal_value {
        auto vec = std::vector<LiteralNode*>{ $1 };
        $$ = new std::vector<LiteralNode*>(std::move(vec));
      }
    | literal_list COMMA literal_value {
        $1->push_back($3);
        $$ = $1;
      }
    ;

literal_value
    : /* empty */    { $$ = new LiteralNode(LiteralNode::Type::EMPTY, ""); }
    | LITERAL_NUMBER { $$ = new LiteralNode(LiteralNode::Type::NUMBER, *$1); delete $1; }
    | LITERAL_STRING { $$ = new LiteralNode(LiteralNode::Type::STRING, *$1); delete $1; }
    | KW_NULL        { $$ = new LiteralNode(LiteralNode::Type::NULLVAL, "NULL"); }
    | KW_TRUE        { $$ = new LiteralNode(LiteralNode::Type::TRUEVAL, "TRUE"); }
    | KW_FALSE       { $$ = new LiteralNode(LiteralNode::Type::FALSEVAL, "FALSE"); }
    ;
%%

void yyerror(const YYLTYPE* loc, const char* msg) {
    if(!loc || !msg) {
      std::cerr << "Parse error - one of yyerror's arguments is a nullptr, Detailed error report not possible.";
    }
    std::cerr << msg << std::endl;
}
