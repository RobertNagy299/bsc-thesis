#include "../public_api.hpp"

void FileHandler::deleteData(const DeleteNode& node, const ExecutionContext& ctx) {
  LoggerService::StatusLogger::printAsStandardOutput("'Delete' command is valid - starting file operations...");
  auto start = std::chrono::steady_clock::now();

  FileHandler::ensureTableFileExists(node.table_name);
  const std::string table_file_path = FileHandler::getTableFilePath(node.table_name);

  // open the file in input/output mode because we need to read the record and also overwrite record types
  std::fstream table_file(table_file_path, std::ios::in | std::ios::out | std::ios::binary);
  FileHandler::checkFileValidity(table_file, node.table_name);

  auto table_cols = ctx.getUntypedTables().at(node.table_name);
  if (table_file.is_open()) {
    // perform actual tombstone logic
    // strategy - WHERE node contains a primaryKeyEQ comparison
    const std::string& primary_key_column = Utilities::ColumnUtils::extractPrimaryKeyColumn(table_cols);
    if (node.opt_where_node &&
        node.opt_where_node->conditions_list_node->conditions[0]->col_name == primary_key_column &&
        node.opt_where_node->conditions_list_node->conditions[0]->cmp_node->type == ComparatorNode::Type::EQ) {
      // perform index look-up for fast delete
      FileHandler::performDeleteByIndexLookup(node, ctx, table_file);
    } else {
      // TODO perform sequential delete
    }
  }

  if (table_file.is_open()) { table_file.close(); }

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> double_duration = end - start;
  LoggerService::StatusLogger::printAsStandardOutput("Deletion finished in " + std::to_string(double_duration.count()) +
                                                     " ms");
}

/**
 * jumps to the specified record in the buffer based on the index
 * by the time this function is called, the table header should have been processed
 */
void FileHandler::performDeleteByIndexLookup(const DeleteNode& node, const ExecutionContext& ctx,
                                             std::fstream& table_file) {
  // find the offset in the index
  const auto& offset = ctx.getHashmapIndices()
                           ->at(node.table_name)
                           ->at(node.opt_where_node->conditions_list_node->conditions[0]->col_name)
                           ->at(node.opt_where_node->conditions_list_node->conditions[0]->literal_value->value);
  // seek to the col offset region and then back to the col type region
  // offset is absolute, and seekp is not influenced by seekg, so we seek from the beg.
  table_file.seekp(0, std::ios::beg);
  table_file.seekp(offset + sizeof(DB_Types::table_file_header_t), std::ios::beg);

  // move back
  table_file.seekp(-sizeof(DB_Types::RecordType), std::ios::cur);

  // overwrite the record type
  FileHandler::writeToBinaryFile(table_file, DB_Types::RecordType::DELETE);
}
