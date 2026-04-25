/**
 * =========== DISCLAIMER! =============
 *
 * This part of the database system is interacting with third party, multiple decades old C-libraries
 * Hence, the coding style might be more C-style in this file and might not resemble idiomatic C++ code
 *
 * Furthermore, Valgrind will report "still reachable" memory "leaks" due to GNU readline.
 * This is expected and a suppression file should take care of GNU readline false positives.
 */

#include "../../DBEngine/execution_context/public_api.hpp"
#include "../../interpreter/interpreter_visitor.hpp"
#include "../../parser/ast.hpp"
#include "../public-api.hpp"
#include <cstring>
#include <iostream>
#include <readline/history.h>
#include <readline/readline.h>
#include <sstream>
#include <string>

extern int yyparse();
extern ProgramNode* root;
extern FILE* yyin;
extern int yylex_destroy();

namespace {

void cleanupReadline() {
  clear_history();
  rl_free_line_state();
  rl_cleanup_after_signal();
}

bool endsWithSemicolonOutsideQuotes(const std::string& s) {
  bool inSingleQuote = false;
  bool escaped = false;

  for (char c : s) {
    if (escaped) {
      escaped = false;
      continue;
    }

    if (c == '\\') {
      escaped = true;
      continue;
    }

    if (c == '\'') { inSingleQuote = !inSingleQuote; }
  }

  if (inSingleQuote) return false;

  for (std::size_t i = (s.size()) - 1u; i >= 0u; --i) {
    if (isspace(static_cast<unsigned char>(s[i]))) continue;
    return s[i] == ';';
  }
  return false;
}

void parseAndExecute(const std::string& sql) {
  auto start = std::chrono::steady_clock::now();
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

  if (root) {
    // clang-format off
    delete root; root = nullptr;
    // clang-format on
  }
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> double_duration = end - start;
  std::cout << std::endl
            << ">>> The complete query was executed in " << std::to_string(double_duration.count()) << " ms"
            << std::endl;
}

} // namespace

void CLIDriver::runCLI() {
  stifle_history(100);
  std::cout << "Welcome to MiniSQL. Type INIT, QUIT, or SQL statements.\n";

  std::string sqlBuffer;
  bool initialized = false;

  while (true) {
    const char* prompt = sqlBuffer.empty() ? "sql> " : "  -> ";
    char* input = readline(prompt);

    if (!input) { // Ctrl+D
      std::cout << "\nBye!\n";
      break;
    }

    std::string line(input);
    free(input);

    // Ignore empty lines (but still allow multiline continuation)
    if (!line.empty()) { add_history(line.c_str()); }

    if (line == "QUIT") {
      if (initialized) { ExecutionContext::destroyInstance(); }
      cleanupReadline();
      std::cout << "Bye.\n";
      break;
    }

    if (line == "INIT") {
      if (!initialized) {
        ExecutionContext::getInstance();
        initialized = true;
      } else {
        std::cout << "Already initialized.\n";
      }
      continue;
    }

    sqlBuffer += line;
    sqlBuffer += '\n';

    if (!endsWithSemicolonOutsideQuotes(sqlBuffer)) { continue; }

    if (!initialized) {
      std::cerr << "Error: INIT must be called first.\n";
      sqlBuffer.clear();
      continue;
    }

    parseAndExecute(sqlBuffer);
    sqlBuffer.clear();
  }
}
