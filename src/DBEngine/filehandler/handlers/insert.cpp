#include "../public_api.hpp"

void FileHandler::insertData(InsertNode& node, const ExecutionContext& ctx) {
  LoggerService::StatusLogger::printAsStandardOutput("'Insert' command is valid - starting file operations...");

  auto start = std::chrono::steady_clock::now();
  FileHandler::ensureTableFileExists(node.tableName);

  auto& current_table = ctx.getUntypedTables().at(node.tableName);
  // command is valid, so we must check for missing literals and get their default values
  // 2 distinct cases: 1. the column list is specified. 2. it is not.
  // we need to iterate through each value record, calculate its length, and create the header

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> double_duration = end - start;
  LoggerService::StatusLogger::printAsStandardOutput("Insertion finished in " +
                                                     std::to_string(double_duration.count()) + " ms");
}
