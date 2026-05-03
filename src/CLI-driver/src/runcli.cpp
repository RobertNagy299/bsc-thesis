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
#include <unistd.h> // for isatty

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

  for (const auto& c : s) {
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

  for (auto it = s.rbegin(); it != s.rend(); ++it) {
    if (std::isspace(static_cast<unsigned char>(*it))) continue;
    return *it == ';';
  }

  return false;
}

void parseAndExecute(const std::string& sql) {
  auto start = std::chrono::steady_clock::now();
  root = nullptr;
  std::vector<char> tmp_buffer(sql.begin(), sql.end());
  FILE* f = fmemopen(tmp_buffer.data(), tmp_buffer.size(), "r");
  if (!f) {
    std::cerr << "Failed to open SQL buffer\n";
    return;
  }

  yyin = f;
  int result = yyparse();
  fclose(f);
  yylex_destroy();

  if (result != 0) {
    if (root) {
      // clang-format off
      delete root; root = nullptr;  // Cleanup partial AST if it exists
      // clang-format on
    }
    std::cerr << "Parse error.\n";
    return;
  }

  ExecutionContext& ctx = ExecutionContext::getInstance();
  InterpreterVisitor visitor(ctx);
  try {
    root->accept(visitor);
  } catch (std::exception& err) {
    std::cerr << "An unexpected exception occured while parsing the input. Error message: " << err.what() << std::endl;
    if (root) {
      // clang-format off
    delete root; root = nullptr;
      // clang-format on
    }
  }

  if (root) {
    // clang-format off
    delete root; root = nullptr;
    // clang-format on
  }
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> double_duration = end - start;
  if (!LoggerService::is_silent_mode) {
    std::cout << std::endl
              << ">>> The complete query was executed in " << std::to_string(double_duration.count()) << " ms"
              << std::endl;
  }
}

void performTotalCleanup(bool is_initialized, std::string& sql_buffer, bool is_interactive) {
  sql_buffer.clear();
  if (is_initialized) { ExecutionContext::destroyInstance(); }
  if (is_interactive) {
    cleanupReadline();
    if (!LoggerService::is_silent_mode) { std::cout << std::endl << "Bye!" << std::endl; }
  }
}

void processArgs(int argc, char** argv) {
  // default flag values
  bool silent = false;

  // user provided flag values
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--silent") { silent = true; }
  }

  LoggerService::setSilentMode(silent);
}

} // namespace

void CLIDriver::runCLI(int argc, char** argv) {
  processArgs(argc, argv);
  stifle_history(100);
  // 1 MB = 1024 * 1024 bytes
  const size_t MAX_SQL_BUFFER_SIZE = 1024u * 1024u;
  if (!LoggerService::is_silent_mode) { std::cout << "Welcome to MiniSQL. Type INIT, QUIT, or SQL statements.\n"; }

  std::string sqlBuffer;
  bool initialized = false;

  const bool interactive = isatty(STDIN_FILENO);

  while (true) {
    std::string line;

    if (interactive) {
      const char* prompt = sqlBuffer.empty() ? "sql> " : "  -> ";
      char* input = readline(prompt);

      if (!input) {
        if (!LoggerService::is_silent_mode) { std::cout << std::endl << "Bye!" << std::endl; }
        break;
      }

      line = input;
      free(input);

      if (!line.empty()) { add_history(line.data()); }
    } else {
      // This part handles the case when SQL commands are piped or coming from stdin, a file, etc.
      if (!std::getline(std::cin, line)) {
        // If the file ends but there's a partial command, we should clear the buffers
        // and destroy everything. Do not execute unfinished commands.
        line.clear();
        break; // EOF
      }
    }
    line.erase(line.begin(),
               std::find_if(line.begin(), line.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
               line.end());

    if (line == "QUIT") { break; }

    if (line == "INIT") {
      if (!initialized) {
        ExecutionContext::getInstance();
        initialized = true;
      }
      continue;
    }

    if (sqlBuffer.size() > MAX_SQL_BUFFER_SIZE || line.size() > MAX_SQL_BUFFER_SIZE) {
      std::cerr << "SQL Command is too large, SQL Buffer exceeded 1 MB. Terminating the CLI..." << std::endl;
      break;
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
  // only put the cleanup here, because this runs when the while loop breaks.
  // Only use the cleanup inside the loop to handle exceptions.
  performTotalCleanup(initialized, sqlBuffer, interactive);
}