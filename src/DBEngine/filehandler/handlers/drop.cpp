#include "../public_api.hpp"

void FileHandler::dropTable(DropTableNode& node, ExecutionContext& ctx) {
  std::string table_metadata_directory = FileHandler::METADATA_BASE_DIRECTORY + "/" + node.tableName;

  if (!std::filesystem::exists(table_metadata_directory)) {
    LoggerService::ErrorLogger::handleFatalError(StatusCode::FatalErrorCode::METADAT_TableMetadataDirectoryDoesNotExist,
                                                 std::vector<std::string>{node.tableName});

    return;
  }

  try {
    // Remove directory and all metadata
    std::filesystem::remove_all(table_metadata_directory);
    // TODO Remove datastorage files

    // Now remove from execution context if it exists
    auto untyped_tables = ctx.getUntypedTables();
    auto it = untyped_tables.find(node.tableName);
    if (it != untyped_tables.end()) {
      // free the schema memory before erasing
      for (UntypedColumnDefNode*& col : it->second) {
        // clang-format off
            delete col; col = nullptr;
        // clang-format on
      }
      ctx.eraseTable(it);
    }
    LoggerService::StatusLogger::printAsStandardOutput("Table \"" + node.tableName + "\" dropped successfully.");

  } catch (const std::filesystem::filesystem_error& e) {
    LoggerService::ErrorLogger::handleFatalError(StatusCode::FatalErrorCode::DROP_UnknownFileSystemError,
                                                 std::vector<std::string>{node.tableName, e.what()});
  }
}