#include "DBEngine/execution_context/public_api.hpp"
#include "interpreter/interpreter_visitor.hpp"
#include "parser/ast.hpp"
#include <cstring>
#include <iostream>

extern int yyparse();
extern ProgramNode* root; // Defined in parser.y
extern FILE* yyin;
extern int yylex_destroy(); // <-- from flex

int main() {
  std::cout << "Parsing started from main.cpp!\n";
  // Todo: investigate why YY_USER_ACTION runs twice - fix that
  // const char *sql = "CREATE UNTYPED TABLE users(id PRIMARY KEY, name NOT NULL DEFAULT 'Gipsz jakab', age DEFAULT
  // 18);"; const char *sql = "DROP TABLE users;"; const char *sql = "INSERT INTO users VALUES(6, 'Frankenstein', 19);";
  // const char* sql = "INSERT INTO users(id, pisa) VALUES (1,4);";
  // const char *sql = "SELECT id, name FROM users WHERE id IS NOT NULL;";
  // const char *sql = "INSERT INTO users(id, name) VALUES (1,'Manfred'), (2, 'Albert');";
  // const char* sql = "DELETE FROM users WHERE name = TRUE;";
  // const char* sql = "UPDATE users SET name = 'retro', age = 76 WHERE id = 1;";
  // const char* sql = "INSERT INTO comments(id, content, timestamp) VALUES (141,'comment');";
  const char* sql = "INSERT INTO comments VALUES (1,'comment', 15);";

  // Open SQL string as input
  FILE* f = fmemopen((void*)sql, strlen(sql), "r");

  yyin = f;
  yyparse();
  fclose(f);
  // Destroy Flex's internal buffers to avoid "still reachable" memory.
  yylex_destroy();

  if (root) {
    ExecutionContext& ctx = ExecutionContext::getInstance();

    InterpreterVisitor visitor(ctx);
    root->accept(visitor); // Run the program via visitor

    std::cout << "Successful parsing in main,cpp!\n";
    // clang-format off
    delete root; root = nullptr;
    // clang-format on
    ExecutionContext::destroyInstance();
  } else {
    std::cerr << "Parsing failed.\n";
  }
  return 0;
}
