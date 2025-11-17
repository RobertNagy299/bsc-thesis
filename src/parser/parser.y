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
    WhereNode* whereNode;
    ComparatorNode* comparatorNode;
    AssignmentNode* assignmentNode;
    AssignmentListNode* assignmentListNode;
    ConditionListNode* conditionListNode;
    std::vector<LiteralNode*>* litList;
    LiteralNode* literal;
}

/* Tell Bison how to free semantic values if it discards them. */
/* TODO: revisit this for syntax errors with update, delete, insert */
%destructor { delete $$; $$ = nullptr; } <str>
%destructor { delete $$; $$ = nullptr; } <strList>

%destructor {
  for (auto &p : *$$) {
    delete p; p = nullptr;
  }
  delete $$; $$ = nullptr;
} <nodeList>


%token SEMICOLON LPAREN RPAREN COMMA ASTERISK
%token KW_CREATE KW_UNTYPED KW_TABLE KW_NOT KW_KEY KW_PRIMARY KW_DEFAULT KW_UNIQUE KW_DROP
%token KW_INSERT KW_INTO KW_NUMBER_T KW_VALUES KW_TRUE KW_NULL KW_FALSE KW_SELECT KW_FROM KW_DELETE
%token KW_WHERE KW_LIKE KW_IS KW_AND KW_OR KW_SET KW_UPDATE
%token OP_EQ OP_GE OP_GT OP_LE OP_LT OP_NE

%token <str> IDENTIFIER LITERAL_STRING LITERAL_NUMBER

%type <node> program statement tabl_crea untyped_col_def tabl_drop tabl_insert tabl_select tabl_delete tabl_update
%type <whereNode> opt_where_statement where_statement
%type <comparatorNode> comparator
%type <conditionListNode> condition_list
%type <nodeList> untyped_col_defs
%type <strList> opt_col_modifiers
%type <str> col_modifier
%type <literal> literal_value
%type <litList> literal_list
%type <valRecord> value_record
%type <valuesList> values_list
%type <colList> opt_col_list col_list selection_col_statement
%type <assignmentNode> assignment
%type <assignmentListNode> assignment_list

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
  | tabl_delete { $$ = $1; }
  | tabl_update { $$ = $1; }
  ;

tabl_update
  : KW_UPDATE IDENTIFIER KW_SET assignment_list opt_where_statement {
      $$ = new UpdateNode(*$2, $4, $5);
      delete $2; $2 = nullptr;
    }
  ;

assignment_list
  : assignment {
      auto assignment_vector = std::vector<AssignmentNode*>();
      assignment_vector.push_back($1);
      $$ = new AssignmentListNode(std::move(assignment_vector));
    } 
  | assignment_list COMMA assignment {
      $1->assignments.push_back($3);
      $$ = $1;
    }
  ;

assignment
  : IDENTIFIER OP_EQ literal_value {
      $$ = new AssignmentNode(*$1, $3);
      delete $1; $1 = nullptr;
    }
  ;

tabl_delete
  : KW_DELETE KW_FROM IDENTIFIER opt_where_statement {
      $$ = new DeleteNode(*$3, $4);
      delete $3; $3 = nullptr;
    }
  ;

tabl_crea
  : KW_CREATE KW_UNTYPED KW_TABLE IDENTIFIER LPAREN untyped_col_defs RPAREN {
      $$ = new CreateUntypedTableNode(*$4, *$6);
      delete $4; $4 = nullptr; 
      delete $6; $6 = nullptr;
    }
  ;

tabl_drop
  : KW_DROP KW_TABLE IDENTIFIER {
      $$ = new DropTableNode(*$3);
      delete $3; $3 = nullptr;
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
      delete $1; $1 = nullptr;
      delete $2; $2 = nullptr;
    }
  ;

opt_col_modifiers
  : /* empty */ {
      $$ = new std::vector<std::string>();
    }
  | opt_col_modifiers col_modifier {
      $1->push_back(*$2);
      $$ = $1;
      delete $2; $2 = nullptr;
    }
  ;

col_modifier
  : KW_PRIMARY KW_KEY        { $$ = new std::string("PRIMARY KEY"); }
  | KW_NOT KW_NULL           { $$ = new std::string("NOT NULL"); }
  | KW_UNIQUE                { $$ = new std::string("UNIQUE"); }
  | KW_DEFAULT literal_value { $$ = new std::string("DEFAULT " + $2->value); delete $2; $2 = nullptr; }
  ;

tabl_insert
  : KW_INSERT KW_INTO IDENTIFIER opt_col_list KW_VALUES values_list {
      $$ = new InsertNode(*$3, $4, $6);
      delete $3; $3 = nullptr;
    }
  ;

tabl_select
  : KW_SELECT selection_col_statement KW_FROM IDENTIFIER opt_where_statement {
      $$ = new SelectNode($2, *$4, $5);
      delete $4; $4 = nullptr;
    }
  ;

opt_where_statement
  : /* empty */ {
      $$ = nullptr;
    }
  | where_statement {
      $$ = $1;
    }
  ;

where_statement
  : KW_WHERE condition_list {
      $$ = new WhereNode($2);
    }
  ;

/* This is just a single condition for now */
condition_list
  : IDENTIFIER comparator literal_value {
      auto new_node = new ConditionNode(*$1, $2, $3);
      auto vec = std::vector<ConditionNode*>();
      vec.push_back(new_node);
      $$ = new ConditionListNode(std::move(vec));
      delete $1; $1 = nullptr;
    }
  ;

comparator
  : KW_IS          { $$ = new ComparatorNode(ComparatorNode::Type::IS); }
  | KW_LIKE        { $$ = new ComparatorNode(ComparatorNode::Type::LIKE); }
  | KW_IS KW_NOT   { $$ = new ComparatorNode(ComparatorNode::Type::IS_NOT); }
  | KW_NOT KW_LIKE { $$ = new ComparatorNode(ComparatorNode::Type::NOT_LIKE); }
  | OP_EQ          { $$ = new ComparatorNode(ComparatorNode::Type::EQ); }
  | OP_GE          { $$ = new ComparatorNode(ComparatorNode::Type::GE); }
  | OP_GT          { $$ = new ComparatorNode(ComparatorNode::Type::GT); }
  | OP_LE          { $$ = new ComparatorNode(ComparatorNode::Type::LE); }
  | OP_LT          { $$ = new ComparatorNode(ComparatorNode::Type::LT); }
  | OP_NE          { $$ = new ComparatorNode(ComparatorNode::Type::NE); }
  ;

selection_col_statement
  : ASTERISK {
      $$ = nullptr;
    } 
  | col_list {
      $$ = $1;
    }
  ;

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
      delete $1; $1 = nullptr;
    }
  | col_list COMMA IDENTIFIER {
      $1->columns.push_back(*$3);
      $$ = $1;
      delete $3; $3 = nullptr;
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
      delete $2; $2 = nullptr;
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
  | LITERAL_NUMBER { $$ = new LiteralNode(LiteralNode::Type::NUMBER, *$1); delete $1; $1 = nullptr; }
  | LITERAL_STRING { $$ = new LiteralNode(LiteralNode::Type::STRING, *$1); delete $1; $1 = nullptr; }
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

