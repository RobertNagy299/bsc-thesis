#include "../public_api.hpp"

void FileHandler::createUntypedTable(CreateUntypedTableNode& node, ExecutionContext& ctx) {
  auto untyped_tables = ctx.getUntypedTables();
  const std::string table_name = node.tableName;

  if (untyped_tables.find(table_name) != untyped_tables.end()) {
    LoggerService::ErrorLogger::printAsStandardError("Error: Table " + table_name + " already exists.");
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
    LoggerService::ErrorLogger::printAsStandardError("Error: Could not create metadata file for table " + table_name);
    return;
  }

  for (auto c : node.columns) {
    auto col = dynamic_cast<UntypedColumnDefNode*>(c);
    if (col) {
      file << col->name;
      if (!col->modifiers.empty()) file << " ";
      for (size_t i = 0; i < col->modifiers.size(); ++i) {
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
  LoggerService::StatusLogger::printAsStandardOutput("Created table " + table_name + " with " +
                                                     std::to_string(cols.size()) + " columns");
  ctx.setUntypedTable(table_name, cols);
}
