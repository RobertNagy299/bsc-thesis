#include <iostream>
#include "parser/ast.hpp"
#include "interpreter/execution_context.hpp"
#include "interpreter/interpreter_visitor.hpp"
#include <cstring>

extern int yyparse();
extern ProgramNode *root; // Defined in parser.y
extern FILE *yyin;
extern int yylex_destroy(); // <-- from flex

int main()
{
  std::cout << "Parsing started from main.cpp!\n";
  const char *sql = "CREATE UNTYPED TABLE users(id, name, age);";

  // Open SQL string as input
  FILE *f = fmemopen((void *)sql, strlen(sql), "r");

  yyin = f;
  yyparse();
  fclose(f);
  // Destroy Flex's internal buffers to avoid "still reachable" memory.
  yylex_destroy();

  if (root)
  {
    ExecutionContext ctx;
    ctx.init();

    InterpreterVisitor visitor(ctx);
    root->accept(visitor); // Run the program via visitor

    std::cout << "Successful parsing in main,cpp!\n";
  }
  else
  {
    std::cerr << "Parsing failed.\n";
  }
  return 0;
}
