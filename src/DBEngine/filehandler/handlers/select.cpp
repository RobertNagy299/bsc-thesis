#include "../public_api.hpp"

DB_Types::ResultSet FileHandler::selectData(const SelectNode& node, const ExecutionContext& ctx) {
  LoggerService::StatusLogger::printAsStandardOutput("'Select' command is valid - starting file operations...");
  auto start = std::chrono::steady_clock::now();

  FileHandler::ensureTableFileExists(node.table_name);
  const std::string table_file_path = FileHandler::getTableFilePath(node.table_name);
  std::ifstream table_file(table_file_path, std::ios::binary);

  // implement selection logic here
  DB_Types::ResultSet results;

  if (table_file.is_open()) { table_file.close(); }

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> double_duration = end - start;
  LoggerService::StatusLogger::printAsStandardOutput("Selection finished in " +
                                                     std::to_string(double_duration.count()) + " ms");
}
