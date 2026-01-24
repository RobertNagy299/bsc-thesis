#include "../../DBEngine/execution_context/public_api.hpp"
#include "../../interpreter/interpreter_visitor.hpp"
#include "../../parser/ast.hpp"
#include "../public-api.hpp"
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

extern int yyparse();
extern ProgramNode* root;
extern FILE* yyin;
extern int yylex_destroy();

namespace {

bool endsWithSemicolonOutsideQuotes(const std::string& s) {
  bool inSingleQuote = false;

  for (char c : s) {
    if (c == '\'') inSingleQuote = !inSingleQuote;
  }

  if (inSingleQuote) return false;

  for (int i = s.size() - 1; i >= 0; --i) {
    if (isspace(s[i])) continue;
    return s[i] == ';';
  }
  return false;
}

void parseAndExecute(const std::string& sql) {
  root = nullptr;

  FILE* f = fmemopen((void*)sql.c_str(), sql.size(), "r");
  if (!f) {
    std::cerr << "Failed to open SQL buffer\n";
    return;
  }

  yyin = f;
  int result = yyparse();
  fclose(f);
  yylex_destroy();

  if (result != 0 || !root) {
    std::cerr << "Parse error.\n";
    return;
  }

  ExecutionContext& ctx = ExecutionContext::getInstance();
  InterpreterVisitor visitor(ctx);
  root->accept(visitor);

  delete root;
  root = nullptr;
}

} // namespace

void CLIDriver::runCLI() {
  std::cout << "Welcome to MiniSQL. Type INIT, QUIT, or SQL statements.\n";

  std::string line;
  std::string sqlBuffer;
  bool initialized = false;

  while (true) {
    std::cout << (sqlBuffer.empty() ? "sql> " : "  -> ");
    if (!std::getline(std::cin, line)) break;

    if (line == "QUIT") {
      if (initialized) { ExecutionContext::destroyInstance(); }
      std::cout << "Bye.\n";
      break;
    }

    if (line == "INIT") {
      if (!initialized) {
        ExecutionContext::getInstance();
        initialized = true;
        std::cout << "Execution context initialized.\n";
      } else {
        std::cout << "Already initialized.\n";
      }
      continue;
    }

    sqlBuffer += line;
    sqlBuffer += '\n';

    if (!endsWithSemicolonOutsideQuotes(sqlBuffer)) continue;

    if (!initialized) {
      std::cerr << "Error: INIT must be called first.\n";
      sqlBuffer.clear();
      continue;
    }

    parseAndExecute(sqlBuffer);
    sqlBuffer.clear();
  }
}
