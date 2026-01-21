#include "../public_api.hpp"

void FileHandler::dropTable(const DropTableNode& node, ExecutionContext& ctx) {

  auto start = std::chrono::steady_clock::now();

  std::string table_metadata_directory = FileHandler::METADATA_BASE_DIRECTORY + "/" + node.table_name;

  if (!std::filesystem::exists(table_metadata_directory)) {
    LoggerService::ErrorLogger::handleFatalError(StatusCode::FatalErrorCode::METADAT_TableMetadataDirectoryDoesNotExist,
                                                 std::vector<std::string>{node.table_name});

    return;
  }

  try {
    // Remove directory and all metadata
    std::filesystem::remove_all(table_metadata_directory);
    // remove data file
    std::filesystem::remove_all(FileHandler::getTableFolderPath(node.table_name));
    // delete indices
    ctx.eraseInMemoryHashMapIndicesForTable(node.table_name);

    // Now remove from execution context if it exists
    auto& untyped_tables = ctx.transferOwnershipOfUntypedTables();
    auto it = untyped_tables.find(node.table_name);
    if (it != untyped_tables.end()) {
      // free the schema memory before erasing
      for (auto& col : it->second) {
        // clang-format off
            delete col; col = nullptr;
        // clang-format on
      }
      ctx.eraseTable(node.table_name);
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    LoggerService::StatusLogger::printAsStandardOutput("Table '" + node.table_name +
                                                       "' dropped successfully. Operation took " +
                                                       std::to_string(duration.count()) + "ms");

  } catch (const std::filesystem::filesystem_error& e) {
    LoggerService::ErrorLogger::handleFatalError(StatusCode::FatalErrorCode::DROP_FILEOPS_UnknownFileSystemError,
                                                 std::vector<std::string>{node.table_name, e.what()});
  }
}