#include "../public_api.hpp"

void FileHandler::createUntypedTable(CreateUntypedTableNode& node, ExecutionContext& ctx) {
  auto untyped_tables = ctx.getUntypedTables();
  const std::string table_name = node.table_name;
  // TODO move this to semantic validator maybe??
  if (untyped_tables.find(table_name) != untyped_tables.end()) {
    LoggerService::ErrorLogger::printAsStandardError(StatusCode::ErrorCode::SEMVAL_CREATE_TableAlreadyExists,
                                                     std::vector<std::string>{table_name});
    return;
  }

  // Store metadata on disk
  // Base directory for schema
  std::filesystem::create_directories(FileHandler::METADATA_BASE_DIRECTORY); // ensures parents exist

  // Path for this table
  std::string table_path = FileHandler::METADATA_BASE_DIRECTORY + "/" + table_name;
  std::filesystem::create_directories(table_path);

  // Write metadata file
  std::string metadata_path = table_path + "/metadata.txt";
  std::ofstream file(metadata_path);
  if (!file.is_open()) {
    LoggerService::ErrorLogger::handleFatalError(StatusCode::FatalErrorCode::METADAT_CouldNotCreateMetadataFileForTable,
                                                 std::vector<std::string>{table_name});
    return;
  }

  for (auto c : node.columns) {
    auto col = dynamic_cast<UntypedColumnDefNode*>(c);
    if (col) {
      file << col->name;
      if (!col->modifiers.empty()) file << " ";
      for (std::size_t i = 0; i < col->modifiers.size(); ++i) {
        file << col->modifiers[i];
        if (i + 1 < col->modifiers.size()) file << ",";
      }
      file << "\n";
    }
  }
  file.close();

  // Store schema in context
  std::vector<UntypedColumnDefNode*> cols;
  cols.reserve(node.columns.size());
  for (auto* col : node.columns) {
    auto casted_col = dynamic_cast<UntypedColumnDefNode*>(col);
    if (casted_col) {
      cols.push_back(new UntypedColumnDefNode(*casted_col)); // deep copy
    }
  }

  // create empty indicies for this table.
  ctx.recalculateIndicesForTable(table_name);

  LoggerService::StatusLogger::printAsStandardOutput("Created table " + table_name + " with " +
                                                     std::to_string(cols.size()) + " columns");
  ctx.setUntypedTable(table_name, cols);
}
