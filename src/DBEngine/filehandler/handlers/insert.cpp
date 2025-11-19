#include "../public_api.hpp"

void FileHandler::insertData(InsertNode& node, const ExecutionContext& ctx) {
  std::cout << "'Insert' command is valid - starting file operations..." << std::endl;
  // std::filesystem::create_directories(FileHandler::DATASTORAGE_BASE_DIRECTORY); // ensures parent exists

  // // Path for this table
  // const std::string table_datastorage_dir = FileHandler::DATASTORAGE_BASE_DIRECTORY + node.tableName;
  // std::filesystem::create_directories(table_datastorage_dir);

  // TODO File logic
}