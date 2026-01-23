#include "../public_api.hpp"

void FileHandler::updateData(const UpdateNode& node, ExecutionContext& ctx) {
  LoggerService::StatusLogger::printAsStandardOutput("'Update' command is valid - starting file operations...");
  auto start = std::chrono::steady_clock::now();

  FileHandler::ensureTableFileExists(node.table_name);
  const std::string table_file_path = FileHandler::getTableFilePath(node.table_name);
  // Update is supposed to mark the old record as a tombstone, and insert a new record with
  // the new (and old) data.
  // open the file in input/output mode because we need to read the record and also overwrite record types
  std::fstream table_file(table_file_path, std::ios::in | std::ios::out | std::ios::binary);
  FileHandler::checkFileValidity(table_file, node.table_name);

  // TODO file operations

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> double_duration = end - start;
  LoggerService::StatusLogger::printAsStandardOutput("Update finished in " + std::to_string(double_duration.count()) +
                                                     " ms");
}
