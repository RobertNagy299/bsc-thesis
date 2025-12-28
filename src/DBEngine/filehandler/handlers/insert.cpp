#include "../public_api.hpp"

void FileHandler::insertData(InsertNode& node, const ExecutionContext& ctx) {
  LoggerService::StatusLogger::printAsStandardOutput("'Insert' command is valid - starting file operations...");

  auto start = std::chrono::steady_clock::now();
  FileHandler::ensureTableFileExists(node.tableName);

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> double_duration = end - start;
  LoggerService::StatusLogger::printAsStandardOutput("Insertion finished in " +
                                                     std::to_string(double_duration.count()) + " ms");
}

void FileHandler::ensureTableFileExists(const std::string& table_name) {
  // ensures parent exists
  std::filesystem::create_directories(FileHandler::DATASTORAGE_BASE_DIRECTORY);
  // Path for this table
  const std::string table_datastorage_dir = FileHandler::DATASTORAGE_BASE_DIRECTORY + "/" + table_name;
  std::filesystem::create_directories(table_datastorage_dir);

  const std::string table_datastorage_file = table_datastorage_dir + "/" + table_name + ".bin";
  if (!std::filesystem::exists(std::filesystem::path(table_datastorage_file))) {
    std::ofstream table_file(table_datastorage_file);
    if (table_file.is_open()) {
      table_file.close();
    } else {
      LoggerService::ErrorLogger::printAsStandardError("ERROR: Could not create file: " + table_datastorage_file);
    }
  }
}